#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/and_then.hpp"
#include <cstring>
#include <source_location>
#include <system_error>
#include <cerrno>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>

using namespace std::chrono_literals;

namespace co_async {

auto checkError(auto res, std::source_location const &loc = std::source_location::current()) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category(),
                                (std::string)loc.file_name() + ":" + std::to_string(loc.line()));
    }
    return res;
}

struct EpollFilePromise : Promise<void> {
    auto get_return_object() {
        return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
    }

    EpollFilePromise &operator=(EpollFilePromise &&) = delete;

    inline ~EpollFilePromise();

    struct EpollLoop *mLoop;
    int mFileNo;
    uint32_t mEvents;
};

struct EpollLoop {
    void addListener(EpollFilePromise &promise) {
        struct epoll_event event;
        event.events = promise.mEvents;
        event.data.ptr = &promise;
        checkError(epoll_ctl(mEpoll, EPOLL_CTL_ADD, promise.mFileNo, &event));
    }

    void removeListener(int fileNo) {
        checkError(epoll_ctl(mEpoll, EPOLL_CTL_DEL, fileNo, NULL));
    }

    void tryRun(int timeout) {
        int res = checkError(epoll_wait(mEpoll, mEventBuf, std::size(mEventBuf), timeout));
        for (int i = 0; i < res; i++) {
            auto &event = mEventBuf[i];
            auto &promise = *(EpollFilePromise *)event.data.ptr;
            std::coroutine_handle<EpollFilePromise>::from_promise(promise).resume();
        }
    }

    EpollLoop &operator=(EpollLoop &&) = delete;
    ~EpollLoop() {
        close(mEpoll);
    }

    int mEpoll = checkError(epoll_create1(0));
    struct epoll_event mEventBuf[64];
};

EpollFilePromise::~EpollFilePromise() {
    if (mLoop) {
        mLoop->removeListener(mFileNo);
    }
}

struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void
    await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) const {
        auto &promise = coroutine.promise();
        promise.mLoop = &mLoop;
        promise.mFileNo = mFileNo;
        promise.mEvents = mEvents;
        mLoop.addListener(promise);
    }

    void await_resume() const noexcept {}

    using ClockType = std::chrono::system_clock;

    EpollLoop &mLoop;
    int mFileNo;
    uint32_t mEvents;
};

inline Task<void, EpollFilePromise>
wait_file(EpollLoop &loop, int fileNo, uint32_t events) {
    co_await EpollFileAwaiter(loop, fileNo, events | EPOLLONESHOT);
}

}

co_async::EpollLoop epollLoop;
co_async::TimerLoop timerLoop;

co_async::Task<std::string> reader(int fileNo) {
    co_await wait_file(epollLoop, fileNo, EPOLLIN);
    std::string s;
    size_t chunk = 8;
    while (true) {
        char c;
        size_t exist = s.size();
        s.resize(exist + chunk);
        ssize_t len = read(fileNo, s.data() + exist, chunk);
        if (len == -1) {
            if (errno != EWOULDBLOCK) [[unlikely]] {
                throw std::system_error(errno, std::system_category());
            }
        }
        if (len != chunk) {
            s.resize(exist + len);
            break;
        }
        if (chunk < 65536)
            chunk *= 4;
    }
    co_return s;
}

co_async::Task<void> async_main() {
    int file = co_async::checkError(open("CMakeLists.txt", O_RDONLY | O_NONBLOCK));
    while (true) {
        auto v = co_await when_any(reader(STDIN_FILENO), reader(file));
        std::string s;
        std::visit([&] (std::string const &v) { s = v; }, v);
        debug(), "读到了", s;
        if (s == "quit\n") break;
    }
}

int main() {
    int attr = 1;
    ioctl(0, FIONBIO, &attr);

    auto t = async_main();
    t.mCoroutine.resume();
    while (!t.mCoroutine.done()) {
        if (auto delay = timerLoop.tryRun()) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(*delay).count();
            epollLoop.tryRun(ms);
        } else {
            epollLoop.tryRun(-1);
        }
    }

    return 0;
}

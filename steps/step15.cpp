#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/and_then.hpp"
#include "co_async/error_handling.hpp"
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>

using namespace std::chrono_literals;

namespace co_async {

struct EpollFilePromise : Promise<uint32_t> {
    auto get_return_object() {
        return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
    }

    EpollFilePromise &operator=(EpollFilePromise &&) = delete;

    inline ~EpollFilePromise();

    struct EpollFileAwaiter *mAwaiter;
};

struct EpollLoop {
    inline void addListener(EpollFilePromise &promise);
    inline void removeListener(int fileNo);
    inline void tryRun(std::optional<std::chrono::system_clock::duration> timeout);

    EpollLoop &operator=(EpollLoop &&) = delete;

    ~EpollLoop() {
        close(mEpoll);
    }

    int mEpoll = checkError(epoll_create1(0));
    struct epoll_event mEventBuf[64];
};

struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void
    await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) {
        auto &promise = coroutine.promise();
        promise.mAwaiter = this;
        mLoop.addListener(promise);
    }

    uint32_t await_resume() const noexcept {
        return mResumeEvents;
    }

    using ClockType = std::chrono::system_clock;

    EpollLoop &mLoop;
    int mFileNo;
    uint32_t mEvents;
    uint32_t mResumeEvents;
};

EpollFilePromise::~EpollFilePromise() {
    if (mAwaiter) [[likely]] {
        mAwaiter->mLoop.removeListener(mAwaiter->mFileNo);
    }
}

void EpollLoop::addListener(EpollFilePromise &promise) {
    struct epoll_event event;
    event.events = promise.mAwaiter->mEvents;
    event.data.ptr = &promise;
    checkError(epoll_ctl(mEpoll, EPOLL_CTL_ADD, promise.mAwaiter->mFileNo, &event));
}

void EpollLoop::removeListener(int fileNo) {
    checkError(epoll_ctl(mEpoll, EPOLL_CTL_DEL, fileNo, NULL));
}

void EpollLoop::tryRun(std::optional<std::chrono::system_clock::duration> timeout) {
    int timeoutInMs = -1;
    if (timeout) {
        timeoutInMs = std::chrono::duration_cast<std::chrono::milliseconds>(*timeout).count();
    }
    int res = checkError(epoll_wait(mEpoll, mEventBuf, std::size(mEventBuf), timeoutInMs));
    for (int i = 0; i < res; i++) {
        auto &event = mEventBuf[i];
        auto &promise = *(EpollFilePromise *)event.data.ptr;
        promise.mAwaiter->mResumeEvents = event.events;
    }
    for (int i = 0; i < res; i++) {
        auto &event = mEventBuf[i];
        auto &promise = *(EpollFilePromise *)event.data.ptr;
        std::coroutine_handle<EpollFilePromise>::from_promise(promise).resume();
    }
}

inline Task<uint32_t, EpollFilePromise>
wait_file(EpollLoop &loop, int fileNo, uint32_t events) {
    uint32_t resumeEvents = co_await EpollFileAwaiter(loop, fileNo, events | EPOLLONESHOT);
    co_return resumeEvents;
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
    int file = co_async::checkError(open("/dev/stdin", O_RDONLY | O_NONBLOCK));
    while (true) {
        debug(), "开始读";
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
        auto timeout = timerLoop.tryRun();
        epollLoop.tryRun(timeout);
    }

    return 0;
}

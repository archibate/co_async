#pragma once

#include <coroutine>
#include <chrono>
#include <cstdint>
#include <utility>
#include <optional>
#include <span>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <co_async/task.hpp>
#include <co_async/error_handling.hpp>

namespace co_async {

using EpollEventMask = std::uint32_t;

struct EpollFilePromise : Promise<EpollEventMask> {
    auto get_return_object() {
        return std::coroutine_handle<EpollFilePromise>::from_promise(*this);
    }

    EpollFilePromise &operator=(EpollFilePromise &&) = delete;

    inline ~EpollFilePromise();

    struct EpollFileAwaiter *mAwaiter{};
};

struct EpollLoop {
    inline void addListener(EpollFilePromise &promise, int ctl);
    inline void removeListener(int fileNo);
    inline bool run(std::optional<std::chrono::system_clock::duration> timeout =
                        std::nullopt);

    bool hasEvent() const noexcept {
        return mCount != 0;
    }

    EpollLoop &operator=(EpollLoop &&) = delete;

    ~EpollLoop() {
        close(mEpoll);
    }

    int mEpoll = checkError(epoll_create1(0));
    std::size_t mCount = 0;
    struct epoll_event mEventBuf[64];
};

struct EpollFileAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) {
        auto &promise = coroutine.promise();
        promise.mAwaiter = this;
        mLoop.addListener(promise, mCtlCode);
    }

    EpollEventMask await_resume() const noexcept {
        return mResumeEvents;
    }

    EpollLoop &mLoop;
    int mFileNo;
    EpollEventMask mEvents;
    EpollEventMask mResumeEvents;
    int mCtlCode = EPOLL_CTL_ADD;
};

EpollFilePromise::~EpollFilePromise() {
    if (mAwaiter) [[likely]] {
        mAwaiter->mLoop.removeListener(mAwaiter->mFileNo);
    }
}

void EpollLoop::addListener(EpollFilePromise &promise, int ctl) {
    struct epoll_event event;
    event.events = promise.mAwaiter->mEvents;
    event.data.ptr = &promise;
    checkError(epoll_ctl(mEpoll, ctl, promise.mAwaiter->mFileNo, &event));
    if (ctl == EPOLL_CTL_ADD)
        ++mCount;
}

void EpollLoop::removeListener(int fileNo) {
    checkError(epoll_ctl(mEpoll, EPOLL_CTL_DEL, fileNo, NULL));
    --mCount;
}

bool EpollLoop::run(
    std::optional<std::chrono::system_clock::duration> timeout) {
    if (mCount == 0) {
        return false;
    }
    int timeoutInMs = -1;
    if (timeout) {
        timeoutInMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(*timeout)
                .count();
    }
    int res = checkError(
        epoll_wait(mEpoll, mEventBuf, std::size(mEventBuf), timeoutInMs));
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
    return true;
}

struct AsyncFile {
    AsyncFile() : mFileNo(-1) {}

    explicit AsyncFile(std::in_place_t, int fileNo) noexcept
        : mFileNo(fileNo) {}

    explicit AsyncFile(int fileNo) : mFileNo(dup(fileNo)) {
        checkError(mFileNo);
        ensureNonblock();
    }

    AsyncFile &ensureNonblock() {
        int attr = 1;
        checkError(ioctl(mFileNo, FIONBIO, &attr));
        return *this;
    }

    explicit AsyncFile(char const *path, int oflags)
        : mFileNo(checkError(open(path, oflags | O_NONBLOCK))) {}

    explicit AsyncFile(char const *path, int oflags, mode_t mode)
        : mFileNo(checkError(open(path, oflags | O_NONBLOCK, mode))) {}

    AsyncFile(AsyncFile &&that) noexcept : mFileNo(that.mFileNo) {
        that.mFileNo = -1;
    }

    AsyncFile &operator=(AsyncFile &&that) noexcept {
        std::swap(mFileNo, that.mFileNo);
        return *this;
    }

    ~AsyncFile() {
        if (mFileNo != -1)
            close(mFileNo);
    }

    int fileNo() const noexcept {
        return mFileNo;
    }

    int releaseOwnership() noexcept {
        int ret = mFileNo;
        mFileNo = -1;
        return ret;
    }

private:
    int mFileNo;
};

inline Task<EpollEventMask, EpollFilePromise>
wait_file_event(EpollLoop &loop, AsyncFile &file, EpollEventMask events) {
    co_return co_await EpollFileAwaiter(loop, file.fileNo(), events);
}

inline std::size_t read_file_sync(AsyncFile &file, std::span<char> buffer) {
    ssize_t len = read(file.fileNo(), buffer.data(), buffer.size());
    if (len == -1) {
        if (errno != EWOULDBLOCK) [[unlikely]] {
            throw std::system_error(errno, std::system_category());
        }
        len = 0;
    }
    return len;
}

inline Task<std::string> read_string(EpollLoop &loop, AsyncFile &file) {
    co_await wait_file_event(loop, file, EPOLLIN | EPOLLET);
    std::string s;
    std::size_t chunk = 15;
    while (true) {
        char c;
        std::size_t exist = s.size();
        s.append(chunk, 0);
        std::span<char> buffer(s.data() + exist, chunk);
        auto len = read_file_sync(file, buffer);
        if (len != chunk) {
            s.resize(exist + len);
            break;
        }
        if (chunk < 65536)
            chunk *= 3;
    }
    co_return s;
}

} // namespace co_async

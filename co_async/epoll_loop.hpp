#pragma once

#include <coroutine>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
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

    struct EpollFileAwaiter *mAwaiter;
};

struct EpollLoop {
    inline void addListener(EpollFilePromise &promise);
    inline void removeListener(int fileNo);
    inline void
    runFor(std::optional<std::chrono::system_clock::duration> timeout);

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

    void await_suspend(std::coroutine_handle<EpollFilePromise> coroutine) {
        auto &promise = coroutine.promise();
        promise.mAwaiter = this;
        mLoop.addListener(promise);
    }

    EpollEventMask await_resume() const noexcept {
        return mResumeEvents;
    }

    using ClockType = std::chrono::system_clock;

    EpollLoop &mLoop;
    int mFileNo;
    EpollEventMask mEvents;
    EpollEventMask mResumeEvents;
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
    checkError(
        epoll_ctl(mEpoll, EPOLL_CTL_ADD, promise.mAwaiter->mFileNo, &event));
}

void EpollLoop::removeListener(int fileNo) {
    checkError(epoll_ctl(mEpoll, EPOLL_CTL_DEL, fileNo, NULL));
}

void EpollLoop::runFor(
    std::optional<std::chrono::system_clock::duration> timeout) {
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
}

struct AsyncFile {
    explicit AsyncFile(int mFileNo) {
        int attr = 1;
        checkError(ioctl(mFileNo, FIONBIO, &attr));
    }

    AsyncFile(AsyncFile &&) = delete;

    ~AsyncFile() {
        close(mFileNo);
    }

    int fileNo() const {
        return mFileNo;
    }

private:
    int mFileNo;
};

inline Task<EpollEvent, EpollFilePromise>
wait_file(EpollLoop &loop, AsyncFile &file, EpollEventMask events) {
    co_return co_await EpollFileAwaiter(loop, file.fileNo(), events | EPOLLONESHOT);
}

inline std::size_t read_file(AsyncFile &file, std::span<char> buffer) {
    ssize_t len = read(file.fileNo(), buffer.data(), buffer.size());
    if (len == -1) {
        if (errno != EWOULDBLOCK) [[unlikely]] {
            throw std::system_error(errno, std::system_category());
        }
    }
    return len;
}

} // namespace co_async

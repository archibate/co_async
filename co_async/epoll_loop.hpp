#pragma once

#include <coroutine>
#include <chrono>
#include <cstdint>
#include <utility>
#include <optional>
#include <string>
#include <string_view>
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

    explicit AsyncFile(int fileNo) noexcept : mFileNo(fileNo) {}

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

    void setNonblock() const {
        int attr = 1;
        checkError(ioctl(fileNo(), FIONBIO, &attr));
    }

private:
    int mFileNo;
};

inline Task<EpollEventMask, EpollFilePromise>
wait_file_event(EpollLoop &loop, AsyncFile &file, EpollEventMask events) {
    co_return co_await EpollFileAwaiter(loop, file.fileNo(), events);
}

inline std::size_t readFileSync(AsyncFile &file, std::span<char> buffer) {
    return checkErrorNonBlock(
        read(file.fileNo(), buffer.data(), buffer.size()));
}

inline Task<std::string> read_string(EpollLoop &loop, AsyncFile &file,
                                     std::size_t maxSize = -1,
                                     std::size_t initialChunk = 15,
                                     std::size_t maxChunk = 65536,
                                     std::size_t chunkGrowth = 3) {
    co_await wait_file_event(loop, file, EPOLLIN | EPOLLET);
    std::string s;
    std::size_t chunk = initialChunk;
    while (true) {
        std::size_t exist = s.size();
        if (chunk + exist > maxSize) {
            chunk = maxSize - exist;
        }
        s.append(chunk, 0);
        std::span<char> buffer(s.data() + exist, chunk);
        auto len = readFileSync(file, buffer);
        if (len != chunk) {
            s.resize(exist + len);
            break;
        }
        if (exist + len == maxSize) {
            break;
        }
        chunk *= chunkGrowth;
        if (chunk > maxChunk) {
            chunk = maxChunk;
        }
    }
    co_return s;
}

} // namespace co_async

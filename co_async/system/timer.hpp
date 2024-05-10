#pragma once

#include <co_async/std.hpp>

#ifdef __linux__

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <co_async/awaiter/task.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/threading/future.hpp>

namespace co_async {

struct SleepTimeoutRemovePromise : CustomPromise<void, SleepTimeoutRemovePromise> {
    ~SleepTimeoutRemovePromise() {
        if (mOp) {
            co_spawn(co_bind([op = mOp, flags = mFlags] () -> Task<> {
                 co_await uring_timeout_remove(op, flags);
            }));
        }
    }

    std::suspend_never await_transform(int flags) {
        mFlags = flags;
        return {};
    }

    UringOp &&await_transform(UringOp &&op) {
        mOp = &op;
        return std::move(op);
    }

    std::suspend_never await_transform(std::nullptr_t) {
        mOp = nullptr;
        return {};
    }

    UringOp *mOp = nullptr;
    int mFlags = 0;
};

template <class Rep, class Period>
inline Task<void, SleepTimeoutRemovePromise> sleep_for(std::chrono::duration<Rep, Period> dur) {
    auto ts = durationToKernelTimespec(dur);
    int flags = IORING_TIMEOUT_BOOTTIME | IORING_TIMEOUT_ETIME_SUCCESS;
    co_await flags;
    int ret = co_await uring_timeout(&ts, 1, flags);
    co_await nullptr;
    if (ret != -ETIME && ret != -ECANCELED) [[unlikely]]
        throwingError(ret);
}

template <class Dur>
inline Task<void, SleepTimeoutRemovePromise> sleep_until(std::chrono::time_point<std::chrono::steady_clock, Dur> tp) {
    auto ts = timePointToKernelTimespec(tp);
    int flags = IORING_TIMEOUT_ABS | IORING_TIMEOUT_ETIME_SUCCESS;
    co_await flags;
    int ret = co_await uring_timeout(&ts, 1, flags);
    co_await nullptr;
    if (ret != -ETIME && ret != -ECANCELED) [[unlikely]]
        throwingError(ret);
}

template <class Dur>
inline Task<void, SleepTimeoutRemovePromise> sleep_until(std::chrono::time_point<std::chrono::system_clock, Dur> tp) {
    auto ts = timePointToKernelTimespec(tp);
    int flags = IORING_TIMEOUT_ABS | IORING_TIMEOUT_REALTIME | IORING_TIMEOUT_ETIME_SUCCESS;
    co_await flags;
    int ret = co_await uring_timeout(&ts, 1, flags);
    co_await nullptr;
    if (ret != -ETIME && ret != -ECANCELED) [[unlikely]]
        throwingError(ret);
}

} // namespace co_async

#endif

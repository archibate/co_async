#pragma once

#include <co_async/std.hpp>

#ifdef __linux__

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_any.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/threading/future.hpp>
#include <co_async/threading/cancel.hpp>

namespace co_async {

struct UringTimeoutCanceller {
    using OpType = UringOp;

    static Task<> doCancel(OpType *op) {
        co_await uring_timeout_remove(op, 0);
    }

    static int earlyCancelValue() noexcept {
        return -ECANCELED;
    }
};

template <class Rep, class Period>
inline Task<Expected<>> sleep_for(std::chrono::duration<Rep, Period> dur) {
    auto ts = durationToKernelTimespec(dur);
    int flags = IORING_TIMEOUT_BOOTTIME | IORING_TIMEOUT_ETIME_SUCCESS;
    int ret = co_await uring_timeout(&ts, 1, flags);
    if (ret < 0 && ret != -ETIME) [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc(-ret))};
    } else {
        co_return {};
    }
}

template <class Clk, class Dur>
inline Task<Expected<>> sleep_until(std::chrono::time_point<Clk, Dur> tp) {
    auto ts = timePointToKernelTimespec(tp);
    int flags = IORING_TIMEOUT_ABS | IORING_TIMEOUT_ETIME_SUCCESS;
    if constexpr (!Clk::is_steady) {
        flags |= IORING_TIMEOUT_REALTIME;
    }
    int ret = co_await uring_timeout(&ts, 1, flags);
    if (ret < 0 && ret != -ETIME) [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc(-ret))};
    } else {
        co_return {};
    }
}

template <class Rep, class Period>
inline Task<Expected<>> sleep_for(std::chrono::duration<Rep, Period> dur,
                                  CancelToken cancel) {
    auto ts = durationToKernelTimespec(dur);
    int flags = IORING_TIMEOUT_BOOTTIME | IORING_TIMEOUT_ETIME_SUCCESS;
    auto ret = co_await cancel.invoke<UringTimeoutCanceller>(
        uring_timeout(&ts, 1, flags));
    if (ret < 0 && ret != -ETIME) {
        co_return Unexpected{std::make_error_code(std::errc(-ret))};
    } else {
        co_return {};
    }
}

template <class Clk, class Dur>
inline Task<Expected<>> sleep_until(std::chrono::time_point<Clk, Dur> tp,
                                    CancelToken cancel) {
    auto ts = timePointToKernelTimespec(tp);
    int flags = IORING_TIMEOUT_ABS | IORING_TIMEOUT_ETIME_SUCCESS;
    if constexpr (!Clk::is_steady) {
        flags |= IORING_TIMEOUT_REALTIME;
    }
    auto ret = co_await cancel.invoke<UringTimeoutCanceller>(
        uring_timeout(&ts, 1, flags));
    if (ret < 0 && ret != -ETIME) {
        co_return Unexpected{std::make_error_code(std::errc(-ret))};
    } else {
        co_return {};
    }
}

template <class F, class... Args, class Rep, class Period>
    requires Awaitable<std::invoke_result_t<F, Args..., CancelToken>>
inline Task<std::optional<typename AwaitableTraits<
    std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>>
timeout_for(F &&task, std::chrono::duration<Rep, Period> dur, Args &&...args) {
    CancelSource cs;
    CancelSource ct;
    auto res = co_await when_any(
        co_bind(
            [&]() mutable
            -> Task<typename AwaitableTraits<
                std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType> {
                auto res = (co_await std::invoke(
                                task, std::forward<Args>(args)..., ct),
                            Void());
                co_await cs.cancel();
                co_return res;
            }),
        co_bind([&]() mutable -> Task<> {
            (void)co_await sleep_for(dur, cs);
            co_await ct.cancel();
        }));
    co_return std::get<0>(res);
}

template <class F, class... Args, class Clk, class Dur>
    requires Awaitable<std::invoke_result_t<F, Args..., CancelToken>>
inline Task<std::optional<typename AwaitableTraits<
    std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>>
timeout_until(F &&task, std::chrono::time_point<Clk, Dur> tp, Args &&...args) {
    CancelSource cs;
    CancelSource ct;
    std::optional<typename AwaitableTraits<
        std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>
        result;
    TaskGroup group;
    group.add(co_bind([&]() mutable -> Task<> {
        auto res = (co_await std::invoke(task, std::forward<Args>(args)..., ct),
                    Void());
        co_await cs.cancel();
        if (!ct.is_canceled()) {
            result = res;
        }
    }));
    group.add(co_bind([&]() mutable -> Task<> {
        (void)co_await sleep_until(tp, cs);
        co_await ct.cancel();
    }));
    co_await group.wait();
    co_return result;
}

} // namespace co_async

#endif

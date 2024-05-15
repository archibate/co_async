#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/generic_io.hpp>

namespace co_async {
template <class F, class Timeout, class... Args>
    requires Awaitable<std::invoke_result_t<F, Args..., CancelToken>>
inline Task<std::optional<typename AwaitableTraits<
    std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>>
co_timeout(F &&task, Timeout timeout, Args &&...args) {
    CancelSource cs;
    CancelSource ct;
    std::optional<typename AwaitableTraits<
        std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>
        result;
    co_await when_all(
        co_bind([&]() mutable -> Task<> {
            auto res =
                (co_await std::invoke(task, std::forward<Args>(args)..., ct),
                 Void());
            co_await cs.cancel();
            if (!ct.is_canceled()) {
                result = res;
            }
        }),
        co_bind([&]() mutable -> Task<> {
            (void)co_await co_sleep(timeout, cs);
            co_await ct.cancel();
        }));
    co_return result;
}
} // namespace co_async

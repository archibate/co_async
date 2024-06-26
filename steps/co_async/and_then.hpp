#pragma once

#include <utility>
#include "task.hpp"
#include "concepts.hpp"
#include "make_awaitable.hpp"

namespace co_async {

template <Awaitable A, std::invocable<typename AwaitableTraits<A>::RetType> F>
    requires(!std::same_as<void, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<
    std::invoke_result_t<F, typename AwaitableTraits<A>::RetType>>::Type>
and_then(A &&a, F &&f) {
    co_return co_await make_awaitable(
        std::forward<F>(f)(co_await std::forward<A>(a)));
}

template <Awaitable A, std::invocable<> F>
    requires(std::same_as<void, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<std::invoke_result_t<F>>::Type> and_then(A &&a,
                                                                       F &&f) {
    co_await std::forward<A>(a);
    co_return co_await make_awaitable(std::forward<F>(f)());
}

template <Awaitable A, Awaitable F>
    requires(!std::invocable<F> &&
             !std::invocable<F, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<F>::RetType> and_then(A &&a, F &&f) {
    co_await std::forward<A>(a);
    co_return co_await std::forward<F>(f);
}

} // namespace co_async

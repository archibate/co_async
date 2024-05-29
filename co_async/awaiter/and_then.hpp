#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {
template <Awaitable A, Awaitable F>
    requires(!std::invocable<F> &&
             !std::invocable<F, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<F>::RetType> and_then(A a, F f) {
    co_await std::move(a);
    co_return co_await std::move(f);
}
} // namespace co_async

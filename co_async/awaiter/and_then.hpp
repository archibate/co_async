#pragma once/*{export module co_async:awaiter.and_then;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/awaiter/concepts.hpp>/*{import :awaiter.concepts;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/

namespace co_async {

/*[export]*/ template <Awaitable A, Awaitable F>
    requires(!std::is_invocable_v<F> &&
             !std::is_invocable_v<F, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<F>::RetType> and_then(A a, F f) {
    co_await std::move(a);
    co_return co_await std::move(f);
}

} // namespace co_async

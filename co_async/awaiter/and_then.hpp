#pragma once /*{export module co_async:awaiter.and_then;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/awaiter/concepts.hpp>         /*{import :awaiter.concepts;}*/
#include <co_async/awaiter/task.hpp>             /*{import :awaiter.task;}*/
#include <co_async/awaiter/details/make_awaitable.hpp>/*{import :awaiter.details.make_awaitable;}*/

namespace co_async {

/*[export]*/ template <Awaitable A,
                       std::invocable<typename AwaitableTraits<A>::RetType> F>
    requires(!std::same_as<void, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<
    std::invoke_result_t<F, typename AwaitableTraits<A>::RetType>>::Type>
and_then(A a, F f) {
    co_return co_await ensureAwaitable(
        std::invoke(std::move(f), co_await std::move(a)));
}

/*[export]*/ template <Awaitable A, std::invocable<> F>
    requires(std::same_as<void, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<std::invoke_result_t<F>>::Type> and_then(A a,
                                                                       F f) {
    co_await std::move(a);
    co_return co_await ensureAwaitable(std::invoke(std::move(f)));
}

/*[export]*/ template <Awaitable A, Awaitable F>
    requires(!std::invocable<F> &&
             !std::invocable<F, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<F>::RetType> and_then(A a, F f) {
    co_await std::move(a);
    co_return co_await std::move(f);
}

} // namespace co_async

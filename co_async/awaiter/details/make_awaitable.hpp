#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {
template <Awaitable A>
A ensureAwaitable(A a) {
    return std::move(a);
}

template <class A>
    requires(!Awaitable<A>)
Task<A> ensureAwaitable(A a) {
    co_return std::move(a);
}

template <Awaitable A>
Task<typename AwaitableTraits<A>::RetType> ensureTask(A a) {
    co_return co_await std::move(a);
}

template <class T>
Task<T> ensureTask(Task<T> &&t) {
    return std::move(t);
}

template <class A>
    requires(!Awaitable<A> && std::invocable<A> &&
             Awaitable<std::invoke_result_t<A>>)
Task<typename AwaitableTraits<std::invoke_result_t<A>>::RetType>
ensureTask(A a) {
    return ensureTask(std::invoke(std::move(a)));
}
} // namespace co_async

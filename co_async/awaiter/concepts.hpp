#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {

template <class A>
concept Awaiter = requires(A a, std::coroutine_handle<> h) {
    { a.await_ready() };
    { a.await_suspend(h) };
    { a.await_resume() };
};
template <class A>
concept Awaitable = Awaiter<A> || requires(A a) {
    { a.operator co_await() } -> Awaiter;
};

template <class A>
struct AwaitableTraits {
    using Type = A;
};

template <Awaiter A>
struct AwaitableTraits<A> {
    using RetType = decltype(std::declval<A>().await_resume());
    using AvoidRetType = Avoid<RetType>;
    using Type = RetType;
    using AwaiterType = A;
};

template <class A>
    requires(!Awaiter<A> && Awaitable<A>)
struct AwaitableTraits<A>
    : AwaitableTraits<decltype(std::declval<A>().operator co_await())> {};

template <class... Ts>
struct TypeList {};

template <class Last>
struct TypeList<Last> {
    using FirstType = Last;
    using LastType = Last;
};

template <class First, class... Ts>
struct TypeList<First, Ts...> {
    using FirstType = First;
    using LastType = typename TypeList<Ts...>::LastType;
};

} // namespace co_async

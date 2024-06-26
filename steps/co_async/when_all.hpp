#pragma once

#include <coroutine>
#include <span>
#include <exception>
#include <vector>
#include <tuple>
#include <type_traits>
#include "uninitialized.hpp"
#include "task.hpp"
#include "return_previous.hpp"
#include "concepts.hpp"

namespace co_async {

struct WhenAllCtlBlock {
    std::size_t mCount;
    std::coroutine_handle<> mPrevious{};
    std::exception_ptr mException{};
};

struct WhenAllAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const {
        if (mTasks.empty())
            return coroutine;
        mControl.mPrevious = coroutine;
        for (auto const &t: mTasks.subspan(0, mTasks.size() - 1))
            t.mCoroutine.resume();
        return mTasks.back().mCoroutine;
    }

    void await_resume() const {
        if (mControl.mException) [[unlikely]] {
            std::rethrow_exception(mControl.mException);
        }
    }

    WhenAllCtlBlock &mControl;
    std::span<ReturnPreviousTask const> mTasks;
};

template <class T>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<T> &result) {
    try {
        result.putValue(co_await std::forward<decltype(t)>(t));
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <class = void>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<void> &) {
    try {
        co_await std::forward<decltype(t)>(t);
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <std::size_t... Is, class... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>>
whenAllImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAllCtlBlock control{sizeof...(Ts)};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask taskArray[]{
        whenAllHelper(ts, control, std::get<Is>(result))...};
    co_await WhenAllAwaiter(control, taskArray);
    co_return std::tuple<typename AwaitableTraits<Ts>::NonVoidRetType...>(
        std::get<Is>(result).moveValue()...);
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_all(Ts &&...ts) {
    return whenAllImpl(std::make_index_sequence<sizeof...(Ts)>{},
                       std::forward<Ts>(ts)...);
}

template <Awaitable T, class Alloc = std::allocator<T>>
Task<std::conditional_t<
    std::same_as<void, typename AwaitableTraits<T>::RetType>,
    std::vector<typename AwaitableTraits<T>::RetType, Alloc>, void>>
when_all(std::vector<T, Alloc> const &tasks) {
    WhenAllCtlBlock control{tasks.size()};
    Alloc alloc = tasks.get_allocator();
    std::vector<Uninitialized<typename AwaitableTraits<T>::RetType>, Alloc>
        result(tasks.size(), alloc);
    {
        std::vector<ReturnPreviousTask, Alloc> taskArray(alloc);
        taskArray.reserve(tasks.size());
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            taskArray.push_back(whenAllHelper(tasks[i], control, result[i]));
        }
        co_await WhenAllAwaiter(control, taskArray);
    }
    if constexpr (!std::same_as<void, typename AwaitableTraits<T>::RetType>) {
        std::vector<typename AwaitableTraits<T>::RetType, Alloc> res(alloc);
        res.reserve(tasks.size());
        for (auto &r: result) {
            res.push_back(r.moveValue());
        }
        co_return res;
    }
}

} // namespace co_async

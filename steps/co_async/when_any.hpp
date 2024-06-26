#pragma once

#include "debug.hpp"
#include <coroutine>
#include <span>
#include <exception>
#include <vector>
#include <variant>
#include <type_traits>
#include "uninitialized.hpp"
#include "task.hpp"
#include "return_previous.hpp"
#include "concepts.hpp"

namespace co_async {

struct WhenAnyCtlBlock {
    static constexpr std::size_t kNullIndex = std::size_t(-1);

    std::size_t mIndex{kNullIndex};
    std::coroutine_handle<> mPrevious{};
    std::exception_ptr mException{};
};

struct WhenAnyAwaiter {
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

    WhenAnyCtlBlock &mControl;
    std::span<ReturnPreviousTask const> mTasks;
};

template <class T>
ReturnPreviousTask whenAnyHelper(auto &&t, WhenAnyCtlBlock &control,
                                 Uninitialized<T> &result, std::size_t index) {
    try {
        result.putValue(
            (co_await std::forward<decltype(t)>(t), NonVoidHelper<>()));
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
    control.mIndex = index;
    co_return control.mPrevious;
}

template <std::size_t... Is, class... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...>>
whenAnyImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAnyCtlBlock control{};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask taskArray[]{
        whenAnyHelper(ts, control, std::get<Is>(result), Is)...};
    co_await WhenAnyAwaiter(control, taskArray);
    Uninitialized<std::variant<typename AwaitableTraits<Ts>::NonVoidRetType...>>
        varResult;
    ((control.mIndex == Is &&
      (varResult.putValue(std::in_place_index<Is>,
                          std::get<Is>(result).moveValue()),
       0)),
     ...);
    co_return varResult.moveValue();
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_any(Ts &&...ts) {
    return whenAnyImpl(std::make_index_sequence<sizeof...(Ts)>{},
                       std::forward<Ts>(ts)...);
}

template <Awaitable T, class Alloc = std::allocator<T>>
Task<typename AwaitableTraits<T>::RetType>
when_any(std::vector<T, Alloc> const &tasks) {
    WhenAnyCtlBlock control{tasks.size()};
    Alloc alloc = tasks.get_allocator();
    Uninitialized<typename AwaitableTraits<T>::RetType> result;
    {
        std::vector<ReturnPreviousTask, Alloc> taskArray(alloc);
        taskArray.reserve(tasks.size());
        for (auto &task: tasks) {
            taskArray.push_back(whenAllHelper(task, control, result));
        }
        co_await WhenAnyAwaiter(control, taskArray);
    }
    co_return result.moveValue();
}

} // namespace co_async

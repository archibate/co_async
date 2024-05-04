#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/details/return_previous.hpp>
#include <co_async/awaiter/concepts.hpp>

namespace co_async {

struct WhenAnyCtlBlock {
    static constexpr std::size_t kNullIndex = std::size_t(-1);

    std::size_t mIndex{kNullIndex};
    std::coroutine_handle<> mPrevious{};
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif
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
            t.get().resume();
        return mTasks.back().get();
    }

    void await_resume() const {
#if CO_ASYNC_EXCEPT
        if (mControl.mException) [[unlikely]] {
            std::rethrow_exception(mControl.mException);
        }
#endif
    }

    WhenAnyCtlBlock &mControl;
    std::span<ReturnPreviousTask const> mTasks;
};

template <class T>
ReturnPreviousTask whenAnyHelper(auto &&t, WhenAnyCtlBlock &control,
                                 Uninitialized<T> &result, std::size_t index) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.putValue((co_await std::forward<decltype(t)>(t), Void()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    control.mIndex = index;
    co_return control.mPrevious;
}

template <std::size_t... Is, class... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
whenAnyImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAnyCtlBlock control{};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask taskArray[]{
        whenAnyHelper(ts, control, std::get<Is>(result), Is)...};
    co_await WhenAnyAwaiter(control, taskArray);
    Uninitialized<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
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
        std::vector<ReturnPreviousTask,
                    typename std::allocator_traits<
                        Alloc>::template rebind_alloc<ReturnPreviousTask>>
            taskArray(alloc);
        taskArray.reserve(tasks.size());
        for (auto &task: tasks) {
            taskArray.push_back(whenAllHelper(task, control, result));
        }
        co_await WhenAnyAwaiter(control, taskArray);
    }
    co_return result.moveValue();
}

} // namespace co_async

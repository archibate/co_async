#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/details/return_previous.hpp>
#include <co_async/awaiter/concepts.hpp>

namespace co_async {

struct WhenAnyCtlBlock {
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

template <std::size_t I, class T>
ReturnPreviousTask whenAnyHelper(auto &&t, WhenAnyCtlBlock &control,
                                 Uninitialized<T> &result, std::atomic_bool &outReady) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        bool notReady{false};
        if (outReady.compare_exchange_strong(notReady, true, std::memory_order_acq_rel)) {
            result.putValue(std::in_place_index<I>, (co_await std::forward<decltype(t)>(t), Void()));
        }
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    co_return control.mPrevious;
}

template <class T>
ReturnPreviousTask whenAnyHelper(auto &&t, WhenAnyCtlBlock &control,
                                 Uninitialized<T> &result, std::size_t index, std::atomic_size_t &outIndex) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        auto res = (co_await std::forward<decltype(t)>(t), Void());
        std::size_t nullIndex(-1);
        if (outIndex.compare_exchange_strong(nullIndex, index, std::memory_order_acq_rel)) {
            result.putValue(std::move(res));
        }
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    co_return control.mPrevious;
}

template <std::size_t... Is, class... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
whenAnyImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAnyCtlBlock control{};
    Uninitialized<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>> result;
    std::atomic_bool outReady{false};
    ReturnPreviousTask taskArray[]{whenAnyHelper<Is>(ts, control, result, outReady)...};
    co_await WhenAnyAwaiter(control, taskArray);
    co_return result.moveValue();
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_any(Ts &&...ts) {
    return whenAnyImpl(std::make_index_sequence<sizeof...(Ts)>{},
                       std::forward<Ts>(ts)...);
}

template <Awaitable T, class Alloc = std::allocator<T>>
Task<std::tuple<typename AwaitableTraits<T>::RetType, std::size_t>>
when_any(std::vector<T, Alloc> const &tasks) {
    WhenAnyCtlBlock control{tasks.size()};
    Alloc alloc = tasks.get_allocator();
    Uninitialized<typename AwaitableTraits<T>::RetType> result;
    std::size_t id;
    {
        std::vector<ReturnPreviousTask,
                    typename std::allocator_traits<
                        Alloc>::template rebind_alloc<ReturnPreviousTask>>
            taskArray(alloc);
        taskArray.reserve(tasks.size());
        std::size_t index = 0;
        std::atomic_size_t outIndex{std::size_t(-1)};
        for (auto &task: tasks) {
            taskArray.push_back(whenAllHelper(task, control, result, index, outIndex));
        }
        co_await WhenAnyAwaiter(control, taskArray);
        id = outIndex.load(std::memory_order_relaxed);
    }
    co_return result.moveValue(), id;
}

} // namespace co_async

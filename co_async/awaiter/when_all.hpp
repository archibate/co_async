#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/details/return_previous.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {
struct WhenAllCtlBlock {
    std::size_t mCount;
    std::coroutine_handle<> mPrevious{};
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif
};

struct WhenAllAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const {
        if (mTasks.empty()) {
            return coroutine;
        }
        mControl.mPrevious = coroutine;
        for (auto const &t: mTasks.subspan(0, mTasks.size() - 1)) {
            t.get().resume();
        }
        return mTasks.back().get();
    }

    void await_resume() const {
#if CO_ASYNC_EXCEPT
        if (mControl.mException) [[unlikely]] {
            std::rethrow_exception(mControl.mException);
        }
#endif
    }

    WhenAllCtlBlock &mControl;
    std::span<ReturnPreviousTask const> mTasks;
};

template <class T>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<T> &result) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.emplace(co_await std::forward<decltype(t)>(t));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <class = void>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<void> &) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        co_await std::forward<decltype(t)>(t);
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <std::size_t... Is, class... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::AvoidRetType...>>
whenAllImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAllCtlBlock control{sizeof...(Ts)};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask taskArray[]{
        whenAllHelper(ts, control, std::get<Is>(result))...};
    co_await WhenAllAwaiter(control, taskArray);
    co_return std::tuple<typename AwaitableTraits<Ts>::AvoidRetType...>(
        std::get<Is>(result).move()...);
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_all(Ts &&...ts) {
    return whenAllImpl(std::make_index_sequence<sizeof...(Ts)>{},
                       std::forward<Ts>(ts)...);
}

template <Awaitable T, class Alloc = std::allocator<T>>
Task<std::conditional_t<
    !std::is_void_v<typename AwaitableTraits<T>::RetType>,
    std::vector<typename AwaitableTraits<T>::RetType,
                typename std::allocator_traits<Alloc>::template rebind_alloc<
                    typename AwaitableTraits<T>::RetType>>,
    void>>
when_all(std::vector<T, Alloc> const &tasks) {
    WhenAllCtlBlock control{tasks.size()};
    Alloc alloc = tasks.get_allocator();
    std::vector<Uninitialized<typename AwaitableTraits<T>::RetType>,
                typename std::allocator_traits<Alloc>::template rebind_alloc<
                    Uninitialized<typename AwaitableTraits<T>::RetType>>>
        result(tasks.size(), alloc);
    {
        std::vector<ReturnPreviousTask,
                    typename std::allocator_traits<
                        Alloc>::template rebind_alloc<ReturnPreviousTask>>
            taskArray(alloc);
        taskArray.reserve(tasks.size());
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            taskArray.push_back(whenAllHelper(tasks[i], control, result[i]));
        }
        co_await WhenAllAwaiter(control, taskArray);
    }
    if constexpr (!std::is_void_v<typename AwaitableTraits<T>::RetType>) {
        std::vector<
            typename AwaitableTraits<T>::RetType,
            typename std::allocator_traits<Alloc>::template rebind_alloc<
                typename AwaitableTraits<T>::RetType>>
            res(alloc);
        res.reserve(tasks.size());
        for (auto &r: result) {
            res.push_back(r.move());
        }
        co_return res;
    }
}
} // namespace co_async

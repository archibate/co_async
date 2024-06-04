#pragma once
#include <co_async/std.hpp>
#include "co_async/utils/debug.hpp"
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/generic/cancel.hpp>

namespace co_async {

struct [[nodiscard("did you forgot to co_await?")]] GetIOCancel {
    explicit GetIOCancel() = default;
};

template <class T>
struct IOTaskPromise : TaskPromise<T> {
    auto get_return_object() {
        return std::coroutine_handle<IOTaskPromise>::from_promise(*this);
    }

    template <class U>
    Task<U, IOTaskPromise<U>> &&
    await_transform(Task<U, IOTaskPromise<U>> &&task) noexcept {
        task.get().promise().mCancel = mCancel;
        return std::move(task);
    }

    template <class U>
    Task<U, IOTaskPromise<U>> const &
    await_transform(Task<U, IOTaskPromise<U>> const &task) noexcept {
        task.get().promise().mCancel = mCancel;
        return task;
    }

    decltype(auto) await_transform(CancelToken cancel) noexcept {
        return TaskPromise<T>::await_transform(cancel.expect());
    }

    ValueAwaiter<CancelToken> await_transform(GetIOCancel) noexcept {
        return ValueAwaiter<CancelToken>(mCancel);
    }

    using TaskPromise<T>::await_transform;

    void setCancelToken(CancelToken cancel) {
        mCancel = cancel;
    }

    CancelToken mCancel;

#if CO_ASYNC_PERF
    Perf mPerf;

    IOTaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T = void>
using IOTask = Task<T, IOTaskPromise<T>>;

inline GetIOCancel co_cancel() noexcept {
    return GetIOCancel();
}

template <class... Fs>
    requires(Awaitable<std::invoke_result_t<Fs, CancelToken>> && ...)
inline Task<std::variant<typename AwaitableTraits<
    std::invoke_result_t<Fs, CancelToken>>::AvoidRetType...>>
whenAnyExplicitCancel(Fs &&...tasks) {
    CancelSource cancel;
    std::optional<std::variant<typename AwaitableTraits<std::invoke_result_t<
                                     Fs, CancelToken>>::AvoidRetType...>>
        result;
    co_await [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return when_all(co_bind([&, cancel = cancel.token()]() mutable -> Task<> {
            auto res = (co_await std::invoke(tasks, cancel), Void());
            if (cancel.is_canceled())
                co_return;
            co_await cancel.cancel();
            result.emplace(std::in_place_index<Is>, std::move(res));
        })...);
    }(std::make_index_sequence<sizeof...(Fs)>());
    co_return result.value();
}

template <Awaitable... Tasks> requires (Cancellable<Tasks> && ...)
inline IOTask<std::variant<typename AwaitableTraits<Tasks>::AvoidRetType...>>
when_any(Tasks &&...tasks) {
    CancelDerived cancel(co_await co_cancel());
    std::optional<std::variant<typename AwaitableTraits<Tasks>::AvoidRetType...>>
        result;
    co_await [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return when_all(co_bind([&, cancel = cancel.token()]() mutable -> Task<> {
            auto res = (co_await cancel.guard(std::forward<Tasks>(tasks)), Void());
            if (cancel.is_canceled())
                co_return;
            co_await cancel.cancel();
            result.emplace(std::in_place_index<Is>, std::move(res));
        })...);
    }(std::make_index_sequence<sizeof...(Tasks)>());
    co_return result.value();
}

}

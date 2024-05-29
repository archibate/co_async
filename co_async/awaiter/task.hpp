#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/uninitialized.hpp>
#if CO_ASYNC_PERF
# include <co_async/utils/perf.hpp>
#endif
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/details/previous_awaiter.hpp>
#include <co_async/awaiter/details/value_awaiter.hpp>

namespace co_async {
template <class T>
struct TaskAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) noexcept {
        mCallerCoroutine = coroutine;
        return mCalleeCoroutine;
    }

    T await_resume() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        if constexpr (!std::is_void_v<T>) {
            return mResult.moveValue();
        }
    }

    template <class P>
    explicit TaskAwaiter(std::coroutine_handle<P> coroutine)
        : mCalleeCoroutine(coroutine) {
        coroutine.promise().mAwaiter = this;
    }

    TaskAwaiter(TaskAwaiter &&that) = delete;

    template <class U>
    void returnValue(U &&result) {
        mResult.putValue(std::forward<U>(result));
    }

    void returnVoid() {
        mResult.putValue();
    }

    void unhandledException() noexcept {
#if CO_ASYNC_EXCEPT
        mException = std::current_exception();
#else
        std::terminate();
#endif
    }


    std::coroutine_handle<> callerCoroutine() const noexcept {
        return mCallerCoroutine;
    }

private:
    std::coroutine_handle<> mCallerCoroutine;
    std::coroutine_handle<> mCalleeCoroutine;
    Uninitialized<T> mResult;
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
};

struct TaskFinalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    template <class P>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<P> coroutine) const noexcept {
        return coroutine.promise().mAwaiter->callerCoroutine();
    }

    void await_resume() const noexcept {}
};

template <class T>
struct TaskPromise {
    void return_value(T &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<T> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    void unhandled_exception() noexcept {
        mAwaiter->unhandledException();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

    std::suspend_always initial_suspend() noexcept {
        return {};
    }

    TaskFinalAwaiter final_suspend() noexcept {
        return {};
    }

    TaskPromise &operator=(TaskPromise &&) = delete;

    TaskAwaiter<T> *mAwaiter;

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <>
struct TaskPromise<void> {
    void return_void() {
        mAwaiter->returnVoid();
    }

    void unhandled_exception() noexcept {
        mAwaiter->unhandledException();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return TaskFinalAwaiter();
    }

    TaskPromise &operator=(TaskPromise &&) = delete;

    TaskAwaiter<void> *mAwaiter;

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class E>
struct TaskPromise<Expected<T, E>> {
    void return_value(Expected<T, E> &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(Expected<T, E> const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<Expected<T, E>> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    void unhandled_exception() noexcept {
        mAwaiter->unhandledException();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            if constexpr (std::is_void_v<E>) {
                mAwaiter->returnValue(Unexpected<E>());
            } else {
                static_assert(std::same_as<E2, E>,
                              "co_await'ing Expected's error type mismatch");
                mAwaiter->returnValue(Unexpected<E>(std::move(e.error())));
            }
            return ValueAwaiter<T2>(mAwaiter->callerCoroutine());
        }
        if constexpr (std::is_void_v<T2>) {
            return ValueAwaiter<void>(std::in_place);
        } else {
            return ValueAwaiter<T2>(std::in_place, *std::move(e));
        }
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class E2>
    ValueAwaiter<void>
    await_transform(std::vector<Expected<void, E2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                if constexpr (std::is_void_v<E>) {
                    mAwaiter->returnValue(Unexpected<E>());
                } else {
                    static_assert(
                        std::same_as<E2, E>,
                        "co_await'ing Expected's error type mismatch");
                    mAwaiter->returnValue(Unexpected<E>(std::move(e[i].error())));
                }
                return ValueAwaiter<void>(mAwaiter->callerCoroutine());
            }
        }
        return ValueAwaiter<void>(std::in_place);
    }

    template <class T2, class E2>
    ValueAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2, E2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                if constexpr (std::is_void_v<E>) {
                    mAwaiter->returnValue(Unexpected<E>());
                } else {
                    static_assert(
                        std::same_as<E2, E>,
                        "co_await'ing Expected's error type mismatch");
                    mAwaiter->returnValue(Unexpected<E>(std::move(e[i].error())));
                }
                return ValueAwaiter<std::vector<T2>>(mAwaiter->callerCoroutine());
            }
        }
        std::vector<T2> ret;
        ret.reserve(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) {
            ret.emplace_back(*std::move(e[i]));
        }
        return ValueAwaiter<std::vector<T2>>(std::in_place, std::move(ret));
    }

    template <class T2, class E2>
    ValueAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2, E2>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class... Ts, class E2>
    ValueAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts, E2>...> &&e) noexcept {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            if (!([&] {
                    if (std::get<Is>(e).has_error()) [[unlikely]] {
                        if constexpr (std::is_void_v<E>) {
                            mAwaiter->returnValue(Unexpected<E>());
                        } else {
                            static_assert(
                                std::same_as<E2, E>,
                                "co_await'ing Expected's error type mismatch");
                            mAwaiter->returnValue(Unexpected<E>(
                                std::move(std::get<Is>(e).error())));
                        }
                        return false;
                    }
                    return true;
                }() &&
                  ...)) {
                return ValueAwaiter<std::tuple<Avoid<Ts>...>>(mAwaiter->callerCoroutine());
            }
            return ValueAwaiter<std::tuple<Avoid<Ts>...>>(
                std::in_place, [&]() -> decltype(auto) {
                    return *std::move(std::get<Is>(e)), Void();
                }()...);
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <class... Ts, class E2>
    ValueAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts, E2>...> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class U>
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }

    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return TaskFinalAwaiter();
    }

    TaskPromise &operator=(TaskPromise &&) = delete;

    TaskAwaiter<Expected<T, E>> *mAwaiter;

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class P>
struct CustomPromise : TaskPromise<T> {
    auto get_return_object() {
        static_assert(std::is_base_of_v<CustomPromise, P>);
        return std::coroutine_handle<P>::from_promise(static_cast<P &>(*this));
    }
};

template <class T = void, class P = TaskPromise<T>>
struct [[nodiscard("did you forgot to co_await?")]] Task {
    using promise_type = P;

    /* Task(std::coroutine_handle<promise_type> coroutine) noexcept */
    /*     : mCoroutine(coroutine) { */
    /* } */
    /* Task(Task &&) = delete; */
    Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    Task(Task &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    Task &operator=(Task &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~Task() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }

    auto operator co_await() const noexcept {
        return TaskAwaiter<T>(mCoroutine);
    }

    std::coroutine_handle<promise_type> get() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> release() noexcept {
        return std::exchange(mCoroutine, nullptr);
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

template <class F, class... Args>
    requires(Awaitable<std::invoke_result_t<F, Args...>>)
inline auto co_bind(F &&f, Args &&...args) {
    return [](auto f) mutable -> std::invoke_result_t<F, Args...> {
        co_return co_await std::move(f)();
        /* std::optional o(std::move(f)); */
        /* decltype(auto) r = co_await std::move(*o)(); */
        /* o.reset(); */
        /* co_return */
        /*     typename AwaitableTraits<std::invoke_result_t<F,
         * Args...>>::RetType( */
        /*         std::forward<decltype(r)>(r)); */
    }(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}
} // namespace co_async

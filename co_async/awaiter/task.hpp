#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>
#if CO_ASYNC_PERF
# include <co_async/utils/perf.hpp>
#endif
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/details/previous_awaiter.hpp>
#include <co_async/awaiter/details/value_awaiter.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {

struct CancelSourceImpl;

template <class T>
struct TaskAwaiter {
    bool await_ready() const noexcept {
#if CO_ASYNC_SAFERET
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            return true;
        }
#endif
        return mResult.has_value();
#else
        return false;
#endif
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
#if CO_ASYNC_SAFERET
#if CO_ASYNC_DEBUG
            if constexpr (std::same_as<Expected<>, T>) {
            }
            return std::move(mResult.value());
#else
            return std::move(*mResult);
#endif
#else
            return mResult.move();
#endif
        }
    }

    template <class P>
    explicit TaskAwaiter(std::coroutine_handle<P> coroutine)
        : mCalleeCoroutine(coroutine) {
#if CO_ASYNC_DEBUG
        assert(!coroutine.promise().mAwaiter);
#endif
        coroutine.promise().mAwaiter = this;
    }

    TaskAwaiter(TaskAwaiter &&that) = delete;

    template <class U>
    void returnValue(U &&result) {
        mResult.emplace(std::forward<U>(result));
    }

    void returnVoid() {
        mResult.emplace();
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

protected:
    std::coroutine_handle<> mCallerCoroutine;
    std::coroutine_handle<> mCalleeCoroutine;
#if CO_ASYNC_SAFERET
    std::optional<Avoid<T>> mResult;
#else
    Uninitialized<Avoid<T>> mResult;
#endif
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
};

template <class T, class E>
struct GeneratorAwaiter { // TODO: support generator
    bool await_ready() const noexcept {
#if CO_ASYNC_SAFERET
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            return true;
        }
#endif
        return mResult.has_value();
#else
        return false;
#endif
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
#if CO_ASYNC_SAFERET
#if CO_ASYNC_DEBUG
            return std::move(mResult.value());
#else
            return std::move(*mResult);
#endif
#else
            return mResult.move();
#endif
        }
    }

    template <class P>
    explicit GeneratorAwaiter(std::coroutine_handle<P> coroutine)
        : mCalleeCoroutine(coroutine) {
        coroutine.promise().mAwaiter = this;
    }

    GeneratorAwaiter(GeneratorAwaiter &&that) = delete;

    template <class U>
    void yieldValue(U &&result) {
        mResult.emplace(std::forward<U>(result));
    }

    void returnVoid() {
        mResult.emplace(std::errc::broken_pipe);
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

protected:
    std::coroutine_handle<> mCallerCoroutine;
    std::coroutine_handle<> mCalleeCoroutine;
#if CO_ASYNC_SAFERET
    std::optional<Expected<T>> mResult;
#else
    Uninitialized<Expected<T>> mResult;
#endif
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
};

template <class T>
struct TaskOwnedAwaiter : TaskAwaiter<T> {
    using TaskAwaiter<T>::TaskAwaiter;

    ~TaskOwnedAwaiter() {
        TaskAwaiter<T>::mCalleeCoroutine.destroy();
    }
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
struct TaskPromise;

template <class T = void, class P = TaskPromise<T>>
struct [[nodiscard("did you forgot to co_await?")]] Task {
    using promise_type = P;

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

    auto operator co_await() const & noexcept {
        return TaskAwaiter<T>(mCoroutine);
    }

    auto operator co_await() && noexcept {
        return TaskOwnedAwaiter<T>(std::exchange(mCoroutine, nullptr));
    }

    std::coroutine_handle<promise_type> get() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> release() noexcept {
        return std::exchange(mCoroutine, nullptr);
    }

    promise_type &promise() const {
        return mCoroutine.promise();
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
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

    template <class U>
    Task<U> &&await_transform(Task<U> &&u) noexcept {
        u.promise().mCancelToken = mCancelToken;
        return std::move(u);
    }

    template <class U>
    Task<U> const &await_transform(Task<U> const &u) noexcept {
        u.promise().mCancelToken = mCancelToken;
        return u;
    }

    template <std::invocable<TaskPromise &> U>
    auto await_transform(U &&u) noexcept {
        return await_transform(u(*this));
    }

    template <class U>
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }

    TaskPromise() = default;
    TaskPromise(TaskPromise &&) = delete;

    TaskAwaiter<T> *mAwaiter{};
    CancelSourceImpl *mCancelToken{};

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

    template <class U>
    Task<U> &&await_transform(Task<U> &&u) noexcept {
        u.promise().mCancelToken = mCancelToken;
        return std::move(u);
    }

    template <class U>
    Task<U> const &await_transform(Task<U> const &u) noexcept {
        u.promise().mCancelToken = mCancelToken;
        return u;
    }

    template <std::invocable<TaskPromise &> U>
    auto await_transform(U &&u) noexcept {
        return await_transform(u(*this));
    }

    template <class U>
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }

    TaskPromise() = default;
    TaskPromise(TaskPromise &&) = delete;

    TaskAwaiter<void> *mAwaiter{};
    CancelSourceImpl *mCancelToken{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T>
struct TaskPromise<Expected<T>> {
    void return_value(Expected<T> &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(Expected<T> const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<Expected<T>> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    void unhandled_exception() noexcept {
        mAwaiter->unhandledException();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            mAwaiter->returnValue(std::move(e).error());
            return ValueOrReturnAwaiter<T2>(mAwaiter->callerCoroutine());
        }
        if constexpr (std::is_void_v<T2>) {
            return ValueOrReturnAwaiter<void>(std::in_place);
        } else {
            return ValueOrReturnAwaiter<T2>(std::in_place, *std::move(e));
        }
    }

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &e) noexcept {
        return await_transform(std::move(e));
    }

    ValueOrReturnAwaiter<void>
    await_transform(std::vector<Expected<void>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                mAwaiter->returnValue(std::move(e[i]).error());
                return ValueOrReturnAwaiter<void>(mAwaiter->callerCoroutine());
            }
        }
        return ValueOrReturnAwaiter<void>(std::in_place);
    }

    ValueOrReturnAwaiter<void>
    await_transform(std::vector<Expected<void>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                    mAwaiter->returnValue(std::move(e[i]).error());
                return ValueOrReturnAwaiter<std::vector<T2>>(mAwaiter->callerCoroutine());
            }
        }
        std::vector<T2> ret;
        ret.reserve(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) {
            ret.emplace_back(*std::move(e[i]));
        }
        return ValueOrReturnAwaiter<std::vector<T2>>(std::in_place, std::move(ret));
    }

    template <class T2>
    ValueOrReturnAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &&e) noexcept {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            if (!([&] {
                    if (std::get<Is>(e).has_error()) [[unlikely]] {
                        mAwaiter->returnValue(std::move(std::get<Is>(e)).error());
                        return false;
                    }
                    return true;
                }() &&
                  ...)) {
                return ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>(mAwaiter->callerCoroutine());
            }
            return ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>(
                std::in_place, [&]() -> decltype(auto) {
                    return *std::move(std::get<Is>(e)), Void();
                }()...);
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class U>
    Task<U> &&await_transform(Task<U> &&u) noexcept {
#if CO_ASYNC_DEBUG
        assert(!mCancelToken);
#endif
        u.promise().mCancelToken = mCancelToken;
        return std::move(u);
    }

    template <class U>
    Task<U> const &await_transform(Task<U> const &u) noexcept {
#if CO_ASYNC_DEBUG
        assert(!mCancelToken);
#endif
        u.promise().mCancelToken = mCancelToken;
        return u;
    }

    template <std::invocable<TaskPromise &> U>
    auto await_transform(U &&u) noexcept {
        return await_transform(u(*this));
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

    TaskPromise() = default;
    TaskPromise(TaskPromise &&) = delete;

    TaskAwaiter<Expected<T>> *mAwaiter{};
    CancelSourceImpl *mCancelToken{};

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

template <class F, class... Args>
    requires(Awaitable<std::invoke_result_t<F, Args...>>)
inline std::invoke_result_t<F, Args...> co_bind(F f, Args ...args) {
    return (+[](F f, Args ...args) mutable -> std::invoke_result_t<F, Args...> {
        co_return co_await std::move(f)(std::move(args)...);
        /* std::optional o(std::move(f)); */
        /* decltype(auto) r = co_await std::move(*o)(); */
        /* o.reset(); */
        /* co_return */
        /*     typename AwaitableTraits<std::invoke_result_t<F,
         * Args...>>::RetType( */
        /*         std::forward<decltype(r)>(r)); */
    })(std::move(f), std::move(args)...);
}

} // namespace co_async

#define co_awaits co_await co_await
#define co_returns co_return {}

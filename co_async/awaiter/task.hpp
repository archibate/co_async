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

/* template <class T, class Final = void> */
/* struct Generative { */
/*     explicit operator bool() const noexcept { */
/*     } */
/* }; */

template <class T, class E = void>
struct GeneratorResult {
    std::variant<T, E> mValue;

    explicit GeneratorResult(std::in_place_index_t<0>, auto &&...args)
    : mValue(std::in_place_index<0>, std::forward<decltype(args)>(args)...)
    {}

    explicit GeneratorResult(std::in_place_index_t<1>, auto &&...args)
    : mValue(std::in_place_index<1>, std::forward<decltype(args)>(args)...)
    {}

    /* GeneratorResult(std::convertible_to<T> auto &&value) */

    bool has_result() const noexcept {
        return mValue.index() == 1;
    }

    bool has_value() const noexcept {
        return mValue.index() == 0;
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T &operator*() & noexcept {
        return *std::get_if<0>(&mValue);
    }

    T &&operator*() && noexcept {
        return std::move(*std::get_if<0>(&mValue));
    }

    T const &operator*() const & noexcept {
        return *std::get_if<0>(&mValue);
    }

    T const &&operator*() const && noexcept {
        return std::move(*std::get_if<0>(&mValue));
    }

    T &operator->() noexcept {
        return std::get_if<0>(&mValue);
    }

    T &value() & {
        return std::get<0>(mValue);
    }

    T &&value() && {
        return std::move(std::get<0>(mValue));
    }

    T const &value() const & {
        return std::get<0>(mValue);
    }

    T const &&value() const && {
        return std::move(std::get<0>(mValue));
    }

    E &result_unsafe() & noexcept {
        return *std::get_if<1>(&mValue);
    }

    E &&result_unsafe() && noexcept {
        return std::move(*std::get_if<1>(&mValue));
    }

    E const &result_unsafe() const & noexcept {
        return *std::get_if<1>(&mValue);
    }

    E const &&result_unsafe() const && noexcept {
        return std::move(*std::get_if<1>(&mValue));
    }

    E &result() & {
        return std::get<1>(mValue);
    }

    E &&result() && {
        return std::move(std::get<1>(mValue));
    }

    E const &result() const & {
        return std::get<1>(mValue);
    }

    E const &&result() const && {
        return std::move(std::get<1>(mValue));
    }
};

template <class T>
struct GeneratorResult<T, void> : GeneratorResult<T, Void> {
    using GeneratorResult<T, Void>::GeneratorResult;
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

struct TaskYieldAwaiter {
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

template <class T, class E>
struct TaskAwaiter<GeneratorResult<T, E>> {
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

    GeneratorResult<T, E> await_resume() {
#if CO_ASYNC_DEBUG
        mCalleeCoroutine.promise().mAwaiter = nullptr;
#endif
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
#if CO_ASYNC_SAFERET
#if CO_ASYNC_DEBUG
        GeneratorResult<T, E> ret = std::move(mResult.value());
        mResult.reset();
#else
        GeneratorResult<T, E> ret = std::move(*mResult);
#endif
        mResult.reset();
        return ret;
#else
        return mResult.move();
#endif
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
    TaskYieldAwaiter yieldValue(U &&value) {
        mResult.emplace(std::in_place_index<0>, std::forward<U>(value));
        return TaskYieldAwaiter();
    }

    template <class U>
    void returnValue(U &&result) {
        mResult.emplace(std::in_place_index<1>, std::forward<U>(result));
    }

    void returnVoid() {
        mResult.emplace(std::in_place_index<1>);
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
    std::optional<GeneratorResult<T, E>> mResult;
#else
    Uninitialized<GeneratorResult<T, E>> mResult;
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
        return *this;
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

template <class TaskPromise>
struct TaskPromiseCommon {
    TaskPromise &self() noexcept { return static_cast<TaskPromise &>(*this); }
    TaskPromise const &self() const noexcept { return static_cast<TaskPromise const &>(*this); }

    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return TaskFinalAwaiter();
    }

    void unhandled_exception() noexcept {
        self().mAwaiter->unhandledException();
    }

    TaskPromiseCommon() = default;
    TaskPromiseCommon(TaskPromiseCommon &&) = delete;

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(self());
    }
};

template <class TaskPromise>
struct TaskPromiseExpectedTransforms {
    TaskPromise &self() noexcept { return static_cast<TaskPromise &>(*this); }
    TaskPromise const &self() const noexcept { return static_cast<TaskPromise const &>(*this); }

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            self().mAwaiter->returnValue(std::move(e).error());
            return {self().mAwaiter->callerCoroutine()};
        }
        if constexpr (std::is_void_v<T2>) {
            return {std::in_place};
        } else {
            return {std::in_place, *std::move(e)};
        }
    }

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<GeneratorResult<T2, E2>> await_transform(GeneratorResult<T2, Expected<E2>> &&g) noexcept {
        if (g.has_value()) {
            if constexpr (std::is_void_v<T2>) {
                return {std::in_place, std::in_place_index<0>};
            } else {
                return {std::in_place, std::in_place_index<0>, std::move(*g)};
            }
        } else {
            auto e = g.result_unsafe();
            if (e.has_error()) [[unlikely]] {
                self().mAwaiter->returnValue(std::move(e).error());
                return {self().mAwaiter->callerCoroutine()};
            } else {
                if constexpr (std::is_void_v<E2>) {
                    return {std::in_place, std::in_place_index<1>, std::move(*e)};
                } else {
                    return {std::in_place, std::in_place_index<1>};
                }
            }
        }
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<GeneratorResult<T2, E2>> await_transform(GeneratorResult<T2, Expected<E2>> &g) noexcept {
        return await_transform(std::move(g));
    }

    ValueOrReturnAwaiter<void>
    await_transform(std::vector<Expected<void>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                self().mAwaiter->returnValue(std::move(e[i]).error());
                return {self().mAwaiter->callerCoroutine()};
            }
        }
        return {std::in_place};
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
                    self().mAwaiter->returnValue(std::move(e[i]).error());
                return {self().mAwaiter->callerCoroutine()};
            }
        }
        std::vector<T2> ret;
        ret.reserve(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) {
            ret.emplace_back(*std::move(e[i]));
        }
        return {std::in_place, std::move(ret)};
    }

    template <class T2>
    ValueOrReturnAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &&e) noexcept {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>> {
            if (!([&] () -> ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>> {
                    if (std::get<Is>(e).has_error()) [[unlikely]] {
                        self().mAwaiter->returnValue(std::move(std::get<Is>(e)).error());
                        return false;
                    }
                    return true;
                }() &&
                  ...)) {
                return {self().mAwaiter->callerCoroutine()};
            }
            return {
                std::in_place, [&]() -> decltype(auto) {
                    return *std::move(std::get<Is>(e)), Void();
                }()...};
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &e) noexcept {
        return await_transform(std::move(e));
    }
};

template <class TaskPromise>
struct TaskPromiseTransforms {
    TaskPromise &self() noexcept { return static_cast<TaskPromise &>(*this); }
    TaskPromise const &self() const noexcept { return static_cast<TaskPromise const &>(*this); }

    template <class U>
    Task<U> &&await_transform(Task<U> &&u) noexcept {
        u.promise().mCancelToken = self().mCancelToken;
        return std::move(u);
    }

    template <class U>
    Task<U> const &await_transform(Task<U> const &u) noexcept {
        u.promise().mCancelToken = self().mCancelToken;
        return u;
    }

    template <std::invocable<TaskPromise &> U>
    auto await_transform(U &&u) noexcept {
        return await_transform(u(self()));
    }

    template <class U> requires (!std::invocable<U, TaskPromise &>)
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }
};

template <class TaskPromise, class T>
struct TaskPromiseImpl : TaskPromiseCommon<TaskPromise>, TaskPromiseTransforms<TaskPromise> {
};

template <class TaskPromise, class T>
struct TaskPromiseImpl<TaskPromise, Expected<T>> : TaskPromiseCommon<TaskPromise>, TaskPromiseTransforms<TaskPromise>, TaskPromiseExpectedTransforms<TaskPromise> {
    using TaskPromiseTransforms<TaskPromise>::await_transform;
    using TaskPromiseExpectedTransforms<TaskPromise>::await_transform;
    static_assert(std::is_void_v<std::void_t<T>>);
};

template <class T>
struct TaskPromise : TaskPromiseImpl<TaskPromise<T>, T> {
    void return_value(T &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<T> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    TaskAwaiter<T> *mAwaiter{};
    CancelSourceImpl *mCancelToken{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <>
struct TaskPromise<void> : TaskPromiseImpl<TaskPromise<void>, void> {
    void return_void() {
        mAwaiter->returnVoid();
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

template <class T, class E>
struct TaskPromise<GeneratorResult<T, E>> : TaskPromiseImpl<TaskPromise<GeneratorResult<T, E>>, E> {
    auto yield_value(T &&ret) {
        return mAwaiter->yieldValue(std::move(ret));
    }

    auto yield_value(T const &ret) {
        return mAwaiter->yieldValue(ret);
    }

    auto yield_value(std::convertible_to<T> auto &&ret) {
        return mAwaiter->yieldValue(std::forward<decltype(ret)>(ret));
    }

    void return_value(E &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(E const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<E> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    TaskAwaiter<GeneratorResult<T, E>> *mAwaiter{};
    CancelSourceImpl *mCancelToken{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T>
struct TaskPromise<GeneratorResult<T, void>> : TaskPromiseImpl<TaskPromise<GeneratorResult<T, void>>, void> {
    auto yield_value(T &&ret) {
        return mAwaiter->yieldValue(std::move(ret));
    }

    auto yield_value(T const &ret) {
        return mAwaiter->yieldValue(ret);
    }

    auto yield_value(std::convertible_to<T> auto &&ret) {
        return mAwaiter->yieldValue(std::forward<decltype(ret)>(ret));
    }

    void return_void() {
        mAwaiter->returnVoid();
    }

    TaskAwaiter<GeneratorResult<T, void>> *mAwaiter{};
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
    co_return co_await std::move(f)(std::move(args)...);
    /* std::optional o(std::move(f)); */
    /* decltype(auto) r = co_await std::move(*o)(); */
    /* o.reset(); */
    /* co_return */
    /*     typename AwaitableTraits<std::invoke_result_t<F,
     * Args...>>::RetType( */
    /*         std::forward<decltype(r)>(r)); */
}

} // namespace co_async

#define co_awaits co_await co_await
#define co_returns co_return {}

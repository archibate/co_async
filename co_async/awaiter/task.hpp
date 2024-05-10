#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/utils/expected.hpp>
#if CO_ASYNC_PERF
#include <co_async/utils/perf.hpp>
#endif
#include <co_async/awaiter/details/previous_awaiter.hpp>
#include <co_async/awaiter/details/value_awaiter.hpp>

namespace co_async {

struct PromiseBase {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    struct FinalAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        template <class P>
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<P> coroutine) const noexcept {
            return static_cast<PromiseBase &>(coroutine.promise()).mPrevious;
        }

        void await_resume() const noexcept {}
    };

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void unhandled_exception() noexcept {
#if CO_ASYNC_EXCEPT
        mException = std::current_exception();
#else
        std::terminate();
#endif
    }

    void setPrevious(std::coroutine_handle<> previous) noexcept {
#if CO_ASYNC_DEBUG
        if (mPrevious) [[unlikely]] {
            std::cerr << "WARNING: co_wait'ing twice on a single task\n";
        }
#endif
        mPrevious = previous;
    }

    PromiseBase &operator=(PromiseBase &&) = delete;

protected:
    std::coroutine_handle<> mPrevious;

#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif
};

template <class T>
struct Promise : PromiseBase {
    void return_value(T &&ret) {
        mResult.putValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mResult.putValue(ret);
    }

    T result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

private:
    Uninitialized<T> mResult;

#if CO_ASYNC_PERF
public:
    Perf mPerf;

    Promise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <>
struct Promise<void> : PromiseBase {
    void return_void() noexcept {}

    void result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

#if CO_ASYNC_PERF
    Perf mPerf;

    Promise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class E>
struct Promise<Expected<T, E>> : PromiseBase {
    void return_value(Expected<T, E> &&ret) {
        mResult.putValue(std::move(ret));
    }

    void return_value(Expected<T, E> const &ret) {
        mResult.putValue(ret);
    }

    /* void return_value(T &&ret) requires (!std::is_void_v<T>) { */
    /*     mResult.putValue(std::move(ret)); */
    /* } */
    /*  */
    /* void return_value(T const &ret) requires (!std::is_void_v<T>) { */
    /*     mResult.putValue(ret); */
    /* } */
    /*  */
    /* void return_void() requires (std::is_void_v<T>) { */
    /*     mResult.putValue(); */
    /* } */

    Expected<T, E> result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            if constexpr (std::is_void_v<E>) {
                mResult.putValue(Unexpected<E>());
            } else {
                static_assert(std::same_as<E2, E>,
                              "co_await'ing Expected's error type mismatch");
                mResult.putValue(Unexpected<E>(std::move(e.error())));
            }
            return ValueAwaiter<T2>(mPrevious);
        }
        if constexpr (std::is_void_v<T2>) {
            return ValueAwaiter<void>(std::in_place);
        } else {
            return ValueAwaiter<T2>(std::in_place, std::move(*e));
        }
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class U>
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }

private:
    Uninitialized<Expected<T, E>> mResult;

#if CO_ASYNC_PERF
public:
    Perf mPerf;

    Promise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class P>
struct CustomPromise : Promise<T> {
    auto get_return_object() {
        static_assert(std::is_base_of_v<CustomPromise, P>);
        return std::coroutine_handle<P>::from_promise(static_cast<P &>(*this));
    }
};

template <class T, class P>
struct TaskAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<P>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        P &promise = mCoroutine.promise();
        promise.setPrevious(coroutine);
        return mCoroutine;
    }

#if CO_ASYNC_DEBUG
    T await_resume() const {
        auto coroutine = mCoroutine;
        mCoroutine = nullptr;
        return coroutine.promise().result();
    }

    explicit TaskAwaiter(std::coroutine_handle<P> coroutine)
        : mCoroutine(coroutine) {}

    TaskAwaiter(TaskAwaiter &&) = delete;

    ~TaskAwaiter() noexcept {
        if (mCoroutine && mCoroutine.done()) [[unlikely]] {
            std::cerr << "WARNING: done coroutine return value ignored\n";
            (void)mCoroutine.promise().result();
        }
    }

private:
    mutable std::coroutine_handle<P> mCoroutine;

#else
    T await_resume() const {
        return mCoroutine.promise().result();
    }

    std::coroutine_handle<P> mCoroutine;
#endif
};

template <class T = void, class P = Promise<T>>
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
        if (!mCoroutine)
            return;
        //#if CO_ASYNC_DEBUG
        //if (!mCoroutine.done()) [[unlikely]] {
        //#if CO_ASYNC_PERF
        //auto &perf = mCoroutine.promise().mPerf;
        //std::cerr << "warning: task (" << perf.file << ":" << perf.line
        //<< ") destroyed undone\n";
        //#else
        //std::cerr << "warning: task destroyed undone\n";
        //#endif
        //}
        //#endif
        mCoroutine.destroy();
    }

    auto operator co_await() const noexcept {
        return TaskAwaiter<T, P>(mCoroutine);
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

} // namespace co_async

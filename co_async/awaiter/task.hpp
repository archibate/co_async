#pragma once/*{export module co_async:awaiter.task;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/utils/uninitialized.hpp>/*{import :utils.uninitialized;}*/
#if CO_ASYNC_PERF
#include <co_async/utils/perf.hpp>/*{import :utils.perf;}*/
#endif
#include <co_async/awaiter/details/previous_awaiter.hpp>/*{import :awaiter.details.previous_awaiter;}*/

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
        mPrevious = previous;
    }

    PromiseBase &operator=(PromiseBase &&) = delete;

protected:
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif

private:
    std::coroutine_handle<> mPrevious;
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

    T await_resume() const {
        return mCoroutine.promise().result();
    }

    std::coroutine_handle<P> mCoroutine;
};

/*[export]*/ template <class T = void, class P = Promise<T>>
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
#if CO_ASYNC_DEBUG
        if (!mCoroutine.done()) [[unlikely]] {
#if CO_ASYNC_PERF
            auto &perf = mCoroutine.promise().mPerf;
            std::cerr << "WARNING: task (" << perf.file << ":" << perf.line
                      << ") destroyed undone\n";
#else
            std::cerr << "WARNING: task destroyed undone\n";
#endif
        }
#endif
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

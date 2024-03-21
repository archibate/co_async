#pragma once

#include <exception>
#include <coroutine>
#include <optional>
#include <co_async/uninitialized.hpp>
#include <co_async/previous_awaiter.hpp>

namespace co_async {

template <class T>
struct GeneratorPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
    }

    void yield_value(T &&ret) {
        mResult.putValue(std::move(ret));
    }

    void yield_value(T const &ret) {
        mResult.putValue(ret);
    }

    void return_void() {
        mFinal = true;
    }

    bool final() {
        if (mFinal) {
            if (mException) [[unlikely]] {
                std::rethrow_exception(mException);
            }
        }
        return mFinal;
    }

    T result() {
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<GeneratorPromise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious;
    bool mFinal = false;
    Uninitialized<T> mResult;
    std::exception_ptr mException{};

    GeneratorPromise &operator=(GeneratorPromise &&) = delete;
};

template <class T>
struct GeneratorPromise<T &> {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
        mResult = nullptr;
    }

    auto yield_value(T &ret) {
        mResult = std::addressof(ret);
        return PreviousAwaiter(mPrevious);
    }

    void return_void() {
        mResult = nullptr;
    }

    bool final() {
        if (!mResult) {
            if (mException) [[unlikely]] {
                std::rethrow_exception(mException);
            }
            return true;
        }
        return false;
    }

    T &result() {
        return *mResult;
    }

    auto get_return_object() {
        return std::coroutine_handle<GeneratorPromise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious{};
    T *mResult;
    std::exception_ptr mException{};

    GeneratorPromise &operator=(GeneratorPromise &&) = delete;
};

template <class T, class P = GeneratorPromise<T>>
struct Generator {
    using promise_type = P;

    Generator(std::coroutine_handle<promise_type> coroutine) noexcept
        : mCoroutine(coroutine) {}

    Generator(Generator &&) = delete;

    ~Generator() {
        mCoroutine.destroy();
    }

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        std::coroutine_handle<promise_type>
        await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            mCoroutine.promise().mPrevious = coroutine;
            return mCoroutine;
        }

        std::optional<T> await_resume() const {
            if (mCoroutine.promise().final())
                return std::nullopt;
            return mCoroutine.promise().result();
        }

        std::coroutine_handle<promise_type> mCoroutine;
    };

    auto operator co_await() const noexcept {
        return Awaiter(mCoroutine);
    }

    operator std::coroutine_handle<promise_type>() const noexcept {
        return mCoroutine;
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

} // namespace co_async

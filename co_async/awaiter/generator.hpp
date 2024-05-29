#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/details/previous_awaiter.hpp>
#include <co_async/utils/uninitialized.hpp>

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
        mFinal = true;
    }

    auto yield_value(T &&ret) {
        mResult.emplace(std::move(ret));
        return PreviousAwaiter(mPrevious);
    }

    auto yield_value(T const &ret) {
        mResult.emplace(ret);
        return PreviousAwaiter(mPrevious);
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
        return mResult.move();
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
struct [[nodiscard]] Generator {
    using promise_type = P;

    Generator(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    Generator(Generator &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    Generator &operator=(Generator &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~Generator() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
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
            if (mCoroutine.promise().final()) {
                return std::nullopt;
            }
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
#if 0
template <class T, class A, class LoopRef>
struct GeneratorIterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    explicit GeneratorIterator(A awaiter, LoopRef loop) noexcept
        : mAwaiter(awaiter),
          mLoop(loop) {
        ++*this;
    }

    bool operator!=(std::default_sentinel_t) const noexcept {
        return mResult.has_value();
    }

    bool operator==(std::default_sentinel_t) const noexcept {
        return !(*this != std::default_sentinel);
    }

    friend bool operator==(std::default_sentinel_t,
                           GeneratorIterator const &it) noexcept {
        return it == std::default_sentinel;
    }

    friend bool operator!=(std::default_sentinel_t,
                           GeneratorIterator const &it) noexcept {
        return it == std::default_sentinel;
    }

    GeneratorIterator &operator++() {
        mAwaiter.mCoroutine.resume();
        mLoop.run();
        mResult = mAwaiter.await_resume();
        return *this;
    }

    GeneratorIterator operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    T &operator*() noexcept {
        return *mResult;
    }

    T *operator->() noexcept {
        return mResult.operator->();
    }

private:
    A mAwaiter;
    LoopRef mLoop;
    std::optional<T> mResult;
};

template <class Loop, class T, class P>
auto run_generator_on(Loop &loop, Generator<T, P> const &g) {
    using Awaiter = typename Generator<T, P>::Awaiter;

    struct GeneratorRange {
        explicit GeneratorRange(Awaiter awaiter, Loop &loop)
            : mAwaiter(awaiter),
              mLoop(loop) {
            mAwaiter.await_suspend(std::noop_coroutine());
        }

        auto begin() const noexcept {
            return GeneratorIterator<T, Awaiter, Loop &>(mAwaiter, mLoop);
        }

        std::default_sentinel_t end() const noexcept {
            return {};
        }

    private:
        Awaiter mAwaiter;
        Loop &mLoop;
    };

    return GeneratorRange(g.operator co_await(), loop);
};
#endif
} // namespace co_async

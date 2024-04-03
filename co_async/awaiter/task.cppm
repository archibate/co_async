export module co_async:awaiter.task;

import std;
import :utils.uninitialized;
import :awaiter.details.previous_awaiter;

namespace co_async {

template <class T>
struct Promise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
    }

    void return_value(T &&ret) {
        mResult.putValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mResult.putValue(ret);
    }

    T result() {
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    void setPrevious(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    std::coroutine_handle<> mPrevious;
    std::exception_ptr mException{};
    Uninitialized<T> mResult; // destructed??

    Promise &operator=(Promise &&) = delete;
};

template <>
struct Promise<void> {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
    }

    void return_void() noexcept {}

    void result() {
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
    }

    auto get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    void setPrevious(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    std::coroutine_handle<> mPrevious;
    std::exception_ptr mException{};

    Promise &operator=(Promise &&) = delete;
};

export template <class T = void, class P = Promise<T>>
struct [[nodiscard]] Task {
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
        if (mCoroutine)
            mCoroutine.destroy();
    }

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        std::coroutine_handle<promise_type>
        await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            promise_type &promise = mCoroutine.promise();
            promise.setPrevious(coroutine);
            return mCoroutine;
        }

        T await_resume() const {
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

    std::coroutine_handle<promise_type> release() noexcept {
        auto coroutine = mCoroutine;
        mCoroutine = nullptr;
        return coroutine;
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

export template <class Loop, class T, class P>
T loop_run(Loop &loop, Task<T, P> const &t) {
    auto a = t.operator co_await();
    auto c = a.await_suspend(std::noop_coroutine());
    c.resume();
    while (!c.done()) {
        loop.run();
    }
    return a.await_resume();
};

} // namespace co_async

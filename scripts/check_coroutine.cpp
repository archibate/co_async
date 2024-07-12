#include <coroutine>
#include <cstdio>

struct RepeatAwaiter
{
    bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        if (coroutine.done())
            return std::noop_coroutine();
        else
            return coroutine;
    }

    void await_resume() const noexcept {}
};

struct RepeatAwaitable
{
    RepeatAwaiter operator co_await() {
        return RepeatAwaiter();
    }
};

struct Promise {
    auto initial_suspend() {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return std::suspend_always();
    }

    void unhandled_exception() {
        throw;
    }

    auto yield_value(int ret) {
        mRetValue = ret;
        return RepeatAwaiter();
    }

    void return_void() {
        mRetValue = 0;
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    int mRetValue;
};

struct Task {
    using promise_type = Promise;

    Task(std::coroutine_handle<promise_type> coroutine)
        : mCoroutine(coroutine) {}

    std::coroutine_handle<promise_type> mCoroutine;
};

Task hello() {
    co_yield 42;
    co_yield 12;
    co_yield 6;
    co_return;
}

int main() {
    Task t = hello();
    while (!t.mCoroutine.done()) {
        t.mCoroutine.resume();
        printf("%d\n", t.mCoroutine.promise().mRetValue);
    }
    return 0;
}

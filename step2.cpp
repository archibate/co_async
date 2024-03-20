#include <chrono>
#include <coroutine>
#include "debug.hpp"

struct RepeatAwaiter // awaiter(原始指针) / awaitable(operator->)
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

struct RepeatAwaitable // awaitable(operator->)
{
    RepeatAwaiter operator co_await() {
        return RepeatAwaiter();
    }
};

struct PreviousAwaiter {
    std::coroutine_handle<> mPrevious;

    bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        if (mPrevious)
            return mPrevious;
        else
            return std::noop_coroutine();
    }

    void await_resume() const noexcept {}
};

struct Promise {
    auto initial_suspend() {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() {
        throw;
    }

    auto yield_value(int ret) {
        mRetValue = ret;
        return std::suspend_always();
    }

    void return_void() {
        mRetValue = 0;
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    int mRetValue;
    std::coroutine_handle<> mPrevious = nullptr;
};

struct Task {
    using promise_type = Promise;

    Task(std::coroutine_handle<promise_type> coroutine)
        : mCoroutine(coroutine) {}

    Task(Task &&) = delete;

    ~Task() {
        mCoroutine.destroy();
    }

    std::coroutine_handle<promise_type> mCoroutine;
};

struct WorldTask {
    using promise_type = Promise;

    WorldTask(std::coroutine_handle<promise_type> coroutine)
        : mCoroutine(coroutine) {}

    WorldTask(WorldTask &&) = delete;

    ~WorldTask() {
        mCoroutine.destroy();
    }

    struct WorldAwaiter {
        bool await_ready() const noexcept { return false; }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            mCoroutine.promise().mPrevious = coroutine;
            return mCoroutine;
        }

        void await_resume() const noexcept {}

        std::coroutine_handle<promise_type> mCoroutine;
    };

    auto operator co_await() {
        return WorldAwaiter(mCoroutine);
    }

    std::coroutine_handle<promise_type> mCoroutine;
};

WorldTask world() {
    debug(), "world";
    co_yield 422;
    co_yield 444;
    co_return;
}

Task hello() {
    debug(), "hello 正在构建worldTask";
    WorldTask worldTask = world();
    debug(), "hello 构建完了worldTask，开始等待world";
    co_await worldTask;
    debug(), "hello得到world返回", worldTask.mCoroutine.promise().mRetValue;
    co_await worldTask;
    debug(), "hello得到world返回", worldTask.mCoroutine.promise().mRetValue;
    debug(), "hello 42";
    co_yield 42;
    debug(), "hello 12";
    co_yield 12;
    debug(), "hello 6";
    co_yield 6;
    debug(), "hello 结束";
    co_return;
}

int main() {
    debug(), "main即将调用hello";
    Task t = hello();
    debug(), "main调用完了hello"; // 其实只创建了task对象，并没有真正开始执行
    while (!t.mCoroutine.done()) {
        t.mCoroutine.resume();
        debug(), "main得到hello结果为",
            t.mCoroutine.promise().mRetValue;
    }
    return 0;
}

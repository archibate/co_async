#include <chrono>
#include <coroutine>
#include <deque>
#include <queue>
#include <thread>
#include "debug.hpp"

using namespace std::chrono_literals;

struct RepeatAwaiter {
    bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        if (coroutine.done())
            return std::noop_coroutine();
        else
            return coroutine;
    }

    void await_resume() const noexcept {}
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

    auto yield_value(T ret) noexcept {
        new (&mResult) T(std::move(ret));
        return std::suspend_always();
    }

    void return_value(T ret) noexcept {
        new (&mResult) T(std::move(ret));
    }

    T result() {
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
        T ret = std::move(mResult);
        mResult.~T();
        return ret;
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious{};
    std::exception_ptr mException{};
    union {
        T mResult;
    };

    Promise() noexcept {}
    Promise(Promise &&) = delete;
    ~Promise() {}
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

    void return_void() noexcept {
    }

    void result() {
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
    }

    std::coroutine_handle<Promise> get_return_object() {
        return std::coroutine_handle<Promise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious{};
    std::exception_ptr mException{};

    Promise() = default;
    Promise(Promise &&) = delete;
    ~Promise() = default;
};

template <class T = void>
struct Task {
    using promise_type = Promise<T>;

    Task(std::coroutine_handle<promise_type> coroutine) noexcept
        : mCoroutine(coroutine) {}

    Task(Task &&) = delete;

    ~Task() {
        mCoroutine.destroy();
    }

    struct Awaiter {
        bool await_ready() const noexcept { return false; }

        std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            mCoroutine.promise().mPrevious = coroutine;
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

    operator std::coroutine_handle<>() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> mCoroutine;
};

struct Loop {
    std::deque<std::coroutine_handle<>> mReadyQueue;

    struct TimerEntry {
        std::chrono::system_clock::time_point expireTime;
        std::coroutine_handle<> coroutine;

        bool operator<(TimerEntry const &that) const noexcept {
            return expireTime > that.expireTime;
        }
    };

    std::priority_queue<TimerEntry> mTimerHeap;

    void addTask(std::coroutine_handle<> coroutine) {
        mReadyQueue.push_front(coroutine);
    }

    void addTimer(std::chrono::system_clock::time_point expireTime, std::coroutine_handle<> coroutine) {
        mTimerHeap.push({expireTime, coroutine});
    }

    void runAll() {
        while (!mTimerHeap.empty() || !mReadyQueue.empty()) {
            while (!mReadyQueue.empty()) {
                auto coroutine = mReadyQueue.front();
                mReadyQueue.pop_front();
                coroutine.resume();
            }
            if (!mTimerHeap.empty()) {
                auto nowTime = std::chrono::system_clock::now();
                auto timer = std::move(mTimerHeap.top());
                if (timer.expireTime < nowTime) {
                    mTimerHeap.pop();
                    timer.coroutine.resume();
                } else {
                    std::this_thread::sleep_until(timer.expireTime);
                }
            }
        }
    }

    Loop &operator=(Loop &&) = delete;
};

Loop &getLoop() {
    static Loop loop;
    return loop;
}

struct SleepAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine) const {
        getLoop().addTimer(mExpireTime, coroutine);
    }

    void await_resume() const noexcept {
    }

    std::chrono::system_clock::time_point mExpireTime;
};

Task<void> sleep_until(std::chrono::system_clock::time_point expireTime) {
    co_await SleepAwaiter(expireTime);
    co_return;
}

Task<void> sleep_for(std::chrono::system_clock::duration duration) {
    co_await SleepAwaiter(std::chrono::system_clock::now() + duration);
    co_return;
}

Task<int> hello1() {
    debug(), "hello1开始睡1秒";
    co_await sleep_for(1s); // 1s 等价于 std::chrono::seconds(1)
    debug(), "hello1睡醒了";
    co_return 1;
}

Task<int> hello2() {
    debug(), "hello2开始睡2秒";
    co_await sleep_for(2s); // 2s 等价于 std::chrono::seconds(2)
    debug(), "hello2睡醒了";
    co_return 2;
}

int main() {
    auto t1 = hello1();
    auto t2 = hello2();
    getLoop().addTask(t1);
    getLoop().addTask(t2);
    getLoop().runAll();
    debug(), "主函数中得到hello1结果:", t1.mCoroutine.promise().result();
    debug(), "主函数中得到hello2结果:", t2.mCoroutine.promise().result();
    return 0;
}

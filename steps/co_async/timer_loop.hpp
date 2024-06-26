#pragma once

#include <coroutine>
#include <chrono>
#include <optional>
#include "task.hpp"
#include "rbtree.hpp"

namespace co_async {

struct SleepUntilPromise : RbTree<SleepUntilPromise>::RbNode, Promise<void> {
    std::chrono::system_clock::time_point mExpireTime;

    auto get_return_object() {
        return std::coroutine_handle<SleepUntilPromise>::from_promise(*this);
    }

    SleepUntilPromise &operator=(SleepUntilPromise &&) = delete;

    friend bool operator<(SleepUntilPromise const &lhs,
                          SleepUntilPromise const &rhs) noexcept {
        return lhs.mExpireTime < rhs.mExpireTime;
    }
};

struct TimerLoop {
    // 弱红黑树，只保留一个引用指向真正的Promise
    RbTree<SleepUntilPromise> mRbTimer;

    bool hasEvent() const noexcept {
        return !mRbTimer.empty();
    }

    void addTimer(SleepUntilPromise &promise) {
        mRbTimer.insert(promise);
    }

    std::optional<std::chrono::system_clock::duration> run() {
        while (!mRbTimer.empty()) {
            auto nowTime = std::chrono::system_clock::now();
            auto &promise = mRbTimer.front();
            if (promise.mExpireTime < nowTime) {
                mRbTimer.erase(promise);
                std::coroutine_handle<SleepUntilPromise>::from_promise(promise)
                    .resume();
            } else {
                return promise.mExpireTime - nowTime;
            }
        }
        return std::nullopt;
    }

    TimerLoop &operator=(TimerLoop &&) = delete;
};

struct SleepAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void
    await_suspend(std::coroutine_handle<SleepUntilPromise> coroutine) const {
        auto &promise = coroutine.promise();
        promise.mExpireTime = mExpireTime;
        mLoop.addTimer(promise);
    }

    void await_resume() const noexcept {}

    using ClockType = std::chrono::system_clock;

    TimerLoop &mLoop;
    ClockType::time_point mExpireTime;
};

template <class Clock, class Dur>
inline Task<void, SleepUntilPromise>
sleep_until(TimerLoop &loop, std::chrono::time_point<Clock, Dur> expireTime) {
    co_await SleepAwaiter(
        loop, std::chrono::time_point_cast<SleepAwaiter::ClockType::duration>(
                  expireTime));
}

template <class Rep, class Period>
inline Task<void, SleepUntilPromise>
sleep_for(TimerLoop &loop, std::chrono::duration<Rep, Period> duration) {
    auto d =
        std::chrono::duration_cast<SleepAwaiter::ClockType::duration>(duration);
    if (d.count() > 0) {
        co_await SleepAwaiter(loop, SleepAwaiter::ClockType::now() + d);
    }
}

} // namespace co_async

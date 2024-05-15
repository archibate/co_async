#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {
struct Semaphore {
private:
    std::size_t mCounter;
    std::size_t const mMaxCount;
    ConditionVariable mChanged;

public:
    explicit Semaphore(std::size_t maxCount = 1, std::size_t initialCount = 0)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::size_t count() const noexcept {
        return mCounter;
    }

    std::size_t max_count() const noexcept {
        return mMaxCount;
    }

    bool try_acquire() {
        auto count = mCounter;
        if (count > 0) {
            mCounter = count - 1;
            if (count == mMaxCount) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    bool try_release() {
        auto count = mCounter;
        if (count < mMaxCount) {
            mCounter = count + 1;
            if (count == 0) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    Task<> acquire() {
        while (!try_acquire()) {
            co_await mChanged.wait();
        }
    }

    Task<> release() {
        while (!try_release()) {
            co_await mChanged.wait();
        }
    }
};

struct TimedSemaphore {
private:
    std::size_t mCounter;
    std::size_t const mMaxCount;
    TimedConditionVariable mChanged;

public:
    explicit TimedSemaphore(std::size_t maxCount = 1,
                            std::size_t initialCount = 0)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::size_t count() const noexcept {
        return mCounter;
    }

    std::size_t max_count() const noexcept {
        return mMaxCount;
    }

    bool try_acquire() {
        auto count = mCounter;
        if (count > 0) {
            mCounter = count - 1;
            if (count == mMaxCount) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    bool try_release() {
        auto count = mCounter;
        if (count < mMaxCount) {
            mCounter = count + 1;
            if (count == 0) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    Task<> acquire() {
        while (!try_acquire()) {
            co_await mChanged.wait();
        }
    }

    Task<> release() {
        while (!try_release()) {
            co_await mChanged.wait();
        }
    }

    Task<bool> try_acquire(std::chrono::steady_clock::duration timeout) {
        return try_acquire(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_acquire(std::chrono::steady_clock::time_point expires) {
        while (!try_acquire()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mChanged.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }

    Task<bool> try_release(std::chrono::steady_clock::duration timeout) {
        return try_release(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_release(std::chrono::steady_clock::time_point expires) {
        while (!try_release()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mChanged.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }
};
} // namespace co_async

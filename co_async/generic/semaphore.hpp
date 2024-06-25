#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/platform/futex.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {
// struct Semaphore {
// private:
//     std::size_t mCounter;
//     std::size_t const mMaxCount;
//     ConditionVariable mChanged;
//
// public:
//     explicit Semaphore(std::size_t maxCount = 1, std::size_t initialCount = 0)
//         : mCounter(initialCount),
//           mMaxCount(maxCount) {}
//
//     std::size_t count() const noexcept {
//         return mCounter;
//     }
//
//     std::size_t max_count() const noexcept {
//         return mMaxCount;
//     }
//
//     bool try_acquire() {
//         auto count = mCounter;
//         if (count > 0) {
//             mCounter = count - 1;
//             if (count == mMaxCount) {
//                 mChanged.notify_one();
//             }
//             return true;
//         }
//         return false;
//     }
//
//     bool try_release() {
//         auto count = mCounter;
//         if (count < mMaxCount) {
//             mCounter = count + 1;
//             if (count == 0) {
//                 mChanged.notify_one();
//             }
//             return true;
//         }
//         return false;
//     }
//
//     Task<> acquire() {
//         while (!try_acquire()) {
//             co_await mChanged.wait();
//         }
//     }
//
//     Task<> release() {
//         while (!try_release()) {
//             co_await mChanged.wait();
//         }
//     }
// };
//
// struct TimedSemaphore {
// private:
//     std::size_t mCounter;
//     std::size_t const mMaxCount;
//     TimedConditionVariable mChanged;
//
// public:
//     explicit TimedSemaphore(std::size_t maxCount = 1,
//                             std::size_t initialCount = 0)
//         : mCounter(initialCount),
//           mMaxCount(maxCount) {}
//
//     std::size_t count() const noexcept {
//         return mCounter;
//     }
//
//     std::size_t max_count() const noexcept {
//         return mMaxCount;
//     }
//
//     bool try_acquire() {
//         auto count = mCounter;
//         if (count > 0) {
//             mCounter = count - 1;
//             if (count == mMaxCount) {
//                 mChanged.notify_one();
//             }
//             return true;
//         }
//         return false;
//     }
//
//     bool try_release() {
//         auto count = mCounter;
//         if (count < mMaxCount) {
//             mCounter = count + 1;
//             if (count == 0) {
//                 mChanged.notify_one();
//             }
//             return true;
//         }
//         return false;
//     }
//
//     Task<> acquire() {
//         while (!try_acquire()) {
//             co_await mChanged.wait();
//         }
//     }
//
//     Task<> release() {
//         while (!try_release()) {
//             co_await mChanged.wait();
//         }
//     }
//
//     Task<bool> try_acquire(std::chrono::steady_clock::duration timeout) {
//         return try_acquire(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<bool> try_acquire(std::chrono::steady_clock::time_point expires) {
//         while (!try_acquire()) {
//             if (std::chrono::steady_clock::now() > expires ||
//                 !co_await mChanged.wait(expires)) {
//                 co_return false;
//             }
//         }
//         co_return true;
//     }
//
//     Task<bool> try_release(std::chrono::steady_clock::duration timeout) {
//         return try_release(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<bool> try_release(std::chrono::steady_clock::time_point expires) {
//         while (!try_release()) {
//             if (std::chrono::steady_clock::now() > expires ||
//                 !co_await mChanged.wait(expires)) {
//                 co_return false;
//             }
//         }
//         co_return true;
//     }
// };

template <std::uint32_t MaxCount = 0>
struct Semaphore {
private:
    std::atomic<std::uint32_t> mCounter;

    enum WakeBits : uint32_t {
        WakeRelease = 1,
        WakeAcquire = 2,
    };

public:
    explicit Semaphore(std::uint32_t initialCount)
        : mCounter(initialCount) {}

    std::uint32_t count() const noexcept {
        return mCounter.load(std::memory_order_acquire);
    }

    constexpr std::uint32_t max_count() const noexcept {
        return MaxCount;
    }

    Task<Expected<>> acquire() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == 0) {
                co_await co_await futex_wait(&mCounter, count, (uint32_t)WakeAcquire);
                count = mCounter.load(std::memory_order_relaxed);
            }
            count = count - 1;
        } while (mCounter.compare_exchange_weak(count, count - 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        if (count == MaxCount) {
            (void)co_await futex_wake(&mCounter, 1, (uint32_t)WakeRelease);
        }
        co_return {};
    }

    Task<Expected<>> release() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == MaxCount) {
                co_await co_await futex_wait(&mCounter, count, (uint32_t)WakeRelease);
                count = mCounter.load(std::memory_order_relaxed);
            }
            count = count + 1;
        } while (mCounter.compare_exchange_weak(count, count - 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        if (count == 0) {
            co_await co_await futex_wake(&mCounter, 1, (uint32_t)WakeRelease);
        }
        co_return {};
    }
};

using BinarySemaphore = Semaphore<1>;

template <>
struct Semaphore<0> {
private:
    std::atomic<std::uint32_t> mCounter;
    std::uint32_t const mMaxCount;

    enum WakeBits : uint32_t {
        WakeRelease = 1,
        WakeAcquire = 2,
    };

public:
    explicit Semaphore(std::uint32_t maxCount, std::uint32_t initialCount)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::uint32_t count() const noexcept {
        return mCounter.load(std::memory_order_acquire);
    }

    std::uint32_t max_count() const noexcept {
        return mMaxCount;
    }

    Task<Expected<>> acquire() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == 0) {
                co_await co_await futex_wait(&mCounter, count, (uint32_t)WakeAcquire);
                count = mCounter.load(std::memory_order_relaxed);
            }
            count = count - 1;
        } while (mCounter.compare_exchange_weak(count, count - 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        if (count == mMaxCount) {
            (void)co_await futex_wake(&mCounter, 1, (uint32_t)WakeRelease);
        }
        co_return {};
    }

    Task<Expected<>> release() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == mMaxCount) {
                co_await co_await futex_wait(&mCounter, count, (uint32_t)WakeRelease);
                count = mCounter.load(std::memory_order_relaxed);
            }
            count = count + 1;
        } while (mCounter.compare_exchange_weak(count, count - 1, std::memory_order_acq_rel, std::memory_order_relaxed));
        if (count == 0) {
            co_await co_await futex_wake(&mCounter, 1, (uint32_t)WakeRelease);
        }
        co_return {};
    }
};
} // namespace co_async

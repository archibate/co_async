#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/platform/futex.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {

struct Semaphore {
private:
    FutexAtomic<std::uint32_t> mCounter;
    std::uint32_t const mMaxCount;

    static constexpr std::uint32_t kAcquireMask = 1;
    static constexpr std::uint32_t kReleaseMask = 2;

public:
    explicit Semaphore(std::uint32_t maxCount, std::uint32_t initialCount)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::uint32_t count() const noexcept {
        return mCounter.load(std::memory_order_relaxed);
    }

    std::uint32_t max_count() const noexcept {
        return mMaxCount;
    }

    Task<Expected<>> acquire() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == 0) {
                co_await co_await futex_wait(&mCounter, count, kAcquireMask);
                count = mCounter.load(std::memory_order_relaxed);
            }
        } while (mCounter.compare_exchange_weak(count, count - 1,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed));
        futex_notify(&mCounter, 1, kReleaseMask);
        co_return {};
    }

    Task<Expected<>> release() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == mMaxCount) {
                co_await co_await futex_wait(&mCounter, count, kReleaseMask);
                count = mCounter.load(std::memory_order_relaxed);
            }
        } while (mCounter.compare_exchange_weak(count, count + 1,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed));
        futex_notify(&mCounter, 1, kAcquireMask);
        co_return {};
    }
};
} // namespace co_async

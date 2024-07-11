#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/ring_queue.hpp>
#include <co_async/utils/spin_mutex.hpp>

namespace co_async {
template <class T>
struct Queue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mReady;

    static constexpr ConditionVariable::Mask kNonEmptyMask = 1;
    static constexpr ConditionVariable::Mask kNonFullMask = 2;

public:
    explicit Queue(std::size_t size) : mQueue(size) {}

    std::optional<T> try_pop() {
        bool wasFull = mQueue.full();
        auto value = mQueue.pop();
        if (value && wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        return value;
    }

    bool try_push(T &&value) {
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        if (ok && wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        return ok;
    }

    Task<Expected<>> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await co_await mReady.wait(kNonFullMask);
        }
        mReady.notify_one(kNonEmptyMask);
        co_return {};
    }

    Task<Expected<T>> pop() {
        while (true) {
            if (auto value = mQueue.pop()) {
                mReady.notify_one(kNonFullMask);
                co_return std::move(*value);
            }
            co_await co_await mReady.wait(kNonEmptyMask);
        }
    }
};

template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mReady;
    SpinMutex mMutex;

    static constexpr ConditionVariable::Mask kNonEmptyMask = 1;
    static constexpr ConditionVariable::Mask kNonFullMask = 2;

public:
    explicit ConcurrentQueue(std::size_t maxSize = 0) : mQueue(maxSize) {}

    void set_max_size(std::size_t maxSize) {
        mQueue.set_max_size(maxSize);
    }

    ConcurrentQueue(ConcurrentQueue &&) = delete;

    std::optional<T> try_pop() {
        std::unique_lock lock(mMutex);
        bool wasFull = mQueue.full();
        auto value = mQueue.pop();
        lock.unlock();
        if (value && wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        return value;
    }

    bool try_push(T &&value) {
        std::unique_lock lock(mMutex);
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        lock.unlock();
        if (ok && wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        return ok;
    }

    Task<Expected<T>> pop() {
        std::unique_lock lock(mMutex);
        while (mQueue.empty()) {
            lock.unlock();
            co_await co_await mReady.wait(kNonEmptyMask);
            lock.lock();
        }
        bool wasFull = mQueue.full();
        T value = mQueue.pop_unchecked();
        lock.unlock();
        if (wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        co_return std::move(value);
    }

    Task<Expected<>> push(T value) {
        std::unique_lock lock(mMutex);
        while (mQueue.full()) {
            lock.unlock();
            co_await co_await mReady.wait(kNonFullMask);
            lock.lock();
        }
        bool wasEmpty = mQueue.empty();
        mQueue.push_unchecked(std::move(value));
        lock.unlock();
        if (wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        co_return {};
    }
};
} // namespace co_async

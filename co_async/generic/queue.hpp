#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/ring_queue.hpp>

namespace co_async {
template <class T>
struct Queue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mNonEmpty;
    ConditionVariable mNonFull;

public:
    explicit Queue(std::size_t size) : mQueue(size) {}

    std::optional<T> try_pop() {
        bool wasFull = mQueue.full();
        auto value = mQueue.pop();
        if (value && wasFull) {
            mNonFull.notify_one();
        }
        return value;
    }

    bool try_push(T &&value) {
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        if (ok && wasEmpty) {
            mNonEmpty.notify_one();
        }
        return ok;
    }

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mNonFull;
        }
        mNonEmpty.notify_one();
    }

    Task<T> pop(T value) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mNonFull.notify_one();
                co_return std::move(*value);
            }
            co_await mNonEmpty;
        }
    }
};

template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
private:
    RingQueue<T> mQueue;
    std::mutex mMutex;
    ConditionVariable mReady;

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
            mReady.notify_one();
        }
        return value;
    }

    bool try_push(T &&value) {
        std::unique_lock lock(mMutex);
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        lock.unlock();
        if (ok && wasEmpty) {
            mReady.notify_one();
        }
        return ok;
    }

    Task<T> pop() {
        std::unique_lock lock(mMutex);
        while (mQueue.empty()) {
            lock.unlock();
            co_await mReady;
            lock.lock();
        }
        bool wasFull = mQueue.full();
        T value = mQueue.pop_unchecked();
        lock.unlock();
        if (wasFull) {
            mReady.notify_one();
        }
        co_return std::move(value);
    }

    Task<> push(T value) {
        std::unique_lock lock(mMutex);
        while (mQueue.full()) {
            lock.unlock();
            co_await mReady;
            lock.lock();
        }
        bool wasEmpty = mQueue.empty();
        mQueue.push_unchecked(std::move(value));
        lock.unlock();
        if (wasEmpty) {
            mReady.notify_one();
        }
        co_return;
    }
};
} // namespace co_async

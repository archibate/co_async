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
struct TimedQueue {
private:
    RingQueue<T> mQueue;
    TimedConditionVariable mNonEmpty;
    TimedConditionVariable mNonFull;

public:
    explicit TimedQueue(std::size_t size) : mQueue(size) {}

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mNonFull.wait();
        }
        mNonEmpty.notify_one();
        co_return;
    }

    Task<Expected<>> push(T value,
                          std::chrono::steady_clock::duration timeout) {
        return push(std::move(value),
                    std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<>> push(T value,
                          std::chrono::steady_clock::time_point expires) {
        while (!mQueue.push(std::move(value))) {
            if (auto e = co_await mNonFull.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
        mNonEmpty.notify_one();
        co_return;
    }

    Task<T> pop() {
        while (true) {
            if (auto value = mQueue.pop()) {
                mNonFull.notify_one();
                co_return std::move(*value);
            }
            co_await mNonEmpty.wait();
        }
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::duration timeout) {
        return pop(std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::time_point expires) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mNonFull.notify_one();
                co_return std::move(*value);
            }
            if (auto e = co_await mNonEmpty.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
    }
};

template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
private:
    RingQueue<T> mQueue;
    std::mutex mMutex;
    ConcurrentConditionVariable mReady;

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

    Task<> push(T &&value) {
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

template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentTimedQueue {
private:
    RingQueue<T> mQueue;
    std::mutex mMutex;
    ConcurrentTimedConditionVariable mReady;

public:
    explicit ConcurrentTimedQueue(std::size_t maxSize = 0) : mQueue(maxSize) {}

    void set_max_size(std::size_t maxSize) {
        mQueue.set_max_size(maxSize);
    }

    ConcurrentTimedQueue(ConcurrentTimedQueue &&) = delete;

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
            co_await mReady.wait();
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

    Task<> push(T &&value) {
        std::unique_lock lock(mMutex);
        while (mQueue.full()) {
            lock.unlock();
            co_await mReady.wait();
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

    Task<Expected<T>> pop(auto waitCond) {
        std::unique_lock lock(mMutex);
        while (mQueue.empty()) {
            lock.unlock();
            co_await co_await mReady.wait(waitCond);
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

    Task<Expected<>> push(T &&value, auto waitCond) {
        std::unique_lock lock(mMutex);
        while (mQueue.full()) {
            lock.unlock();
            co_await co_await mReady.wait(waitCond);
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

template <class T>
struct ConcurrentStealingQueue {
private:
    std::unique_ptr<ConcurrentTimedQueue<T>[]> mQueues;
    std::size_t mNumWorkers;
    std::size_t mNextIndex = 0;

public:
    explicit ConcurrentStealingQueue(std::size_t numWorkers,
                                     std::size_t maxSize = 0)
        : mQueues(std::make_unique<ConcurrentTimedQueue<T>[]>(numWorkers)),
          mNumWorkers(numWorkers) {
        if (maxSize) {
            for (std::size_t i = 0; i < mNumWorkers; ++i) {
                mQueues[i].set_max_size(maxSize);
            }
        }
    }

    void set_max_size(std::size_t maxSize) {
        for (std::size_t i = 0; i < mNumWorkers; ++i) {
            mQueues[i].set_max_size(maxSize);
        }
    }

    ConcurrentStealingQueue(ConcurrentStealingQueue &&) = delete;

    struct Consumer {
    private:
        ConcurrentTimedQueue<T> &mQueue;
        std::span<ConcurrentTimedQueue<T>> mOtherQueues;

    public:
        explicit Consumer(ConcurrentTimedQueue<T> &queue,
                          std::span<ConcurrentTimedQueue<T>> otherQueues)
            : mQueue(queue),
              mOtherQueues(otherQueues) {}

        std::optional<T> try_pop() const {
            auto *p = &mQueue;
            while (true) {
                if (auto value = p->try_pop()) {
                    return std::move(*value);
                }
                if (p != &mOtherQueues.back()) {
                    ++p;
                } else {
                    p = &mOtherQueues.front();
                }
                if (p == &mQueue) {
                    return std::nullopt;
                }
            }
        }

        Task<T> pop(std::chrono::steady_clock::duration interval =
                        std::chrono::milliseconds(400)) const {
            auto *p = &mQueue;
            while (true) {
                if (auto value = p->try_pop()) {
                    co_return std::move(*value);
                }
            again:
                if (p != &mOtherQueues.back()) {
                    ++p;
                } else {
                    p = &mOtherQueues.front();
                }
                if (p == &mQueue) {
                    if (auto value = co_await mQueue.pop(interval)) {
                        co_return std::move(*value);
                    } else {
                        goto again;
                    }
                }
            }
        }

        Task<> push(T &&value) const {
            return mQueue.push(std::move(value));
        }
    };

    Consumer consumer(std::size_t workerId) {
#if CO_ASYNC_DEBUG
        if (workerId >= mNumWorkers) [[unlikely]] {
            throw std::out_of_range("consumer id out of range");
        }
#endif
        return Consumer(mQueues[workerId],
                        std::span(mQueues.get(), mNumWorkers));
    }

    Task<> push(T &&value) {
        co_await mQueues[mNextIndex].push(std::move(value));
        mNextIndex = (mNextIndex + 1) % mNumWorkers;
    }
};

template <class T>
struct ConcurrentRobinhoodQueue {
private:
    std::unique_ptr<ConcurrentQueue<T>[]> mQueues;
    std::size_t mNumWorkers;
    std::size_t mNextIndex = 0;

public:
    explicit ConcurrentRobinhoodQueue(std::size_t numWorkers,
                                      std::size_t maxSize = 0)
        : mQueues(std::make_unique<ConcurrentQueue<T>[]>(numWorkers)),
          mNumWorkers(numWorkers) {
        if (maxSize) {
            for (std::size_t i = 0; i < mNumWorkers; ++i) {
                mQueues[i].set_max_size(maxSize);
            }
        }
    }

    void set_max_size(std::size_t maxSize) {
        for (std::size_t i = 0; i < mNumWorkers; ++i) {
            mQueues[i].set_max_size(maxSize);
        }
    }

    ConcurrentRobinhoodQueue(ConcurrentRobinhoodQueue &&) = delete;

    struct Consumer {
    private:
        ConcurrentQueue<T> &mQueue;
        std::span<ConcurrentQueue<T>> mOtherQueues;

    public:
        explicit Consumer(ConcurrentQueue<T> &queue,
                          std::span<ConcurrentQueue<T>> otherQueues)
            : mQueue(queue),
              mOtherQueues(otherQueues) {}

        std::optional<T> try_pop() const {
            return mQueue.try_pop();
        }

        Task<T> pop() const {
            co_return co_await mQueue.pop();
        }

        Task<> push(T &&value) const {
            return mQueue.push(std::move(value));
        }
    };

    Consumer consumer(std::size_t workerId) {
#if CO_ASYNC_DEBUG
        if (workerId >= mNumWorkers) [[unlikely]] {
            throw std::out_of_range("consumer id out of range");
        }
#endif
        return Consumer(mQueues[workerId],
                        std::span(mQueues.get(), mNumWorkers));
    }

    Task<> push(T &&value) {
        co_await mQueues[mNextIndex].push(std::move(value));
        mNextIndex = (mNextIndex + 1) % mNumWorkers;
    }
};
} // namespace co_async

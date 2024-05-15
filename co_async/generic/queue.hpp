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
    ConditionVariable mChanged;

public:
    explicit Queue(std::size_t size) : mQueue(size) {}

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mChanged;
        }
        mChanged.notify_one();
    }

    Task<T> pop(T value) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            co_await mChanged;
        }
    }
};

template <class T>
struct TimedQueue {
private:
    RingQueue<T> mQueue;
    TimedConditionVariable mChanged;

public:
    explicit TimedQueue(std::size_t size) : mQueue(size) {}

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mChanged.wait();
        }
        mChanged.notify_one();
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
            if (auto e = co_await mChanged.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
        mChanged.notify_one();
        co_return;
    }

    Task<T> pop() {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            co_await mChanged.wait();
        }
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::duration timeout) {
        return pop(std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::time_point expires) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            if (auto e = co_await mChanged.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
    }
};
} // namespace co_async

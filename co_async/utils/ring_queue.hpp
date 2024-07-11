#pragma once
#include <co_async/std.hpp>

namespace co_async {
template <class T>
struct RingQueue {
    std::unique_ptr<T[]> mHead;
    T *mTail;
    T *mRead;
    T *mWrite;

    explicit RingQueue(std::size_t maxSize = 0)
        : mHead(maxSize ? std::make_unique<T[]>(maxSize) : nullptr),
          mTail(maxSize ? mHead.get() + maxSize : nullptr),
          mRead(mHead.get()),
          mWrite(mHead.get()) {}

    void set_max_size(std::size_t maxSize) {
        mHead = maxSize ? std::make_unique<T[]>(maxSize) : nullptr;
        mTail = maxSize ? mHead.get() + maxSize : nullptr;
        mRead = mHead.get();
        mWrite = mHead.get();
    }

    [[nodiscard]] std::size_t max_size() const noexcept {
        return mTail - mHead.get();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return static_cast<std::size_t>(mWrite - mRead + max_size()) %
               max_size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return mRead == mWrite;
    }

    [[nodiscard]] bool full() const noexcept {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        return nextWrite == mRead;
    }

    [[nodiscard]] std::optional<T> pop() {
        if (mRead == mWrite) {
            return std::nullopt;
        }
        T p = std::move(*mRead);
        mRead = mRead == mTail ? mHead.get() : mRead + 1;
        return p;
    }

    [[nodiscard]] T pop_unchecked() {
        T p = std::move(*mRead);
        mRead = mRead == mTail ? mHead.get() : mRead + 1;
        return p;
    }

    [[nodiscard]] bool push(T &&value) {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        if (nextWrite == mRead) {
            return false;
        }
        *mWrite = std::move(value);
        mWrite = nextWrite;
        return true;
    }

    void push_unchecked(T &&value) {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        *mWrite = std::move(value);
        mWrite = nextWrite;
    }
};

template <class T>
struct InfinityQueue {
    [[nodiscard]] std::optional<T> pop() {
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T value = std::move(mQueue.front());
        mQueue.pop_front();
        return value;
    }

    [[nodiscard]] T pop_unchecked() {
        T value = std::move(mQueue.front());
        mQueue.pop_front();
        return value;
    }

    void push(T &&value) {
        mQueue.push_back(std::move(value));
    }

private:
    std::deque<T> mQueue;
};
} // namespace co_async

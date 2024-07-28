#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/utils/ring_queue.hpp>
#include <co_async/utils/spin_mutex.hpp>

namespace co_async {
template <class T, std::size_t Capacity = 0>
struct alignas(hardware_destructive_interference_size) ConcurrentRingQueue {
    static constexpr std::size_t Shift = std::bit_width(Capacity);
    using Stamp = std::conditional_t<
        Shift <= 4, std::uint8_t,
        std::conditional_t<
            Shift <= 8, std::uint16_t,
            std::conditional_t<Shift <= 16, std::uint32_t, std::uint64_t>>>;
    static_assert(Shift * 2 <= sizeof(Stamp) * 8);
    static_assert(Capacity < (1 << Shift));
    static constexpr Stamp kSize = 1 << Shift;

    [[nodiscard]] std::optional<T> pop() {
        auto s = mStamp.load(std::memory_order_relaxed);
        if (!canRead(s)) {
            return std::nullopt;
        }
        while (!mStamp.compare_exchange_weak(s, advectRead(s),
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
            if (!canRead(s)) {
                return std::nullopt;
            }
        }
        return std::move(mHead[offsetRead(s)]);
    }

    [[nodiscard]] bool push(T &&value) {
        auto s = mStamp.load(std::memory_order_relaxed);
        if (!canWrite(s)) [[unlikely]] {
            return false;
        }
        while (!mStamp.compare_exchange_weak(s, advectWrite(s),
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
            if (!canWrite(s)) [[unlikely]] {
                return false;
            }
        }
        mHead[offsetWrite(s)] = std::move(value);
        return true;
    }

    ConcurrentRingQueue() = default;
    ConcurrentRingQueue(ConcurrentRingQueue &&) = delete;

private:
    inline Stamp offsetRead(Stamp s) const {
        return s >> Shift;
    }

    inline Stamp offsetWrite(Stamp s) const {
        return s & (kSize - 1);
    }

    inline bool canRead(Stamp s) const {
        return offsetRead(s) != offsetWrite(s);
    }

    inline bool canWrite(Stamp s) const {
        return (offsetRead(s) & static_cast<Stamp>(kSize - 1)) !=
               ((offsetWrite(s) + static_cast<Stamp>(kSize - Capacity)) &
                static_cast<Stamp>(kSize - 1));
    }

    inline Stamp advectRead(Stamp s) const {
        return static_cast<Stamp>(
                   ((static_cast<Stamp>(s >> Shift) + static_cast<Stamp>(1)) &
                    static_cast<Stamp>(kSize - 1))
                   << Shift) |
               (s & static_cast<Stamp>(kSize - 1));
    }

    inline Stamp advectWrite(Stamp s) const {
        return (((s & static_cast<Stamp>(kSize - 1)) + static_cast<Stamp>(1)) &
                static_cast<Stamp>(kSize - 1)) |
               static_cast<Stamp>(s & (static_cast<Stamp>(kSize - 1) << Shift));
    }

    std::unique_ptr<T[]> const mHead = std::make_unique<T[]>(kSize);
    std::atomic<Stamp> mStamp{0};
};

template <class T>
struct alignas(hardware_destructive_interference_size)
    ConcurrentRingQueue<T, 0> {
    std::optional<T> pop() {
        std::lock_guard lck(mMutex);
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T p = std::move(mQueue.front());
        mQueue.pop_front();
        return p;
    }

    void push(T &&value) {
        std::lock_guard lck(mMutex);
        mQueue.push_back(std::move(value));
    }

private:
    std::deque<T> mQueue;
    SpinMutex mMutex;
};
} // namespace co_async

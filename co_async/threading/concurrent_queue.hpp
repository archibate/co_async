#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {

inline void assume(bool v) {
#if defined(__GNUC__) && __has_builtin(__builtin_unreachable)
    if (!v) [[unlikely]] {
#if CO_ASYNC_DEBUG
        throw std::logic_error("assumption failed");
#else
        __builtin_unreachable();
#endif
    }
#endif
}

#if 1
template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
    std::optional<T> pop() {
        std::lock_guard lck(mMutex);
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T p = std::move(mQueue.front());
        mQueue.pop_front();
        return p;
    }

    bool push(T p) {
        std::lock_guard lck(mMutex);
        mQueue.push_back(p);
        return true;
    }

private:
    std::deque<T> mQueue;
    std::mutex mMutex;
};
#else
template <class T, std::size_t Shift = 12, class Stamp = std::uint32_t>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
    static constexpr Stamp kSize = 1 << Shift;

    [[nodiscard]] std::optional<T> pop() {
        auto s = mStamp.load(std::memory_order_acquire);
        if (!canRead(s)) {
            /* mStamp.compare_exchange_weak(s, Stamp(0)); */
            return std::nullopt;
        }
        while (!mStamp.compare_exchange_weak(s, advectRead(s),
                                             std::memory_order_acq_rel)) {
            if (!canRead(s)) {
                return std::nullopt;
            }
        }
        return mHead[offsetRead(s)];
    }

    [[nodiscard]] bool push(T value) {
        auto s = mStamp.load(std::memory_order_acquire);
        if (!canWrite(s)) [[unlikely]] {
            return false;
        }
        while (!mStamp.compare_exchange_weak(s, advectWrite(s),
                                             std::memory_order_acq_rel)) {
            if (!canWrite(s)) [[unlikely]] {
                return false;
            }
        }
        mHead[offsetWrite(s)] = std::move(value);
        return true;
    }

    ConcurrentQueue() = default;
    ConcurrentQueue(ConcurrentQueue &&) = delete;

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
        return offsetRead(s) != offsetWrite(s) + 1;
    }

    inline Stamp advectRead(Stamp s) const {
        return ((((s >> Shift) + 1) & (kSize - 1)) << Shift) |
               (s & (kSize - 1));
    }

    inline Stamp advectWrite(Stamp s) const {
        return (((s & (kSize - 1)) + 1) & (kSize - 1)) |
               (s & ((kSize - 1) << Shift));
    }

    std::unique_ptr<T[]> mHead = std::make_unique<T[]>(kSize);
    std::atomic<Stamp> mStamp{0};
};
#endif

/* template <class T, std::size_t N = 256, std::size_t Shift = 8> */
/* struct alignas(hardware_destructive_interference_size) ConcurrentQueue { */
/*     using Block = ConcurrentQueueBlock<T, Shift>; */
/*  */
/*     Block mBlocks[N]; */
/*     std::size_t mTop{0}; */
/*  */
/*     [[nodiscard]] std::optional<T> pop() { */
/*         auto ret = mBlocks[0].pop(); */
/*         if (ret || !mTop) [[likely]] { */
/*             return ret; */
/*         } */
/*         for (std::size_t i = 1; i <= mTop; ++i) { */
/*             ret = mBlocks[i].pop(); */
/*             if (ret) [[likely]] { */
/*                 return ret; */
/*             } */
/*         } */
/*         return std::nullopt; */
/*     } */
/* }; */

} // namespace co_async

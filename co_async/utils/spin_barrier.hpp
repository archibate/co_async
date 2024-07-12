#pragma once

#include <co_async/std.hpp>

namespace co_async {

struct SpinBarrier {
    explicit SpinBarrier(std::size_t n) noexcept
        : m_top_waiting(static_cast<std::uint32_t>(n) - 1),
          m_num_waiting(0),
          m_sync_flip(0) {}

    bool arrive_and_wait() noexcept {
        bool old_flip = m_sync_flip.load(std::memory_order_relaxed);
        if (m_num_waiting.fetch_add(1, std::memory_order_relaxed) ==
            m_top_waiting) {
            m_num_waiting.store(0, std::memory_order_relaxed);
            m_sync_flip.store(!old_flip, std::memory_order_release);
            return true;
        } else {
            while (m_sync_flip.load(std::memory_order_acquire) == old_flip)
                ;
#if __cpp_lib_atomic_wait
            m_sync_flip.wait(old_flip, std::memory_order_acquire);
#endif
            return false;
        }
    }

    bool arrive_and_drop() noexcept {
        bool old_flip = m_sync_flip.load(std::memory_order_relaxed);
        if (m_num_waiting.fetch_add(1, std::memory_order_relaxed) ==
            m_top_waiting) {
            m_num_waiting.store(0, std::memory_order_relaxed);
            m_sync_flip.store(!old_flip, std::memory_order_release);
            return true;
        } else {
            return false;
        }
    }

private:
    std::uint32_t const m_top_waiting;
    FutexAtomic<std::uint32_t> m_num_waiting;
    FutexAtomic<bool> m_sync_flip;
};

} // namespace co_async

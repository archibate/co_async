#pragma once
#include <co_async/std.hpp>
#include "co_async/utils/debug.hpp"
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/platform_io.hpp>
#include <linux/futex.h>
#include <unistd.h>

namespace co_async {
inline Task<Expected<>> futex_wait(void *futex, uint32_t val, uint32_t mask = FUTEX_BITSET_MATCH_ANY, uint32_t futex_flags = FUTEX_PRIVATE_FLAG) {
    debug(), "wait";
    // co_return expectError(co_await UringOp().prep_futex_wait(reinterpret_cast<uint32_t *>(futex), static_cast<uint64_t>(val), static_cast<uint64_t>(mask), futex_flags, 0));
    struct futex_waitv waits[1] = {
        {
            .val = (__u64)val,
            .uaddr = (__u64)futex,
            .flags = futex_flags,
            .__reserved = {},
        },
    };
    co_return expectError(co_await UringOp().prep_futex_waitv({waits, 0}, 1));
}

inline Task<Expected<>> futex_wake(void *futex, std::size_t count = static_cast<std::size_t>(-1), uint32_t mask = FUTEX_BITSET_MATCH_ANY, uint32_t futex_flags = FUTEX_PRIVATE_FLAG) {
    debug(), "wake";
    co_return expectError(co_await UringOp().prep_futex_wake(reinterpret_cast<uint32_t *>(futex), static_cast<uint64_t>(count), static_cast<uint64_t>(mask), futex_flags, 0));
}

inline void futex_notify(void *futex, std::size_t count = static_cast<std::size_t>(-1), uint32_t mask = FUTEX_BITSET_MATCH_ANY, uint32_t futex_flags = FUTEX_PRIVATE_FLAG) {
    debug(), "notify";
    UringOp().prep_futex_wake(reinterpret_cast<uint32_t *>(futex), static_cast<uint64_t>(count), static_cast<uint64_t>(mask), futex_flags, 0).detach();
}

// struct Futex {
// private:
//     std::atomic<std::uint32_t> mValue;
//
// public:
//     std::atomic<std::uint32_t> &raw_atomic() {
//         return mValue;
//     }
//
//     std::atomic<std::uint32_t> const &raw_atomic() const {
//         return mValue;
//     }
//
//     std::uint32_t load() const {
//         return mValue.load(std::memory_order_relaxed);
//     }
//
//     std::uint32_t fetch_add(std::uint32_t n) {
//         std::uint32_t old = mValue.fetch_add(n, std::memory_order_release);
//         return old;
//     }
//
//     std::uint32_t fetch_sub(std::uint32_t n) {
//         std::uint32_t old = mValue.fetch_sub(n, std::memory_order_release);
//         return old;
//     }
//
//     std::uint32_t fetch_xor(std::uint32_t n) {
//         std::uint32_t old = mValue.fetch_xor(n, std::memory_order_release);
//         return old;
//     }
//
//     Task<Expected<>> wait(std::uint32_t old) {
//         do
//             co_await co_await futex_wait(&mValue, old);
//         while (mValue.load(std::memory_order_acquire) == old);
//         co_return {};
//     }
//
//     Task<Expected<>> wait() {
//         return wait(load());
//     }
//
//     void notify_one() {
//         futex_wake_detach(&mValue, 1);
//     }
//
//     void notify_all() {
//         futex_wake_detach(&mValue, static_cast<std::uint32_t>(-1));
//     }
// };

} // namespace co_async

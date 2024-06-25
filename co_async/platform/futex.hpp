#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/platform_io.hpp>
#include <linux/futex.h>
#include <unistd.h>

namespace co_async {
template <class T>
inline constexpr uint32_t getFutexFlagsFor() {
    switch (sizeof(T)) {
    case sizeof(uint8_t):  return FUTEX2_SIZE_U8 | FUTEX2_PRIVATE;
    case sizeof(uint16_t): return FUTEX2_SIZE_U16 | FUTEX2_PRIVATE;
    case sizeof(uint32_t): return FUTEX2_SIZE_U32 | FUTEX2_PRIVATE;
    case sizeof(uint64_t): return FUTEX2_SIZE_U64 | FUTEX2_PRIVATE;
    }
}

template <class T>
inline constexpr uint64_t futexValueExtend(T value) {
    uint64_t ret = 0;
    std::memcpy(&ret, &value, sizeof(T));
    return ret;
}

template <class T>
inline Task<Expected<>> futex_wait(std::atomic<T> *futex,
                                   std::type_identity_t<T> val,
                                   uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(co_await UringOp().prep_futex_wait(
        reinterpret_cast<uint32_t *>(futex),
        futexValueExtend(val),
        static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0).cancelGuard(co_await co_cancel));
}

template <class T>
inline Task<Expected<>>
futex_wake(std::atomic<T> *futex,
           std::size_t count = static_cast<std::size_t>(-1),
           uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(co_await UringOp().prep_futex_wake(
        reinterpret_cast<uint32_t *>(futex), static_cast<uint64_t>(count),
        static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0).cancelGuard(co_await co_cancel));
}

template <class T>
inline void futex_notify(std::atomic<T> *futex,
                         std::size_t count = static_cast<std::size_t>(-1),
                         uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    UringOp()
        .prep_futex_wake(reinterpret_cast<uint32_t *>(futex),
                         static_cast<uint64_t>(count),
                         static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0)
        .startDetach();
}

template <class T>
inline void futex_notify_sync(std::atomic<T> *futex,
                              std::size_t count = static_cast<std::size_t>(-1),
                              uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    syscall(SYS_futex_wake, reinterpret_cast<uint32_t *>(futex),
            static_cast<uint64_t>(count), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
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

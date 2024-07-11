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
#ifdef FUTEX2_PRIVATE
    case sizeof(uint8_t):  return FUTEX2_SIZE_U8 | FUTEX2_PRIVATE;
    case sizeof(uint16_t): return FUTEX2_SIZE_U16 | FUTEX2_PRIVATE;
    case sizeof(uint32_t): return FUTEX2_SIZE_U32 | FUTEX2_PRIVATE;
    case sizeof(uint64_t): return FUTEX2_SIZE_U64 | FUTEX2_PRIVATE;
#else
    case sizeof(uint8_t):  return 0 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint16_t): return 1 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint32_t): return 2 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint64_t): return 3 | FUTEX_PRIVATE_FLAG;
#endif
    }
}

template <class T>
inline constexpr uint64_t futexValueExtend(T value) {
    static_assert(std::is_trivial_v<T> && sizeof(T) <= sizeof(uint64_t));
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
        static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0).cancelGuard(co_await co_cancel)).ignore_error(std::errc::resource_unavailable_try_again);
}

template <class T>
inline Task<Expected<>>
futex_notify_async(std::atomic<T> *futex,
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

template <class T>
inline void futex_wait_sync(std::atomic<T> *futex,
                            std::size_t count = static_cast<std::size_t>(-1),
                            uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    syscall(SYS_futex_wait, reinterpret_cast<uint32_t *>(futex),
            static_cast<uint64_t>(count), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
}

} // namespace co_async

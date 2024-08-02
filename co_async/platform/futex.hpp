#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/platform_io.hpp>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace co_async {

inline constexpr std::size_t kFutexNotifyAll = static_cast<std::size_t>(std::numeric_limits<int>::max());

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
inline Expected<> futex_notify_sync(std::atomic<T> *futex,
                                    std::size_t count = kFutexNotifyAll,
                                    uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#ifndef SYS_futex_wake
    const long SYS_futex_wake = 454;
#endif
    long res = syscall(SYS_futex_wake, reinterpret_cast<uint32_t *>(futex),
            static_cast<uint64_t>(count), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
#if CO_ASYNC_INVALFIX
    if (res == -EBADF || res == -ENOSYS) {
        res = syscall(SYS_futex, reinterpret_cast<uint32_t *>(futex), FUTEX_WAKE_BITSET_PRIVATE,
                static_cast<uint32_t>(count), nullptr, nullptr, mask);
    }
#endif
    return expectError(static_cast<int>(res));
}

template <class T>
inline Expected<> futex_wait_sync(std::atomic<T> *futex,
                            std::type_identity_t<T> val,
                            uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#ifndef SYS_futex_wait
    const long SYS_futex_wait = 455;
#endif
    long res = syscall(SYS_futex_wait, reinterpret_cast<uint32_t *>(futex),
            futexValueExtend(val), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
#if CO_ASYNC_INVALFIX
    if (res == -EBADF || res == -ENOSYS) {
        res = syscall(SYS_futex, reinterpret_cast<uint32_t *>(futex), FUTEX_WAIT_BITSET_PRIVATE,
                static_cast<uint32_t>(futexValueExtend(val)), nullptr, nullptr, mask);
    }
#endif
    return expectError(static_cast<int>(res));
}


template <class T>
inline Task<Expected<>> futex_wait(std::atomic<T> *futex,
                                   std::type_identity_t<T> val,
                                   uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(
        co_await UringOp()
            .prep_futex_wait(reinterpret_cast<uint32_t *>(futex),
                             futexValueExtend(val), static_cast<uint64_t>(mask),
                             getFutexFlagsFor<T>(), 0)
            .cancelGuard(co_await co_cancel)).transform([] (int) {})
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] {
            return futex_wait_sync(futex, val, mask);
        })
#endif
        .ignore_error(std::errc::resource_unavailable_try_again)
    ;
}

template <class T>
inline Task<Expected<>>
futex_notify_async(std::atomic<T> *futex,
                   std::size_t count = kFutexNotifyAll,
                   uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(
        co_await UringOp()
            .prep_futex_wake(reinterpret_cast<uint32_t *>(futex),
                             static_cast<uint64_t>(count),
                             static_cast<uint64_t>(mask), getFutexFlagsFor<T>(),
                             0)
            .cancelGuard(co_await co_cancel)).transform([] (int) {})
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] {
            return futex_notify_sync(futex, count, mask);
        })
#endif
    ;
}

template <class T>
inline void futex_notify(std::atomic<T> *futex,
                         std::size_t count = kFutexNotifyAll,
                         uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#if CO_ASYNC_INVALFIX
    futex_notify_sync(futex, count, mask);
#else
    UringOp()
        .prep_futex_wake(reinterpret_cast<uint32_t *>(futex),
                         static_cast<uint64_t>(count),
                         static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0)
        .startDetach();
#endif
}

#if CO_ASYNC_INVALFIX
template <class>
using FutexAtomic = std::atomic<uint32_t>;
#else
template <class T>
using FutexAtomic = std::atomic<T>;
#endif

} // namespace co_async

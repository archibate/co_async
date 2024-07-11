#pragma once
#include <co_async/std.hpp>

namespace co_async {
#if CO_ASYNC_ALLOC
using String = std::pmr::string;
#else
using String = std::string;
#endif

inline String operator""_s(char const *str, size_t len) {
    return String(str, len);
}

extern thread_local std::pmr::memory_resource *currentAllocator;

struct ReplaceAllocator {
    ReplaceAllocator(std::pmr::memory_resource *allocator) {
        lastAllocator = currentAllocator;
        currentAllocator = allocator;
    }

    ReplaceAllocator(ReplaceAllocator &&) = delete;

    ~ReplaceAllocator() {
        currentAllocator = lastAllocator;
    }

private:
    std::pmr::memory_resource *lastAllocator;
};

} // namespace co_async

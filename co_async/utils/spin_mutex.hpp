#pragma once

#include <co_async/std.hpp>

namespace co_async {

struct SpinMutex {
    bool try_lock() {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

    std::atomic_flag flag{false};
};

} // namespace co_async

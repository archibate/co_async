#pragma once /*{export module co_async:threading.concurrent_queue;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

namespace co_async {

struct ConcurrentQueue {
    void *pop() {
        std::lock_guard lck(mMutex);
        if (mQueue.empty()) {
            return nullptr;
        }
        void *p = mQueue.front();
        mQueue.pop_front();
#if defined(__GNUC__) && __has_builtin(__builtin_unreachable)
        if (!p) [[unlikely]] __builtin_unreachable();
#endif
        return p;
    }

    void push(void *p) {
#if defined(__GNUC__) && __has_builtin(__builtin_unreachable)
        if (p) [[unlikely]] __builtin_unreachable();
#endif
        std::lock_guard lck(mMutex);
        mQueue.push_back(p);
    }

private:
    std::deque<void *> mQueue;
    std::mutex mMutex;
};

}

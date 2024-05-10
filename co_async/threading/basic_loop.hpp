#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/concurrent_queue.hpp>

namespace co_async {

struct alignas(hardware_destructive_interference_size) BasicLoop {
    bool run() {
        if (auto coroutine = mQueue.pop()) {
            coroutine->resume();
            return true;
        }
        return false;
    }

    void enqueue(std::coroutine_handle<> coroutine) {
        if (!mQueue.push(coroutine)) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: coroutine queue overrun\n";
#endif
            while (true) {
                if (auto coroutine = mQueue.pop()) [[likely]] {
                    coroutine->resume();
                }
                if (mQueue.push(coroutine)) [[likely]] {
                    break;
                }
            }
        }
    }

    BasicLoop() = default;
    BasicLoop(BasicLoop &&) = delete;

    static inline thread_local BasicLoop *tlsInstance;

private:
    ConcurrentQueue<std::coroutine_handle<>, (1 << 16) - 1> mQueue;
};

template <class A>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
loopEnqueueDetachStarter(A awaitable) {
    (void)co_await std::move(awaitable);
}

template <class A>
inline void loopEnqueueDetached(BasicLoop &loop, A awaitable) {
    auto wrapped = loopEnqueueDetachStarter(std::move(awaitable));
    auto coroutine = wrapped.get();
    loop.enqueue(coroutine);
    wrapped.release();
}

inline void loopEnqueueHandle(BasicLoop &loop, std::coroutine_handle<> coroutine) {
    loop.enqueue(coroutine);
}

template <class T, class P>
inline Task<> loopEnqueueSyncStarter(Task<T, P> task,
                                     std::condition_variable &cv,
                                     Uninitialized<T> &result
#if CO_ASYNC_EXCEPT
                                     ,
                                     std::exception_ptr exception
#endif
) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.putValue((co_await task, Void()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
#if CO_ASYNC_DEBUG
        std::cerr << "WARNING: exception occurred in co_synchronize\n";
#endif
        exception = std::current_exception();
    }
#endif
    cv.notify_one();
}

template <class T, class P>
inline T loopEnqueueSynchronized(BasicLoop &loop, Task<T, P> task) {
    std::condition_variable cv;
    std::mutex mtx;
    Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
    std::exception_ptr exception;
#endif
    loopEnqueueDetached(loop, loopEnqueueSyncStarter(std::move(task), cv, result
#if CO_ASYNC_EXCEPT
                                                     ,
                                                     exception
#endif
                                                     ));
    std::unique_lock lck(mtx);
    cv.wait(lck);
    lck.unlock();
#if CO_ASYNC_EXCEPT
    if (exception) [[unlikely]] {
        std::rethrow_exception(exception);
    }
#endif
    if constexpr (!std::is_void_v<T>) {
        return result.moveValue();
    }
}

} // namespace co_async

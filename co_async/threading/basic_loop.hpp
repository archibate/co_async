#pragma once /*{export module co_async:threading.basic_loop;}*/

#include <co_async/std.hpp>             /*{import std;}*/
#include <co_async/awaiter/concepts.hpp>/*{import :awaiter.concepts;}*/
#include <co_async/awaiter/task.hpp>    /*{import :awaiter.task;}*/
#include <co_async/awaiter/details/ignore_return_promise.hpp>/*{import :awaiter.details.ignore_return_promise;}*/
#include <co_async/utils/uninitialized.hpp>  /*{import :utils.uninitialized;}*/
#include <co_async/utils/non_void_helper.hpp>/*{import :utils.non_void_helper;}*/
#include <co_async/threading/concurrent_queue.hpp>/*{import :threading.concurrent_queue;}*/

namespace co_async {

struct BasicLoop {
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
    ConcurrentQueue<std::coroutine_handle<>> mQueue;
};

template <class T, class P>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
loopEnqueueDetachStarter(Task<T, P> task) {
    co_await task;
}

template <class T, class P>
inline void loop_enqueue_detached(BasicLoop &loop, Task<T, P> task) {
    auto wrapped = loopEnqueueDetachStarter(std::move(task));
    auto coroutine = wrapped.get();
    loop.enqueue(coroutine);
    wrapped.release();
}

template <class T, class P>
inline Task<> loopEnqueueSyncStarter(Task<T, P> task, std::condition_variable &cv, Uninitialized<T> &result
#if CO_ASYNC_EXCEPT
                                ,
                                std::exception_ptr exception
#endif
) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.putValue((co_await task, NonVoidHelper<>()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        exception = std::current_exception();
    }
#endif
    cv.notify_one();
}

template <class T, class P>
inline T loop_enqueue_synchronized(BasicLoop &loop, Task<T, P> task) {
    std::condition_variable cv;
    std::mutex mtx;
    Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
    std::exception_ptr exception;
#endif
    loop_enqueue_detached(loop, loopEnqueueSyncStarter(std::move(task), cv, result
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

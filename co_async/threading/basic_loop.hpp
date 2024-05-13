#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/concurrent_queue.hpp>
#include <co_async/threading/cancel.hpp>

namespace co_async {

struct BasicLoop;

struct alignas(hardware_destructive_interference_size) BasicLoop {
    struct TimerNode : CustomPromise<Expected<>, TimerNode>, RbTree<TimerNode>::NodeType {
        using RbTree<TimerNode>::NodeType::destructiveErase;

        std::chrono::steady_clock::time_point mExpires;
        bool mCancelled = false;

        bool operator<(TimerNode const &that) const {
            return mExpires < that.mExpires;
        }

        struct Awaiter {
            std::chrono::steady_clock::time_point mExpires;
            BasicLoop *mLoop;
            TimerNode *mPromise = nullptr;

            bool await_ready() const noexcept {
                return false;
            }

            inline void await_suspend(std::coroutine_handle<TimerNode> coroutine);

            Expected<> await_resume() const {
                if (!mPromise->mCancelled) {
                    return {};
                } else {
                    return Unexpected{std::make_error_code(std::errc::operation_canceled)};
                }
            }
        };

        struct Canceller {
            using OpType = Task<Expected<>, BasicLoop::TimerNode>;

            static Task<> doCancel(OpType *op) {
                auto &promise = op->get().promise();
                promise.mCancelled = true;
                promise.destructiveErase();
                BasicLoop::tlsInstance->enqueue(op->get());
                co_return;
            }

            static Expected<> earlyCancelValue(OpType *op) {
                return Unexpected{std::make_error_code(std::errc::operation_canceled)};
            }
        };
    };

    bool run() {
        if (auto coroutine = mQueue.pop()) {
            coroutine->resume();
            return true;
        }
        return false;
    }

    std::optional<std::chrono::nanoseconds> runTimers(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) {
        while (true) {
            if (mTimers.empty()) return std::nullopt;
            auto &promise = mTimers.front();
            if (promise.mExpires <= now) {
                promise.mCancelled = false;
                promise.destructiveErase();
                enqueue(std::coroutine_handle<TimerNode>::from_promise(promise));
            } else {
                debug(), duration_cast<std::chrono::milliseconds>(promise.mExpires - now);
                return promise.mExpires - now;
            }
        }
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

    void enqueueTimer(TimerNode &promise) {
        mTimers.insert(promise);
    }

    BasicLoop() = default;
    BasicLoop(BasicLoop &&) = delete;

    static inline thread_local BasicLoop *tlsInstance;

private:
    ConcurrentQueue<std::coroutine_handle<>, (1 << 12) - 1> mQueue;
    RbTree<TimerNode> mTimers;
};

inline void BasicLoop::TimerNode::Awaiter::await_suspend(std::coroutine_handle<BasicLoop::TimerNode> coroutine) {
    mPromise = &coroutine.promise();
    mPromise->mExpires = mExpires;
    mLoop->enqueueTimer(*mPromise);
}

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

inline Task<Expected<>, BasicLoop::TimerNode> loopWaitTimer(BasicLoop &loop, std::chrono::steady_clock::time_point expires) {
    co_return co_await BasicLoop::TimerNode::Awaiter(expires, &loop);
}

inline Task<Expected<>, BasicLoop::TimerNode> loopWaitTimer(BasicLoop &loop, std::chrono::steady_clock::time_point expires, CancelToken cancel) {
    co_return co_await cancel.invoke<BasicLoop::TimerNode::Canceller>(loopWaitTimer(loop, expires));
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

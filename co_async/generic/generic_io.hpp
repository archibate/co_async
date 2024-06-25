#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/spin_mutex.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/utils/ring_queue.hpp>
#if CO_ASYNC_STEAL
# include <co_async/utils/concurrent_queue.hpp>
#endif
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {
struct IOContext;

struct GenericIOContext {
    struct TimerNode : CustomPromise<Expected<>, TimerNode>,
                       RbTree<TimerNode>::NodeType {
        using RbTree<TimerNode>::NodeType::erase_from_parent;
        std::chrono::steady_clock::time_point mExpires;
        CancelToken mCancelToken;
        bool mCancelled = false;

        void doCancel() {
            mCancelled = true;
            erase_from_parent();
        }

        bool operator<(TimerNode const &that) const {
            return mExpires < that.mExpires;
        }

        struct Awaiter {
            std::chrono::steady_clock::time_point mExpires;
            TimerNode *mPromise = nullptr;

            bool await_ready() const noexcept {
                return false;
            }

            inline void
            await_suspend(std::coroutine_handle<TimerNode> coroutine);

            Expected<> await_resume() const {
                if (!mPromise->mCancelled) {
                    return {};
                } else {
                    return 
                        std::errc::operation_canceled;
                }
            }
        };

        /* struct Canceller { */
        /*     using OpType = Task<Expected<>, GenericIOContext::TimerNode>; */
        /*  */
        /*     static Task<> doCancel(OpType *op) { */
        /*         auto &promise = op->get().promise(); */
        /*         promise.mCancelled = true; */
        /*         promise.erase_from_parent(); */
        /*         GenericIOContext::instance->enqueueJob(op->get()); */
        /*         co_return; */
        /*     } */
        /*  */
        /*     static Expected<> earlyCancelValue(OpType *op) { */
        /*         return std::errc::operation_canceled; */
        /*     } */
        /* }; */
    };

    bool runComputeOnly() {
        std::unique_lock lock(mMutex);
        if (auto coroutine = mQueue.pop()) {
            lock.unlock();
            coroutine->resume();
            return true;
        }
        lock.unlock();
        return false;
    }

    std::optional<std::chrono::steady_clock::duration> runDuration() {
        while (true) {
            std::unique_lock lock(mMutex);
            while (auto coroutine = mQueue.pop()) {
                lock.unlock();
                coroutine->resume();
                lock.lock();
            }
            lock.unlock();
            if (!mTimers.empty()) {
                auto &promise = mTimers.front();
                std::chrono::steady_clock::time_point now =
                    std::chrono::steady_clock::now();
                /* now += std::chrono::nanoseconds(1000); */
                if (promise.mExpires <= now) {
                    promise.mCancelled = false;
                    promise.erase_from_parent();
                    auto coroutine =
                        std::coroutine_handle<TimerNode>::from_promise(promise);
                    enqueueJob(coroutine);
                    continue;
                } else {
                    return promise.mExpires - now;
                }
            } else {
                return std::nullopt;
            }
        }
    }

    void enqueueJob(std::coroutine_handle<> coroutine) {
        std::unique_lock lock(mMutex);
        mQueue.push(std::move(coroutine));
    }

    void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            auto duration = runDuration();
            if (duration) {
                std::this_thread::sleep_for(*duration);
            } else {
                break;
            }
        }
    }

    GenericIOContext() = default;

    GenericIOContext(GenericIOContext &&) = delete;
    static inline thread_local GenericIOContext *instance;

private:
#if CO_ASYNC_STEAL
    ConcurrentRingQueue<std::coroutine_handle<>, 1024 - 1> mQueue;
#else
    /* RingQueue<std::coroutine_handle<>> mQueue{1024}; */
    InfinityQueue<std::coroutine_handle<>> mQueue;
    SpinMutex mMutex;
#endif
    RbTree<TimerNode> mTimers;
};

inline void GenericIOContext::TimerNode::Awaiter::await_suspend(
    std::coroutine_handle<GenericIOContext::TimerNode> coroutine) {
    mPromise = &coroutine.promise();
    mPromise->mExpires = mExpires;
    GenericIOContext::instance->enqueueTimerNode(*mPromise);
}

template <class A>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
coSpawnStarter(A awaitable) {
    (void)co_await std::move(awaitable);
}

template <Awaitable A>
inline void co_spawn(A awaitable) {
    auto wrapped = coSpawnStarter(std::move(awaitable));
    auto coroutine = wrapped.get();
    GenericIOContext::instance->enqueueJob(coroutine);
    wrapped.release();
}

inline void co_spawn(std::coroutine_handle<> coroutine) {
    GenericIOContext::instance->enqueueJob(coroutine);
}

inline auto co_resume() {
    struct ResumeAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            co_spawn(coroutine);
        }

        void await_resume() const noexcept {}
    };

    return ResumeAwaiter();
}
} // namespace co_async

#pragma once
#include <co_async/std.hpp>
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
        using RbTree<TimerNode>::NodeType::destructiveErase;
        std::chrono::steady_clock::time_point mExpires;
        bool mCancelled = false;

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

        struct Canceller {
            using OpType = Task<Expected<>, GenericIOContext::TimerNode>;

            static Task<> doCancel(OpType *op) {
                auto &promise = op->get().promise();
                promise.mCancelled = true;
                promise.destructiveErase();
                GenericIOContext::instance->enqueueJob(op->get());
                co_return;
            }

            static Expected<> earlyCancelValue(OpType *op) {
                return std::errc::operation_canceled;
            }
        };
    };

    bool runComputeOnly() {
        if (auto coroutine = mQueue.pop()) {
            coroutine->resume();
            return true;
        }
        return false;
    }

    bool runMTQueue() {
        std::unique_lock lock(mMTMutex);
        if (!mMTQueue.empty()) {
            auto coroutine = mMTQueue.front();
            mMTQueue.pop_front();
            lock.unlock();
            enqueueJob(coroutine);
            return true;
        }
        return false;
    }

    std::optional<std::chrono::steady_clock::duration> runDuration() {
        while (true) {
            while (auto coroutine = mQueue.pop()) {
                coroutine->resume();
            }
            if (!mTimers.empty()) {
                auto &promise = mTimers.front();
                std::chrono::steady_clock::time_point now =
                    std::chrono::steady_clock::now();
                if (promise.mExpires <= now) {
                    promise.mCancelled = false;
                    promise.destructiveErase();
                    auto coroutine =
                        std::coroutine_handle<TimerNode>::from_promise(promise);
                    enqueueJob(coroutine);
                    continue;
                } else {
                    return promise.mExpires - now;
                }
            }
            return std::nullopt;
        }
    }

    void enqueueJob(std::coroutine_handle<> coroutine) {
        mQueue.push(std::move(coroutine));
        /*         if (!mQueue.push(std::move(coroutine))) [[unlikely]] { */
        /* #if CO_ASYNC_DEBUG */
        /*             std::cerr << "WARNING: coroutine queue overrun\n"; */
        /* #endif */
        /*             std::lock_guard lock(mMTMutex); */
        /*             mMTQueue.push_back(coroutine); */
        /*         } */
    }

    void enqueueJobMT(std::coroutine_handle<> coroutine) {
#if CO_ASYNC_STEAL
        enqueueJob(coroutine);
#else
        std::lock_guard lock(mMTMutex);
        mMTQueue.push_back(coroutine);
#endif
    }

    void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            auto duration = runDuration();
            if (runMTQueue()) {
                continue;
            }
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
#endif
    RbTree<TimerNode> mTimers;
    std::mutex mMTMutex;
    std::list<std::coroutine_handle<>> mMTQueue;
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

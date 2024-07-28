#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/utils/cacheline.hpp>
#include <co_async/utils/concurrent_queue.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/utils/ring_queue.hpp>
#include <co_async/utils/spin_mutex.hpp>
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
                    return std::errc::operation_canceled;
                }
            }
        };
    };

    [[gnu::hot]] std::optional<std::chrono::steady_clock::duration>
    runDuration();

    // [[gnu::hot]] void enqueueJob(std::coroutine_handle<> coroutine) {
    //     mQueue.push(std::move(coroutine));
    // }

    [[gnu::hot]] void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    GenericIOContext();
    ~GenericIOContext();

    GenericIOContext(GenericIOContext &&) = delete;
    static inline thread_local GenericIOContext *instance;

private:
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
    auto coroutine = wrapped.release();
    coroutine.resume();
}

inline void co_spawn(std::coroutine_handle<> coroutine) {
    coroutine.resume();
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

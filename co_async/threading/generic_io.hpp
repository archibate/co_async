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

struct GenericIOContext {
    struct TimerNode : CustomPromise<Expected<>, TimerNode>, RbTree<TimerNode>::NodeType {
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
            using OpType = Task<Expected<>, GenericIOContext::TimerNode>;

            static Task<> doCancel(OpType *op) {
                auto &promise = op->get().promise();
                promise.mCancelled = true;
                promise.destructiveErase();
                GenericIOContext::instance->enqueueJob(op->get());
                co_return;
            }

            static Expected<> earlyCancelValue(OpType *op) {
                return Unexpected{std::make_error_code(std::errc::operation_canceled)};
            }
        };
    };

    std::optional<std::chrono::steady_clock::duration> runDuration() {
        while (true) {
            if (!mQueue.empty()) {
                auto coroutine = mQueue.front();
                mQueue.pop_front();
                coroutine.resume();
                continue;
            }
            if (mTimers.empty()) return std::nullopt;
            auto &promise = mTimers.front();
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (promise.mExpires <= now) {
                promise.mCancelled = false;
                promise.destructiveErase();
                enqueueJob(std::coroutine_handle<TimerNode>::from_promise(promise));
            } else {
                /* debug(), duration_cast<std::chrono::milliseconds>(promise.mExpires - now); */
                return promise.mExpires - now;
            }
        }
    }

    void enqueueJob(std::coroutine_handle<> coroutine) {
        mQueue.push_back(coroutine);
    }

    void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            if (auto duration = runDuration()) {
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
    std::deque<std::coroutine_handle<>> mQueue;
    RbTree<TimerNode> mTimers;
};

inline void GenericIOContext::TimerNode::Awaiter::await_suspend(std::coroutine_handle<GenericIOContext::TimerNode> coroutine) {
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

inline Task<Expected<>, GenericIOContext::TimerNode> co_sleep(std::chrono::steady_clock::time_point expires) {
    co_return co_await GenericIOContext::TimerNode::Awaiter(expires);
}

inline Task<Expected<>, GenericIOContext::TimerNode> co_sleep(std::chrono::steady_clock::time_point expires, CancelToken cancel) {
    co_return co_await cancel.invoke<GenericIOContext::TimerNode::Canceller>(co_sleep(expires));
}

inline Task<Expected<>, GenericIOContext::TimerNode> co_sleep(std::chrono::steady_clock::duration timeout) {
    return co_sleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<Expected<>, GenericIOContext::TimerNode> co_sleep(std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    return co_sleep(std::chrono::steady_clock::now() + timeout, cancel);
}

} // namespace co_async

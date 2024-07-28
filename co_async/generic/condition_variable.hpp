#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/timeout.hpp>
#include <co_async/generic/when_any.hpp>
#include <co_async/platform/futex.hpp>
#include <co_async/utils/ilist.hpp>

namespace co_async {
// struct TimedConditionVariable {
// private:
//     struct PromiseNode : CustomPromise<void, PromiseNode>,
//                          IntrusiveList<PromiseNode>::NodeType {
//         void doCancel() {
//             mCancelled = true;
//             this->erase_from_parent();
//             co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
//         }
//
//         bool mCancelled = false;
//     };
//
//     IntrusiveList<PromiseNode> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<PromiseNode> coroutine)
//         const {
//             mThat->pushWaiting(coroutine.promise());
//         }
//
//         void await_resume() const noexcept {}
//
//         TimedConditionVariable *mThat;
//     };
//
//     PromiseNode *popWaiting() {
//         return mWaitingList.pop_front();
//     }
//
//     void pushWaiting(PromiseNode &promise) {
//         mWaitingList.push_back(promise);
//     }
//
//     /* struct Canceller { */
//     /*     using OpType = Task<void, PromiseNode>; */
//     /*  */
//     /*     static Task<> doCancel(OpType *op) { */
//     /*         op->get().promise().doCancel(); */
//     /*         co_return; */
//     /*     } */
//     /*  */
//     /*     static void earlyCancelValue(OpType *op) noexcept {} */
//     /* }; */
//
// public:
//     Task<void, PromiseNode> waitNotCancellable() {
//         co_await Awaiter(this);
//     }
//
//     Task<Expected<>> wait() {
//         auto waiter = waitNotCancellable();
//         CancelCallback _(co_await co_cancel, [&] {
//             waiter.promise().doCancel();
//         });
//         co_await waiter;
//         if (waiter.promise().mCancelled) {
//             co_return std::errc::operation_canceled;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
//         auto res =
//             co_await when_any(wait(), co_sleep(expires));
//         if (auto r = std::get_if<0>(&res)) {
//             co_return std::move(*r);
//         }
//         co_return std::errc::stream_timeout;
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
//         return wait(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>> wait_predicate(std::invocable<> auto &&pred) {
//         while (!std::invoke(pred)) {
//             co_await co_await wait();
//         }
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::time_point expires) {
//         while (!std::invoke(pred)) {
//             if (co_await co_cancel) {
//                 co_return std::errc::operation_canceled;
//             }
//             if (std::chrono::steady_clock::now() > expires) {
//                 co_return std::errc::stream_timeout;
//             }
//             co_await co_await wait();
//         }
//         co_return {};
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::duration timeout) {
//         return wait_predicate(std::forward<decltype(pred)>(pred),
//                               std::chrono::steady_clock::now() + timeout);
//     }
//
//     void notify() {
//         while (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     void notify_one() {
//         if (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (auto promise = popWaiting()) {
//             return
//             std::coroutine_handle<PromiseNode>::from_promise(*promise);
//         }
//         return nullptr;
//     }
// };
//
// struct ConditionVariable {
// private:
//     std::deque<std::coroutine_handle<>> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             mThat->mWaitingList.push_back(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         ConditionVariable *mThat;
//     };
//
// public:
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         while (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             co_spawn(coroutine);
//         }
//     }
//
//     void notify_one() {
//         if (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             co_spawn(coroutine);
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             return coroutine;
//         }
//         return nullptr;
//     }
// };
//
// struct OneshotConditionVariable {
// private:
//     std::coroutine_handle<> mWaitingCoroutine{nullptr};
//     bool mReady{false};
//
// public:
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return mThat->mReady;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
// #if CO_ASYNC_DEBUG
//             if (mThat->mWaitingCoroutine) [[unlikely]] {
//                 throw std::logic_error(
//                     "please do not co_await on the same "
//                     "OneshotConditionVariable or Future for multiple times");
//             }
// #endif
//             mThat->mWaitingCoroutine = coroutine;
//         }
//
//         void await_resume() const noexcept {
// #if CO_ASYNC_DEBUG
//             if (!mThat->mReady) [[unlikely]] {
//                 throw std::logic_error("OneshotConditionVariable or Future "
//                                        "waked up but not ready");
//             }
// #endif
//             mThat->mReady = false;
//         }
//
//         OneshotConditionVariable *mThat;
//     };
//
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         mReady = true;
//         if (auto coroutine = mWaitingCoroutine) {
//             mWaitingCoroutine = nullptr;
//             co_spawn(coroutine);
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         mReady = true;
//         if (auto coroutine = mWaitingCoroutine) {
//             mWaitingCoroutine = nullptr;
//             return coroutine;
//         }
//         return nullptr;
//     }
// };
//
// struct ConcurrentConditionVariable {
// private:
//     struct WaitEntry {
//         std::coroutine_handle<> coroutine;
//         GenericIOContext *context;
//     };
//
//     std::deque<WaitEntry> mWaitingList;
//     std::mutex mMutex;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             std::lock_guard lock(mThat->mMutex);
//             mThat->mWaitingList.emplace_back(coroutine,
//                                              GenericIOContext::instance);
//         }
//
//         void await_resume() const noexcept {}
//
//         ConcurrentConditionVariable *mThat;
//     };
//
// public:
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         while (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             waitEntry.context->enqueueJobMT(waitEntry.coroutine);
//         }
//     }
//
//     void notify_one() {
//         if (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             waitEntry.context->enqueueJobMT(waitEntry.coroutine);
//         }
//     }
//
//     std::optional<WaitEntry> notifyPopCoroutine() {
//         std::lock_guard lock(mMutex);
//         if (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             return waitEntry;
//         }
//         return std::nullopt;
//     }
// };
//
// struct ConcurrentTimedConditionVariable {
// private:
//     struct PromiseNode : CustomPromise<void, PromiseNode>,
//                          ConcurrentIntrusiveList<PromiseNode>::NodeType {
//         void doCancel() {
//             mCancelled = true;
//             this->erase_from_parent();
//             co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
//         }
//
//         bool mCancelled = false;
//     };
//
//     ConcurrentIntrusiveList<PromiseNode> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<PromiseNode> coroutine)
//         const {
//             mThat->pushWaiting(coroutine.promise());
//         }
//
//         void await_resume() const noexcept {}
//
//         ConcurrentTimedConditionVariable *mThat;
//     };
//
//     PromiseNode *popWaiting() {
//         return mWaitingList.pop_front();
//     }
//
//     void pushWaiting(PromiseNode &promise) {
//         return mWaitingList.push_back(promise);
//     }
//
//     /* struct Canceller { */
//     /*     using OpType = Task<void, PromiseNode>; */
//     /*  */
//     /*     static Task<> doCancel(OpType *op) { */
//     /*         op->get().promise().doCancel(); */
//     /*         co_return; */
//     /*     } */
//     /*  */
//     /*     static void earlyCancelValue(OpType *op) noexcept {} */
//     /* }; */
//
// public:
//     Task<void, PromiseNode> waitNotCancellable() {
//         co_await Awaiter(this);
//     }
//
//     Task<Expected<>> wait() {
//         auto waiter = waitNotCancellable();
//         CancelCallback _(co_await co_cancel, [&] {
//             waiter.promise().doCancel();
//         });
//         co_await waiter;
//         if (waiter.promise().mCancelled) {
//             co_return std::errc::operation_canceled;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
//         auto res =
//             co_await when_any(wait(), co_sleep(expires));
//         if (res.index() != 0) {
//             co_return std::errc::stream_timeout;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
//         return wait(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::time_point expires) {
//         while (!std::invoke(pred)) {
//             if (co_await co_cancel) {
//                 co_return std::errc::operation_canceled;
//             }
//             if (std::chrono::steady_clock::now() > expires) {
//                 co_return std::errc::stream_timeout;
//             }
//             co_await co_await wait();
//         }
//         co_return {};
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::duration timeout) {
//         return wait_predicate(std::forward<decltype(pred)>(pred),
//                               std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>> wait_predicate(std::invocable<> auto &&pred) {
//         while (!std::invoke(pred)) {
//             co_await co_await wait();
//         }
//     }
//
//     void notify() {
//         while (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     void notify_one() {
//         if (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (auto promise = popWaiting()) {
//             return
//             std::coroutine_handle<PromiseNode>::from_promise(*promise);
//         }
//         return nullptr;
//     }
// };

struct ConditionVariable {
private:
    FutexAtomic<std::uint32_t> mFutex{};

public:
    Task<Expected<>> wait() {
        std::uint32_t old = mFutex.load(std::memory_order_relaxed);
        do {
            co_await co_await futex_wait(&mFutex, old);
        } while (mFutex.load(std::memory_order_acquire) == old);
        co_return {};
    }

    void notify_one() {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, 1);
    }

    void notify_all() {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, kFutexNotifyAll);
    }

    using Mask = std::uint32_t;

    Task<Expected<>> wait(Mask mask) {
        std::uint32_t old = mFutex.load(std::memory_order_relaxed);
        do {
            co_await co_await futex_wait(&mFutex, old, mask);
        } while (mFutex.load(std::memory_order_acquire) == old);
        co_return {};
    }

    void notify_one(Mask mask) {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, 1, mask);
    }

    void notify_all(Mask mask) {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, kFutexNotifyAll, mask);
    }
};

} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/timeout.hpp>
#include <co_async/utils/rbtree.hpp>

namespace co_async {
struct TimedConditionVariable {
private:
    struct PromiseNode : CustomPromise<void, PromiseNode>,
                         RbTree<PromiseNode>::NodeType {
        bool operator<(PromiseNode const &that) const {
            if (!mExpires) {
                return false;
            }
            if (!that.mExpires) {
                return true;
            }
            return *mExpires < *that.mExpires;
        }

        void doCancel() {
            this->destructiveErase();
            co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
        }

        bool isCanceled() const noexcept {
            return this->rbTree == nullptr;
        }

        std::optional<std::chrono::steady_clock::time_point> mExpires;
    };

    RbTree<PromiseNode> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<PromiseNode> coroutine) const {
            mThat->pushWaiting(coroutine.promise());
        }

        void await_resume() const noexcept {}

        TimedConditionVariable *mThat;
    };

    PromiseNode *popWaiting() {
        if (mWaitingList.empty()) {
            return nullptr;
        }
        auto &promise = mWaitingList.front();
        mWaitingList.erase(promise);
        return &promise;
    }

    void pushWaiting(PromiseNode &promise) {
        mWaitingList.insert(promise);
    }

    struct Canceller {
        using OpType = Task<void, PromiseNode>;

        static Task<> doCancel(OpType *op) {
            op->get().promise().doCancel();
            co_return;
        }

        static void earlyCancelValue(OpType *op) noexcept {}
    };

    Task<> waitCancellable(std::chrono::steady_clock::time_point expires,
                           CancelToken cancel) {
        auto waiter = wait();
        waiter.get().promise().mExpires = expires;
        co_await cancel.guard<Canceller>(waiter);
    }

public:
    Task<void, PromiseNode> wait() {
        co_await Awaiter(this);
    }

    Task<Expected<>> wait(CancelToken cancel) {
        auto waiter = wait();
        co_await cancel.guard<Canceller>(waiter);
        if (waiter.get().promise().isCanceled()) {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        co_return {};
    }

    Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
        auto res = co_await co_timeout(&TimedConditionVariable::waitCancellable,
                                       expires, this, expires);
        if (!res) {
            co_return Unexpected{
                std::make_error_code(std::errc::stream_timeout)};
        }
        co_return {};
    }

    Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
        return wait(std::chrono::steady_clock::now() + timeout);
    }

    Task<> wait_until(std::invocable<> auto &&pred) {
        while (!std::invoke(pred)) {
            co_await wait();
        }
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred, std::chrono::steady_clock::time_point expires) {
        while (!std::invoke(pred)) {
            if (std::chrono::steady_clock::now() > expires || !co_await wait(expires)) {
                co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
            }
        }
        co_return {};
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred, std::chrono::steady_clock::duration timeout) {
        return wait_until(std::forward<decltype(pred)>(pred), std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred, CancelToken cancel) {
        while (!std::invoke(pred)) {
            if (cancel.is_canceled() || !co_await wait(cancel)) {
                co_return Unexpected{std::make_error_code(std::errc::operation_canceled)};
            }
        }
    }

    void notify() {
        while (auto promise = popWaiting()) {
            co_spawn(
                std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }

    void notify_one() {
        if (auto promise = popWaiting()) {
            co_spawn(
                std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        if (auto promise = popWaiting()) {
            return std::coroutine_handle<PromiseNode>::from_promise(*promise);
        }
        return nullptr;
    }
};

struct ConditionVariable {
private:
    std::deque<std::coroutine_handle<>> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mThat->mWaitingList.push_back(coroutine);
        }

        void await_resume() const noexcept {}

        ConditionVariable *mThat;
    };

public:
    Awaiter operator co_await() noexcept {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        while (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            co_spawn(coroutine);
        }
    }

    void notify_one() {
        if (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            co_spawn(coroutine);
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        if (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            return coroutine;
        }
        return nullptr;
    }
};

struct OneshotConditionVariable {
private:
    std::coroutine_handle<> mWaitingCoroutine{nullptr};
    bool mReady{false};

public:
    struct Awaiter {
        bool await_ready() const noexcept {
            return mThat->mReady;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
#if CO_ASYNC_DEBUG
            if (mThat->mWaitingCoroutine) [[unlikely]] {
                throw std::logic_error(
                    "please do not co_await on the same "
                    "OneshotConditionVariable or Future for multiple times");
            }
#endif
            mThat->mWaitingCoroutine = coroutine;
        }

        void await_resume() const noexcept {
#if CO_ASYNC_DEBUG
            if (!mThat->mReady) [[unlikely]] {
                throw std::logic_error("OneshotConditionVariable or Future "
                                       "waked up but not ready");
            }
#endif
            mThat->mReady = false;
        }

        OneshotConditionVariable *mThat;
    };

    Awaiter operator co_await() noexcept {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        mReady = true;
        if (auto coroutine = mWaitingCoroutine) {
            mWaitingCoroutine = nullptr;
            co_spawn(coroutine);
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        mReady = true;
        if (auto coroutine = mWaitingCoroutine) {
            mWaitingCoroutine = nullptr;
            return coroutine;
        }
        return nullptr;
    }
};
} // namespace co_async

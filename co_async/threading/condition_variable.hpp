#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/system/timer.hpp>
#include <co_async/threading/future.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/utils/concurrent_queue.hpp>

namespace co_async {

struct ConditionVariable {
private:
    struct PromiseNode : CustomPromise<void, PromiseNode>, ConcurrentRbTree<PromiseNode>::NodeType {
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
            /* std::coroutine_handle<PromiseNode>::from_promise(*this).resume(); */
        }

        std::optional<std::chrono::steady_clock::time_point> mExpires;
    };

    ConcurrentRbTree<PromiseNode> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<PromiseNode> coroutine) const {
            mThat->pushWaiting(coroutine.promise());
        }

        void await_resume() const noexcept {
        }

        ConditionVariable *mThat;
    };

    PromiseNode *popWaiting() {
        auto locked = mWaitingList.lock();
        if (locked->empty()) {
            return nullptr;
        }
        auto &promise = locked->front();
        locked->erase(promise);
        return &promise;
    }

    void pushWaiting(PromiseNode &promise) {
        mWaitingList.lock()->insert(promise);
    }

    struct Canceller {
        using OpType = Task<void, PromiseNode>;

        static Task<> doCancel(OpType *op) {
            op->get().promise().doCancel();
            co_return;
        }

        static void earlyCancelValue() noexcept {
        }
    };

    Task<> waitCancellable(std::chrono::steady_clock::time_point expires, CancelToken cancel) {
        auto waiter = wait();
        waiter.get().promise().mExpires = expires;
        co_await cancel.invoke<Canceller>(waiter);
    }

public:
    Task<void, PromiseNode> wait() {
        co_await Awaiter(this);
    }

    Task<Expected<>> wait_until(std::chrono::steady_clock::time_point expires) {
        auto res = co_await timeout_until(&ConditionVariable::waitCancellable, expires, this, expires);
        if (!res) {
            co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
        }
        co_return {};
    }

    Task<Expected<>> wait_for(std::chrono::nanoseconds timeout) {
        auto res = co_await timeout_for(&ConditionVariable::waitCancellable, timeout, this, std::chrono::steady_clock::now() + timeout);
        if (!res) {
            co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
        }
        co_return {};
    }

    void notify() {
        while (auto promise = popWaiting()) {
            co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }

    void notify_one() {
        if (auto promise = popWaiting()) {
            co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }
};

struct ConditionList {
private:
    ConcurrentQueue<std::coroutine_handle<>, (1 << 4) - 1> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> coroutine) const {
            if (!mThat->mWaitingList.push(coroutine)) [[unlikely]] return false;
            return true;
        }

        void await_resume() const noexcept {}

        ConditionList *mThat;
    };

public:
    Awaiter operator co_await() {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        while (auto coroutine = mWaitingList.pop()) {
            co_spawn(*coroutine);
        }
    }

    void notify_one() {
        if (auto coroutine = mWaitingList.pop()) {
            co_spawn(*coroutine);
        }
    }
};

struct ConditionOnce {
private:
    std::atomic<void *> mWaitingCoroutine{nullptr};

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mThat->mWaitingCoroutine.store(coroutine.address(),
                                           std::memory_order_release);
        }

        void await_resume() const noexcept {}

        ConditionOnce *mThat;
    };

public:
    Awaiter operator co_await() {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        auto coroPtr =
            mWaitingCoroutine.exchange(nullptr, std::memory_order_acq_rel);
        if (coroPtr) {
            co_spawn(std::coroutine_handle<>::from_address(coroPtr));
        }
    }
};

} // namespace co_async

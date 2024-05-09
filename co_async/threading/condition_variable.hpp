#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/system/timer.hpp>
#include <co_async/awaiter/when_any.hpp>
#include <co_async/threading/future.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/threading/concurrent_queue.hpp>

namespace co_async {

struct ConditionTimed {
private:
    struct ConditionTimedPromise : Promise<void>, ConcurrentRbTree<ConditionTimedPromise>::NodeType {
        auto get_return_object() {
            return std::coroutine_handle<ConditionTimedPromise>::from_promise(*this);
        }

        bool operator<(ConditionTimedPromise const &that) const {
            return this < &that;
        }
    };

    ConcurrentRbTree<ConditionTimedPromise> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<ConditionTimedPromise> coroutine) const {
            mThat->pushWaiting(coroutine.promise());
            return true;
        }

        void await_resume() const noexcept {}

        ConditionTimed *mThat;
    };

    ConditionTimedPromise *popWaiting() {
        auto locked = mWaitingList.lock();
        if (locked->empty()) {
            return nullptr;
        }
        auto &promise = locked->front();
        locked->erase(promise);
        return &promise;
    }

    void pushWaiting(ConditionTimedPromise &promise) {
        mWaitingList.lock()->insert(promise);
    }

public:
    Task<void, ConditionTimedPromise> wait() {
        co_await Awaiter(this);
    }

    Task<Expected<>> wait_for(std::chrono::nanoseconds timeout) {
        auto res = co_await when_any(wait(), sleep_for(timeout));
        if (res.index() != 0) {
            co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
        }
        co_return {};
    }

    void notify() {
        while (auto promise = popWaiting()) {
            std::coroutine_handle<ConditionTimedPromise>::from_promise(*promise).resume();
        }
    }

    void notify_one() {
        if (auto promise = popWaiting()) {
            std::coroutine_handle<ConditionTimedPromise>::from_promise(*promise).resume();
        }
    }
};

struct ConditionVariable {
private:
    ConcurrentQueue<std::coroutine_handle<>> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> coroutine) const {
            if (!mThat->mWaitingList.push(coroutine)) [[unlikely]] return false;
            return true;
        }

        void await_resume() const noexcept {}

        ConditionVariable *mThat;
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
            coroutine->resume();
        }
    }

    void notify_one() {
        if (auto coroutine = mWaitingList.pop()) {
            coroutine->resume();
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
            auto coroutine = std::coroutine_handle<>::from_address(coroPtr);
            coroutine.resume();
        }
    }
};

} // namespace co_async

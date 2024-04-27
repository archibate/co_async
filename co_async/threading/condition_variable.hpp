#pragma once

#include <co_async/std.hpp>
#include <co_async/threading/concurrent_queue.hpp>

namespace co_async {

struct MultishotConditionVariable {
    ConcurrentQueue<std::coroutine_handle<>> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mThat->mWaitingList.push(coroutine);
        }

        void await_resume() const noexcept {}

        MultishotConditionVariable *mThat;
    };

    Awaiter operator co_await() {
        return Awaiter(this);
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

struct ConditionVariable {
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

        ConditionVariable *mThat;
    };

    Awaiter operator co_await() {
        return Awaiter(this);
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

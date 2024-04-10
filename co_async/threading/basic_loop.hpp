#pragma once /*{export module co_async:threading.basic_loop;}*/

#include "co_async/awaiter/just.hpp"
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/awaiter/concepts.hpp>         /*{import :awaiter.concepts;}*/
#include <co_async/awaiter/task.hpp>             /*{import :awaiter.task;}*/
#include <co_async/awaiter/unique_task.hpp> /*{import :awaiter.unique_task;}*/
#include <co_async/awaiter/details/ignore_return_promise.hpp> /*{import :awaiter.details.ignore_return_promise;}*/
#include <co_async/utils/uninitialized.hpp>  /*{import :utils.uninitialized;}*/
#include <co_async/utils/non_void_helper.hpp>/*{import :utils.non_void_helper;}*/

namespace co_async {

struct BasicLoop {
    void run() {
        while (!mQueue.empty()) {
            auto coroutine = mQueue.front();
            mQueue.pop_front();
            coroutine.resume();
        }
    }

    void enqueue(std::coroutine_handle<> coroutine) {
        mQueue.push_back(coroutine);
    }

    BasicLoop() = default;
    BasicLoop(BasicLoop &&) = delete;

private:
    std::deque<std::coroutine_handle<>> mQueue;
};

struct FutureTokenBase {
    void setOwningCoroutine(std::coroutine_handle<> coroutine) noexcept {
        mCoroutineOwning = coroutine;
    }

    std::coroutine_handle<>
    setWaitingCoroutine(std::coroutine_handle<> coroutine) {
        void *expect{nullptr};
        if (mCoroutineWaiting.compare_exchange_strong(
                expect, coroutine.address(), std::memory_order_acq_rel)) {
            return std::noop_coroutine();
        } else {
#if CO_ASYNC_DEBUG
            if (expect == (void *)-1) [[unlikely]] {
                throw std::logic_error("future cannot be waited twice");
            }
#endif
            return coroutine;
        }
    }

    std::coroutine_handle<> getWaitingCoroutine() {
        auto p =
            mCoroutineWaiting.exchange((void *)-1, std::memory_order_acq_rel);
#if CO_ASYNC_DEBUG
        if (p == (void *)-1) [[unlikely]] {
            throw std::logic_error("future value cannot be set twice");
        }
        mCompleted = true;
#endif
        return p ? std::coroutine_handle<>::from_address(p) : nullptr;
    }

    FutureTokenBase &operator=(FutureTokenBase &&) = delete;

    ~FutureTokenBase() {
        if (mCoroutineOwning) [[likely]] {
#if CO_ASYNC_DEBUG
            if (!mCompleted) [[unlikely]] {
                std::cerr << "WARNING: future destroyed unvalued\n";
            }
            if (!mCoroutineOwning.done()) [[unlikely]] {
                std::cerr << "WARNING: future destroyed undone\n";
            }
#endif
            mCoroutineOwning.destroy();
        }
    }

private:
    std::coroutine_handle<> mCoroutineOwning{nullptr};
    std::atomic<void *> mCoroutineWaiting{nullptr};
#if CO_ASYNC_DEBUG
    bool mCompleted{false};
#endif
};

template <class T>
/*[export]*/ struct FutureToken : FutureTokenBase {
    template <class U>
    void set_value(U &&value) {
        mData.putValue(std::forward<U>(value));
        if (auto coroutine = getWaitingCoroutine()) {
            mLoop.enqueue(coroutine);
        }
    }

#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        mException = e;
        if (auto coroutine = getWaitingCoroutine()) {
            mLoop.enqueue(coroutine);
        }
    }
#endif

    T fetchValue() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        if constexpr (!std::is_void_v<T>) {
            return mData.moveValue();
        }
    }

    explicit FutureToken(BasicLoop &loop) : mLoop(loop) {}

private:
    BasicLoop &mLoop;
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
    Uninitialized<T> mData;
};

template <class T>
struct FutureAwaiter {
    explicit FutureAwaiter(FutureToken<T> *token) : mToken(token) {}

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const {
        return mToken->setWaitingCoroutine(coroutine);
    }

    T await_resume() const {
        return mToken->fetchValue();
    }

private:
    FutureToken<T> *mToken;
};

template <class T>
/*[export]*/ struct [[nodiscard]] Future {
    explicit Future(BasicLoop &loop)
        : mToken(std::make_unique<FutureToken<T>>(loop)) {}

    FutureToken<T> *get_token() const noexcept {
        return mToken.get();
    }

    auto operator co_await() const {
        return FutureAwaiter<T>(mToken.get());
    }

    Task<T> wait() const {
        co_return co_await *this;
    }

private:
    std::unique_ptr<FutureToken<T>> mToken;
};

template <class T, class P>
inline UniqueTask<void, IgnoreReturnPromise<AutoDestroyingFinalAwaiter>>
loopEnqueueDetachStarter(BasicLoop &loop, UniqueTask<T, P> task) {
    co_await task;
}

template <class T, class P>
inline void loop_enqueue_detach(BasicLoop &loop, UniqueTask<T, P> task) {
    auto wrapped = loopEnqueueDetachStarter(loop, std::move(task));
    auto coroutine = wrapped.get();
    loop.enqueue(coroutine);
    wrapped.release();
}

template <class T, class P>
inline UniqueTask<void, IgnoreReturnPromise<>>
loopEnqueueFutureStarter(BasicLoop &loop, UniqueTask<T, P> task,
                         FutureToken<T> *token) {
    try {
        token->set_value((co_await task, NonVoidHelper<>()));
    } catch (...) {
        token->set_exception(std::current_exception());
    }
}

template <class T, class P>
inline Future<T> loop_enqueue_future(BasicLoop &loop, UniqueTask<T, P> task) {
    Future<T> future(loop);
    auto *token = future.get_token();
    auto wrapped = loopEnqueueFutureStarter(loop, std::move(task), token);
    auto coroutine = wrapped.get();
    token->setOwningCoroutine(coroutine);
    wrapped.release();
    loop.enqueue(coroutine);
    return future;
}

template <class Loop, class T, class P>
T loop_enqueue_and_wait(Loop &loop, Task<T, P> const &task) {
    auto awaiter = task.operator co_await();
    auto coroutine = awaiter.await_suspend(std::noop_coroutine());
    coroutine.resume();
    while (!coroutine.done()) {
        loop.run();
    }
    return awaiter.await_resume();
}

} // namespace co_async

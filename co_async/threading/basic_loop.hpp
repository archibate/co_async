#pragma once/*{export module co_async:threading.basic_loop;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/awaiter/concepts.hpp>/*{import :awaiter.concepts;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/awaiter/details/ignore_return_promise.hpp>/*{import :awaiter.details.ignore_return_promise;}*/
#include <co_async/utils/uninitialized.hpp>/*{import :utils.uninitialized;}*/
#include <co_async/utils/non_void_helper.hpp>/*{import :utils.non_void_helper;}*/
#include <co_async/threading/concurrent_queue.hpp>/*{import :threading.concurrent_queue;}*/

namespace co_async {

struct BasicLoop {
    bool run() {
        if (auto coroutine = mQueue.pop()) {
            coroutine->resume();
            return true;
        }
        return false;
    }

    void enqueue(std::coroutine_handle<> coroutine) {
        if (!mQueue.push(coroutine)) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: coroutine queue overrun\n";
#endif
            while (true) {
                if (auto coroutine = mQueue.pop()) [[likely]] {
                    coroutine->resume();
                }
                if (mQueue.push(coroutine)) [[likely]] {
                    break;
                }
            }
        }
    }

    BasicLoop() = default;
    BasicLoop(BasicLoop &&) = delete;

    static inline thread_local BasicLoop *tlsInstance;

private:
    ConcurrentQueue<std::coroutine_handle<>> mQueue;
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
            return coroutine;
        }
    }

    std::coroutine_handle<> getWaitingCoroutine() {
        auto p =
            mCoroutineWaiting.exchange((void *)-1, std::memory_order_acq_rel);
#if CO_ASYNC_DEBUG
        if (p == (void *)-1) [[unlikely]] {
            throw std::logic_error("future value cannot be set twice (get)");
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

template <class T = void>
/*[export]*/ struct FutureToken : FutureTokenBase {
    template <class U>
    void set_value(U &&value) {
        mData.putValue(std::forward<U>(value));
        if (auto coroutine = getWaitingCoroutine()) {
            BasicLoop::tlsInstance->enqueue(coroutine);
        }
    }

#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        mException = e;
        if (auto coroutine = getWaitingCoroutine()) {
            BasicLoop::tlsInstance->enqueue(coroutine);
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

    FutureToken &operator=(FutureToken &&) = delete;

private:
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

template <class T = void>
/*[export]*/ struct [[nodiscard]] Future { // TODO: crash!
    explicit Future() : mToken(std::make_unique<FutureToken<T>>()) {}

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
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
loopEnqueueDetachStarter(Task<T, P> task) {
    co_await task;
}

template <class T, class P>
inline void loop_enqueue_detach(BasicLoop &loop, Task<T, P> task) {
    auto wrapped = loopEnqueueDetachStarter(std::move(task));
    auto coroutine = wrapped.get();
    loop.enqueue(coroutine);
    wrapped.release();
}

template <class T, class P>
inline Task<void, IgnoreReturnPromise<>>
loopEnqueueFutureStarter(Task<T, P> task, FutureToken<T> *token) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        token->set_value((co_await task, NonVoidHelper<>()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        token->set_exception(std::current_exception());
    }
#endif
}

template <class T, class P>
inline Future<T> loop_enqueue_future(BasicLoop &loop, Task<T, P> task) {
    Future<T> future;
    auto *token = future.get_token();
    auto wrapped = loopEnqueueFutureStarter(std::move(task), token);
    auto coroutine = wrapped.get();
    token->setOwningCoroutine(coroutine);
    wrapped.release();
    loop.enqueue(coroutine);
    return future;
}

template <class T>
inline Task<void, IgnoreReturnPromise<>>
loopEnqueueFutureNotifier(std::condition_variable &cv, Future<T> &future,
                          Uninitialized<T> &result
#if CO_ASYNC_EXCEPT
                          ,
                          std::exception_ptr &exception
#endif
) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.putValue((co_await future, NonVoidHelper<>()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        exception = std::current_exception();
    }
#endif
    cv.notify_one();
}

template <class T, class P>
inline T loop_enqueue_synchronized(BasicLoop &loop, Task<T, P> task) {
    auto future = loop_enqueue_future(loop, std::move(task));
    std::condition_variable cv;
    std::mutex mtx;
    Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
    std::exception_ptr exception;
#endif
    auto notifier = loopEnqueueFutureNotifier(cv, future, result
#if CO_ASYNC_EXCEPT
                                              ,
                                              exception
#endif
    );
    loop.enqueue(notifier.get());
    std::unique_lock lck(mtx);
    cv.wait(lck);
    lck.unlock();
#if CO_ASYNC_EXCEPT
    if (exception) [[unlikely]] {
        std::rethrow_exception(exception);
    }
#endif
    if constexpr (!std::is_void_v<T>) {
        return result.moveValue();
    }
}

struct FutureGroup {
    std::vector<Future<>> mFutures;

    FutureGroup &add(Future<> f) {
        mFutures.push_back(std::move(f));
        return *this;
    }

    Task<> wait() {
        for (auto &f: mFutures) {
            co_await f;
        }
        mFutures.clear();
    }
};

} // namespace co_async

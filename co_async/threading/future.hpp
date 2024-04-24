#pragma once /*{export module co_async:threading.future;}*/

#include <co_async/std.hpp>               /*{import std;}*/
#include <co_async/awaiter/task.hpp>      /*{import :awaiter.task;}*/
#include <co_async/system/system_loop.hpp>/*{import :system.system_loop;}*/
#include <co_async/threading/concurrent_queue.hpp>/*{import :threading.concurrent_queue;}*/
#include <co_async/awaiter/details/ignore_return_promise.hpp>/*{import :awaiter.details.ignore_return_promise;}*/

namespace co_async {

template <class T>
struct FutureToken;

template <class T>
struct [[nodiscard]] FutureSource {
private:
    std::atomic<void *> mWaitingCoroutine{nullptr};
    Uninitialized<T> mValue;
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{nullptr};
#endif

public:
    FutureSource() = default;
    FutureSource(FutureSource &&) = delete;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mThat->mWaitingCoroutine.store(coroutine.address(),
                                           std::memory_order_release);
        }

        T await_resume() const noexcept {
#if CO_ASYNC_EXCEPT
            if (mThat->mException) [[unlikely]] {
                std::rethrow_exception(mThat->mException);
            }
#endif
            return mThat->mValue.moveValue();
        }

        FutureSource<T> *mThat;
    };

    Awaiter operator co_await() {
        return Awaiter(this);
    }

    inline FutureToken<T> token() noexcept;

    template <class>
    friend struct FutureToken;
};

template <class T>
struct [[nodiscard]] FutureToken {
    FutureToken(FutureSource<T> &that) noexcept : mThat(&that) {}

    void set_value(T &&value) {
        auto coroPtr = mThat->mWaitingCoroutine.exchange(
            nullptr, std::memory_order_acq_rel);
        if (coroPtr) {
            mThat->mValue.putValue(std::move(value));
            auto coroutine = std::coroutine_handle<>::from_address(coroPtr);
            coroutine.resume();
        }
    }

#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        auto coroPtr = mThat->mWaitingCoroutine.exchange(
            nullptr, std::memory_order_acq_rel);
        if (coroPtr) {
            mThat->mException = e;
            auto coroutine = std::coroutine_handle<>::from_address(coroPtr);
            coroutine.resume();
        } else {
            std::rethrow_exception(e);
        }
    }
#endif

    struct ReferenceWriter {
        ReferenceWriter(FutureToken<T> token) noexcept : mToken(token) {}

        operator T &() noexcept {
            return mValue;
        }

        ~ReferenceWriter() {
            if (auto e = std::current_exception()) [[unlikely]] {
                mToken.set_exception(e);
            } else {
                mToken.set_value(std::move(mValue));
            }
        }

    private:
        T mValue;
        FutureToken<T> mToken;
    };

    [[nodiscard]] ReferenceWriter reference_writer() const {
        return ReferenceWriter(*this);
    }

    FutureSource<T>::Awaiter operator co_await() {
        return mThat->operator co_await();
    }

private:
    FutureSource<T> *mThat;
};

template <class T>
inline FutureToken<T> FutureSource<T>::token() noexcept {
    return FutureToken<T>(this);
}

template <class T>
FutureToken(FutureSource<T> &) -> FutureToken<T>;

template <class T>
inline Task<void, IgnoreReturnPromise<>>
futureStartHelper(FutureToken<T> &future, Task<T> task) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        future.set_value((co_await task, NonVoidHelper<>()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        future.set_exception(std::current_exception());
    }
#endif
}

template <class T>
inline FutureSource<T> co_future(Task<T> task) {
    FutureSource<T> future;
    co_spawn(futureStartHelper(future.token(), std::move(task)));
    return future;
}

} // namespace co_async

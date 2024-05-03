#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/threading/concurrent_queue.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>

namespace co_async {

template <class T = void>
struct [[nodiscard]] FutureToken;

template <class T>
    requires(!std::is_reference_v<T> && !std::is_void_v<T>)
struct FutureReference;

template <class T = void>
struct [[nodiscard]] FutureSource {
private:
    struct Impl {
        std::atomic<void *> mWaitingCoroutine{nullptr};
        Uninitialized<T> mValue;
#if CO_ASYNC_EXCEPT
        std::exception_ptr mException{nullptr};
#endif
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        bool await_suspend(std::coroutine_handle<> coroutine) const {
            void *coroPtr = nullptr;
            if (!mImpl->mWaitingCoroutine.compare_exchange_strong(
                    coroPtr, coroutine.address(), std::memory_order_acq_rel)) {
                // coroPtr != nullptr
#if CO_ASYNC_DEBUG
                if (coroPtr != (void *)-1) [[unlikely]] {
                    throw std::logic_error(
                        "someone is already waiting on this future (" +
                        std::to_string((std::uintptr_t)coroPtr) + ")");
                }
#endif
                // -1 means future already done, don't suspend then
                return false;
            }
            return true;
        }

        T await_resume() const noexcept {
#if CO_ASYNC_EXCEPT
            if (mImpl->mException) [[unlikely]] {
                std::rethrow_exception(mImpl->mException);
            }
#endif
            if constexpr (!std::is_void_v<T>) {
                return mImpl->mValue.moveValue();
            }
        }

        Impl *mImpl;
    };

public:
    FutureSource() = default;
    FutureSource(FutureSource &&) = default;
    FutureSource &operator=(FutureSource &&) = default;

    Awaiter operator co_await() const noexcept {
        return Awaiter(mImpl.get());
    }

    inline FutureToken<T> token() const noexcept;
    inline FutureReference<T> reference() const noexcept;

    template <class>
    friend struct FutureToken;
};

template <class T>
struct FutureToken {
    FutureToken(FutureSource<T> const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    inline FutureReference<T> reference() const noexcept;

    template <class... Args>
    void set_value(Args &&...args) {
        auto coroutine = setComplete();
        mImpl->mValue.putValue(std::forward<Args>(args)...);
        coroutine.resume();
    }

#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        auto coroutine = setComplete();
        mImpl->mException = e;
        coroutine.resume();
    }
#endif

    auto operator co_await() const noexcept {
        return typename FutureSource<T>::Awaiter(mImpl);
    }

private:
    typename FutureSource<T>::Impl *mImpl;

    std::coroutine_handle<> setComplete() {
        void *coroPtr = nullptr;
        if (!mImpl->mWaitingCoroutine.compare_exchange_strong(
                coroPtr, (void *)-1, std::memory_order_acq_rel)) {
            // coroPtr != nullptr
#if CO_ASYNC_DEBUG
            if (coroPtr == (void *)-1) [[unlikely]] {
                throw std::logic_error("future seems already set complete");
            }
#endif
            return std::coroutine_handle<>::from_address(coroPtr);
        }
        return std::noop_coroutine();
    }
};

template <class T>
    requires(!std::is_reference_v<T> && !std::is_void_v<T>)
struct [[nodiscard]] FutureReference {
    FutureReference(FutureToken<T> token) noexcept : mToken(token) {}

    operator T &() const noexcept {
        return mValue;
    }

    FutureReference(FutureReference &&) = delete;

    ~FutureReference() {
#if CO_ASYNC_EXCEPT
        if (auto e = std::current_exception()) [[unlikely]] {
            mToken.set_exception(e);
        } else {
#endif
            mToken.set_value(std::move(mValue));
#if CO_ASYNC_EXCEPT
        }
#endif
    }

private:
    mutable T mValue;
    FutureToken<T> mToken;
};

template <class T>
inline FutureToken<T> FutureSource<T>::token() const noexcept {
    return FutureToken<T>(*this);
}

template <class T>
inline FutureReference<T> FutureToken<T>::reference() const noexcept {
    return FutureReference<T>(*this);
}

template <class T>
inline FutureReference<T> FutureSource<T>::reference() const noexcept {
    return token().reference();
}

template <class T>
FutureToken(FutureSource<T> &) -> FutureToken<T>;

template <class T>
inline Task<void, IgnoreReturnPromise<>>
futureStartHelper(FutureToken<T> future, Task<T> task) {
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

template <class F, class... Args>
    requires(Awaitable<std::invoke_result_t<F, Args...>>)
inline auto co_bind(F &&f, Args &&...args) {
    return [](auto f) mutable -> std::invoke_result_t<F, Args...> {
        co_return co_await std::move(f)();
    }(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}

} // namespace co_async

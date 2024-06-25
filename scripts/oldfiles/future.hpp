#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/platform/platform_io.hpp>

namespace co_async {
template <class T = void>
struct [[nodiscard]] FutureToken;

template <class T = void>
struct [[nodiscard]] FutureSource {
public:
    struct Awaiter;

private:
    struct Impl : ConditionVariable {
        Uninitialized<T> mValue;
#if CO_ASYNC_EXCEPT
        std::exception_ptr mException{nullptr};
#endif
        inline FutureSource::Awaiter makeAwaiter();
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

public:
    struct Awaiter : ConditionVariable::Awaiter {
        T await_resume() const noexcept {
            ConditionVariable::Awaiter::await_resume();
            auto impl = static_cast<Impl *>(mThat);
            if constexpr (!std::is_void_v<T>) {
                return impl->mValue.move();
            }
        }
    };

    FutureSource() = default;
    FutureSource(FutureSource &&) = default;
    FutureSource &operator=(FutureSource &&) = default;

    auto operator co_await() const noexcept {
        return mImpl->makeAwaiter();
    }

    inline FutureToken<T> token() const noexcept;
    template <class>
    friend struct FutureToken;
};

template <class T>
auto FutureSource<T>::Impl::makeAwaiter() -> FutureSource::Awaiter {
    return FutureSource::Awaiter(
        static_cast<ConditionVariable &>(*this).operator co_await());
}

template <class T>
struct FutureToken {
    FutureToken(FutureSource<T> const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    template <class... Args>
        requires std::constructible_from<T, Args...>
    void set_value(Args &&...args) {
        mImpl->mValue.emplace(std::forward<Args>(args)...);
        mImpl->notify();
    }
#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        mImpl->mException = e;
        mImpl->notify();
    }
#endif
    auto operator co_await() const noexcept {
        return mImpl->makeAwaiter();
    }

private:
    typename FutureSource<T>::Impl *mImpl;
};
template <class T>
FutureToken(FutureSource<T> &) -> FutureToken<T>;

template <class T>
inline FutureToken<T> FutureSource<T>::token() const noexcept {
    return FutureToken<T>(*this);
}

template <class T, class P>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
coFutureHelper(FutureToken<T> future, Task<T, P> task) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        future.set_value((co_await task, Void()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        future.set_exception(std::current_exception());
    }
#endif
}

template <class T, class P>
inline FutureSource<T> co_future(Task<T, P> task) {
    FutureSource<T> future;
    auto wrapped = coFutureHelper(future.token(), std::move(task));
    auto coroutine = wrapped.get();
    GenericIOContext::instance->enqueueJob(coroutine);
    wrapped.release();
    return future;
}
} // namespace co_async

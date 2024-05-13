#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/system/platform_io.hpp>
#include <co_async/threading/condition_variable.hpp>
#include <co_async/awaiter/details/ignore_return_promise.hpp>

namespace co_async {

template <class T = void>
struct [[nodiscard]] FutureToken;

template <class T = void>
struct [[nodiscard]] FutureSource {
private:
    struct Impl : ConditionOnce {
        Uninitialized<T> mValue;
#if CO_ASYNC_EXCEPT
        std::exception_ptr mException{nullptr};
#endif

        Awaiter makeAwaiter() {
            return Awaiter(static_cast<ConditionOnce &>(*this).operator co_await());
        }
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

public:
    struct Awaiter : ConditionOnce::Awaiter {
        T await_resume() const noexcept {
            auto impl = static_cast<Impl *>(mThat);
            if constexpr (!std::is_void_v<T>) {
                co_return impl->mValue.moveValue();
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
struct FutureToken {
    FutureToken(FutureSource<T> const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    template <class... Args>
    void set_value(Args &&...args) {
        mImpl->mValue.putValue(std::forward<Args>(args)...);
        mImpl->notify();
    }

#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        mImpl->mException = e;
        mImpl->mCondition.notify();
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

template <class T>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
coFutureHelper(FutureToken<T> future, Task<T> task) {
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

template <class T>
inline FutureSource<T> co_future(Task<T> task) {
    FutureSource<T> future;
    auto wrapped = coFutureHelper(future.token(), std::move(task));
    auto coroutine = wrapped.get();
    GenericIOContext::instance->enqueueJob(coroutine);
    wrapped.release();
    return future;
}

} // namespace co_async

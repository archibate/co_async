#pragma once
#include <co_async/std.hpp>
#include "co_async/utils/debug.hpp"
#include <co_async/awaiter/just.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/utils/rbtree.hpp>

namespace co_async {
struct CancelToken;

struct CancelSourceImpl {
    struct CancellerBase : RbTree<CancellerBase>::NodeType {
        virtual Task<> doCancel() = 0;

        CancellerBase &operator=(CancellerBase &&) = delete;

        bool operator<(CancellerBase const &that) const noexcept {
            return this < &that;
        }

        virtual ~CancellerBase() = default;
    };

    template <class AwaiterPtr, class Canceller>
    struct CancellerImpl : CancellerBase {
        AwaiterPtr mOp;

        explicit CancellerImpl(AwaiterPtr op) : mOp(op) {}

        virtual Task<> doCancel() {
            return Canceller::doCancel(mOp);
        }
    };

    RbTree<CancellerBase> mCancellers;
    bool mCanceled;

    Task<> doCancel() {
        if (mCanceled) {
            co_return;
        }
        mCanceled = true;
        std::vector<Task<>> tasks;
        if (!mCancellers.empty()) {
            mCancellers.traverseInorder([&](CancellerBase &canceller) {
                tasks.push_back(canceller.doCancel());
            });
            mCancellers.clear();
            co_await when_all(tasks);
        }
    }

    bool doIsCanceled() const noexcept {
        return mCanceled;
    }

    void doRegister(CancellerBase &canceller) {
        mCancellers.insert(canceller);
    }

    template <class Canceller, class Awaiter>
    static Task<typename AwaitableTraits<Awaiter>::RetType>
    doGuard(CancelSourceImpl *impl, Awaiter &&awaiter) {
        if (impl) {
            debug().fail(), impl;
            auto *op = std::addressof(awaiter);
            CancellerImpl<decltype(op), Canceller> canceller(op);
            impl->doRegister(canceller);
            co_return co_await awaiter;
        } else {
            co_return co_await awaiter;
        }
    }
};

struct [[nodiscard]] CancelSourceBase {
private:
    std::unique_ptr<CancelSourceImpl> mImpl = std::make_unique<CancelSourceImpl>();

    friend CancelToken;

    friend struct CancelSource;

    template <class Callback>
    friend struct CancelCallback;

public:
    Task<> cancel() const {
        return mImpl->doCancel();
    }

    inline CancelToken token() const;
    CancelSourceBase() = default;
    CancelSourceBase(CancelSourceBase &&) = default;
    CancelSourceBase &operator=(CancelSourceBase &&) = default;
};

struct CancelToken {
private:
    CancelSourceImpl *mImpl;

    explicit CancelToken(CancelSourceImpl *impl) noexcept : mImpl(impl) {}

    friend struct GetThisCancel;

public:
    CancelToken() noexcept : mImpl(nullptr) {}

    CancelToken(CancelSourceBase const &that) noexcept : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl ? mImpl->doCancel() : just_void();
    }

    [[nodiscard]] bool is_canceled() const noexcept {
        return mImpl && mImpl->doIsCanceled();
    }

    [[nodiscard]] operator bool() const noexcept {
        return is_canceled();
    }

    Expected<> expect() {
        if (mImpl->doIsCanceled()) [[unlikely]] {
            return std::errc::operation_canceled;
        }
        return {};
    }

    template <class Awaiter,
              class Canceller = typename std::decay_t<Awaiter>::Canceller>
    auto guard(Awaiter &&awaiter) const {
        return CancelSourceImpl::doGuard<Canceller>(
            mImpl, std::forward<Awaiter>(awaiter));
    }

    template <class Awaiter, class Canceller>
    auto guard(std::in_place_type_t<Canceller>, Awaiter &&awaiter) const {
        return CancelSourceImpl::doGuard<Canceller>(
            mImpl, std::forward<Awaiter>(awaiter));
    }

    /* template <class Callback> */
    /* [[nodiscard("capture me in a local variable")]] auto callback(Callback callback) { */
    /*     std::unique_ptr<CancelSource::CancellerCallback<Callback>> canceller; */
    /*     if (mImpl) { */
    /*         canceller = std::make_unique<CancelSource::CancellerCallback>(std::move(callback)); */
    /*         mImpl->doRegister(*canceller); */
    /*     } */
    /*     return canceller; */
    /* } */

    template <class T>
    Expected<> operator()(TaskPromise<T> &promise) const {
        if (is_canceled())
            return std::errc::operation_canceled;
        return {};
    }

    friend struct CancelSource;

    template <class Callback>
    friend struct CancelCallback;
};

struct CancelSource : private CancelSourceImpl::CancellerBase, public CancelSourceBase {
private:
    CancelSourceImpl *mDeriveImpl;

    virtual Task<> doCancel() {
        return mDeriveImpl->doCancel();
    }

public:
    CancelSource() noexcept : mDeriveImpl(nullptr) {}

    explicit CancelSource(CancelToken cancel) : mDeriveImpl(cancel.mImpl) {
        if (mDeriveImpl) {
            mDeriveImpl->doRegister(*this);
        }
    }
};

inline CancelToken CancelSourceBase::token() const {
    return *this;
}

template <class Callback>
struct CancelCallback : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelSourceBase cancel, Callback callback) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    virtual Task<> doCancel() {
        std::invoke(std::move(mCallback));
        co_return;
    }

    Callback mCallback;
};

template <class Callback>
    requires Awaitable<std::invoke_result_t<Callback>>
struct CancelCallback<Callback> : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelSourceBase cancel, Callback callback)
    : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    virtual Task<> doCancel() {
        co_await std::invoke(std::move(mCallback));
    }

    Callback mCallback;
};

template <class Callback>
CancelCallback(Callback) -> CancelCallback<Callback>;

struct GetThisCancel {
    template <class T>
    ValueAwaiter<CancelToken> operator()(TaskPromise<T> &promise) const {
        return ValueAwaiter<CancelToken>(CancelToken(promise.mCancelToken));
    }

    template <class T>
    static T &&bind(CancelToken cancel, T &&task) {
        task.promise().mCancelToken = cancel.mImpl;
        return std::forward<T>(task);
    }

    struct CancelThis {
        template <class T>
        Task<> operator()(TaskPromise<T> &promise) const {
            return CancelToken(promise.mCancelToken).cancel();
        }
    };

    static CancelThis cancel() {
        return {};
    }
};

inline constexpr GetThisCancel co_cancel;

} // namespace co_async

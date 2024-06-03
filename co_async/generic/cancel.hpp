#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/utils/rbtree.hpp>

namespace co_async {
struct CancelToken;

struct [[nodiscard]] CancelSource {
private:
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

    struct Impl {
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

        void doResigeter(CancellerBase &canceller) {
            mCancellers.insert(canceller);
        }
    };

    template <class Canceller, class Awaiter>
    static Task<typename AwaitableTraits<Awaiter>::RetType>
    doGuard(Impl *impl, Awaiter &&awaiter) {
        if (impl) {
            auto *op = std::addressof(awaiter);
            CancellerImpl<decltype(op), Canceller> canceller(op);
            impl->doResigeter(canceller);
            co_return co_await awaiter;
        } else {
            co_return co_await awaiter;
        }
    }

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

    friend CancelToken;

    friend struct CancelDerived;

    template <class Callback>
    friend struct CancelCallback;

public:
    Task<> cancel() const {
        return mImpl->doCancel();
    }

    inline CancelToken token() const;
    CancelSource() = default;
    CancelSource(CancelSource &&) = default;
    CancelSource &operator=(CancelSource &&) = default;
};

template <class Awaiter>
concept Cancellable = requires(Awaiter &&awaiter, CancelToken const &cancel) {
    awaiter.promise().setCancelToken(cancel);
};

struct CancelToken {
private:
    CancelSource::Impl *mImpl;

public:
    CancelToken() noexcept : mImpl(nullptr) {}

    CancelToken(CancelSource const &that) noexcept : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl->doCancel();
    }

    [[nodiscard]] bool is_canceled() const noexcept {
        return mImpl->doIsCanceled();
    }

    [[nodiscard]] operator bool() const noexcept {
        return mImpl->doIsCanceled();
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
        return CancelSource::doGuard<Canceller>(
            mImpl, std::forward<Awaiter>(awaiter));
    }

    template <Cancellable Awaiter>
    Awaiter &&guard(Awaiter &&awaiter) const {
        awaiter.promise().setCancelToken(*this);
        return std::forward<Awaiter>(awaiter);
    }

    template <class Awaiter, class Canceller>
    auto guard(std::in_place_type_t<Canceller>, Awaiter &&awaiter) const {
        return CancelSource::doGuard<Canceller>(
            mImpl, std::forward<Awaiter>(awaiter));
    }

    /* template <class Callback> */
    /* [[nodiscard("capture me in a local variable")]] auto callback(Callback callback) { */
    /*     std::unique_ptr<CancelSource::CancellerCallback<Callback>> canceller; */
    /*     if (mImpl) { */
    /*         canceller = std::make_unique<CancelSource::CancellerCallback>(std::move(callback)); */
    /*         mImpl->doResigeter(*canceller); */
    /*     } */
    /*     return canceller; */
    /* } */

    friend struct CancelDerived;

    template <class Callback>
    friend struct CancelCallback;
};

inline CancelToken CancelSource::token() const {
    return *this;
}

struct CancelDerived : private CancelSource::CancellerBase, CancelSource {
private:
    CancelSource::Impl *mDeriveImpl;

    virtual Task<> doCancel() {
        return mDeriveImpl->doCancel();
    }

public:
    explicit CancelDerived(CancelToken cancel) : mDeriveImpl(cancel.mImpl) {
        if (mDeriveImpl) {
            mDeriveImpl->doResigeter(*this);
        }
    }
};

template <class Callback>
struct CancelCallback : private CancelSource::CancellerBase {
    explicit CancelCallback(CancelSource cancel, Callback callback) {
        if (cancel.mImpl) {
            cancel.mImpl->doResigeter(*this);
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
struct CancelCallback<Callback> : private CancelSource::CancellerBase {
    explicit CancelCallback(CancelSource cancel, Callback callback)
    : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doResigeter(*this);
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

} // namespace co_async

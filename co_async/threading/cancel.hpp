#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/rbtree.hpp>
#include <co_async/threading/future_group.hpp>

namespace co_async {

struct CancelToken;

struct [[nodiscard]] CancelSource {
private:
    struct CancellerBase : ConcurrentRbTree<CancellerBase>::NodeType {
        virtual Task<> doCancel() = 0;

        CancellerBase &operator=(CancellerBase &&) = delete;

        bool operator<(CancellerBase const &that) const noexcept {
            return this < &that;
        }

        virtual ~CancellerBase() = default;
    };

    template <class Canceller>
    struct CancellerImpl : CancellerBase {
        typename Canceller::OpType *mOp;

        explicit CancellerImpl(typename Canceller::OpType *op) : mOp(op) {}

        virtual Task<> doCancel() {
            return Canceller::doCancel(mOp);
        }
    };

    struct Impl {
        ConcurrentRbTree<CancellerBase> mCancellers;
        std::atomic_bool mCanceled;

        Task<> doCancel() {
            mCanceled.store(true, std::memory_order_release);
            TaskGroup group;
            {
                auto locked = mCancellers.lock();
                locked->traverseInorder([&] (CancellerBase &canceller) {
                    group.add(canceller.doCancel());
                });
                locked->clear();
            }
            co_await group.wait();
        }

        bool doIsCanceled() {
            return mCanceled.load(std::memory_order_acquire);
        }

        template <class Canceller, class Awaiter>
        Task<typename AwaitableTraits<Awaiter>::RetType> doInvoke(Awaiter &&awaiter) {
            if (mCanceled.load(std::memory_order_acquire)) {
                co_return Canceller::earlyCancelValue();
            }
            CancellerImpl<Canceller> cancellerImpl(std::addressof(awaiter));
            mCancellers.lock()->insert(static_cast<CancellerBase &>(cancellerImpl));
            if (mCanceled.load(std::memory_order_acquire)) [[unlikely]] {
                co_return Canceller::earlyCancelValue();
            }
            co_return co_await awaiter;
        }
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

    friend CancelToken;

public:
    Task<> cancel() const {
        return mImpl->doCancel();
    }

    bool is_canceled() const {
        return mImpl->doIsCanceled();
    }

    template <class Canceller>
    auto invoke(auto &&awaiter) const {
        return mImpl->doInvoke<Canceller>(std::forward<decltype(awaiter)>(awaiter));
    }

    inline CancelToken get_token() const;

    CancelSource() = default;
    CancelSource(CancelSource &&) = default;
    CancelSource &operator=(CancelSource &&) = default;
};

struct CancelToken {
private:
    CancelSource::Impl *mImpl;

public:
    CancelToken() noexcept
        : mImpl(nullptr) {}

    CancelToken(CancelSource const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl->doCancel();
    }

    bool is_canceled() const {
        return mImpl->doIsCanceled();
    }

    template <class Canceller>
    auto invoke(auto &&awaiter) const {
        return mImpl->doInvoke<Canceller>(std::forward<decltype(awaiter)>(awaiter));
    }
};

inline CancelToken CancelSource::get_token() const {
    return *this;
}

}

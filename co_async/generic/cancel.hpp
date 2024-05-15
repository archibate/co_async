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

    template <class Canceller>
    struct CancellerImpl : CancellerBase {
        typename Canceller::OpType *mOp;

        explicit CancellerImpl(typename Canceller::OpType *op) : mOp(op) {}

        virtual Task<> doCancel() {
            return Canceller::doCancel(mOp);
        }
    };

    struct Impl {
        RbTree<CancellerBase> mCancellers;
        bool mCanceled;

        Task<> doCancel() {
            mCanceled = true;
            std::vector<Task<>> tasks;
            mCancellers.traverseInorder([&](CancellerBase &canceller) {
                tasks.push_back(canceller.doCancel());
            });
            mCancellers.clear();
            co_await when_all(tasks);
        }

        bool doIsCanceled() const noexcept {
            return mCanceled;
        }

        template <class Canceller, class Awaiter>
        Task<typename AwaitableTraits<Awaiter>::RetType>
        doGuard(Awaiter &&awaiter) {
            typename Canceller::OpType *op = std::addressof(awaiter);
            CancellerImpl<Canceller> cancellerImpl(op);
            mCancellers.insert(static_cast<CancellerBase &>(cancellerImpl));
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
        return mImpl->doGuard<Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }

    inline CancelToken token() const;
    CancelSource() = default;
    CancelSource(CancelSource &&) = default;
    CancelSource &operator=(CancelSource &&) = default;
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

    bool is_canceled() const noexcept {
        return mImpl->doIsCanceled();
    }

    Expected<> check() {
        if (mImpl->doIsCanceled()) [[unlikely]] {
            return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        return {};
    }

    template <class Canceller>
    auto guard(auto &&awaiter) const {
        return mImpl->doGuard<Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }

    auto guard(auto &&awaiter) const {
        return guard<typename std::decay_t<decltype(awaiter)>::Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }
};

inline CancelToken CancelSource::token() const {
    return *this;
}
} // namespace co_async

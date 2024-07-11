#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/just.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/utils/ilist.hpp>

namespace co_async {

struct CancelSourceImpl {
    struct CancellerBase : IntrusiveList<CancellerBase>::NodeType {
        virtual Task<> doCancel() = 0;

        CancellerBase &operator=(CancellerBase &&) = delete;

        bool operator<(CancellerBase const &that) const noexcept {
            return this < &that;
        }

        // virtual ~CancellerBase() = default;
    };

    /* template <class AwaiterPtr, class Canceller> */
    /* struct CancellerImpl : CancellerBase { */
    /*     AwaiterPtr mOp; */
    /*  */
    /*     explicit CancellerImpl(AwaiterPtr op) : mOp(op) {} */
    /*  */
    /*     virtual Task<> doCancel() { */
    /*         return Canceller::doCancel(mOp); */
    /*     } */
    /* }; */

    IntrusiveList<CancellerBase> mCancellers;
    bool mCanceled;

    Task<> doCancel() {
        if (mCanceled) {
            co_return;
        }
        mCanceled = true;
        if (!mCancellers.empty()) {
            std::vector<Task<>> tasks;
            for (auto &canceller: mCancellers) {
                tasks.push_back(canceller.doCancel());
            }
            /* for (auto &&task: tasks) { */
            /*     co_await task; */
            /* } */
            co_await when_all(tasks);
            mCancellers.clear();
        }
    }

    bool doIsCanceled() const noexcept {
        return mCanceled;
    }

    void doRegister(CancellerBase &canceller) {
        mCancellers.push_front(canceller);
    }

    /* template <class Canceller, class Awaiter> */
    /* static Task<typename AwaitableTraits<Awaiter>::RetType> */
    /* doGuard(CancelSourceImpl *impl, Awaiter &&awaiter) { */
    /*     if (impl) { */
    /*         auto *op = std::addressof(awaiter); */
    /*         CancellerImpl<decltype(op), Canceller> canceller(op); */
    /*         impl->doRegister(canceller); */
    /*         co_return co_await awaiter; */
    /*     } else { */
    /*         co_return co_await awaiter; */
    /*     } */
    /* } */
};

struct CancelToken;

struct [[nodiscard]] CancelSourceBase {
protected:
    std::unique_ptr<CancelSourceImpl> mImpl =
        std::make_unique<CancelSourceImpl>();

    friend CancelToken;

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

struct [[nodiscard(
    "did you forget to capture or co_await the cancel token?")]] CancelToken {
private:
    CancelSourceImpl *mImpl;

    explicit CancelToken(CancelSourceImpl *impl) noexcept : mImpl(impl) {}

public:
    CancelToken() noexcept : mImpl(nullptr) {}

    CancelToken(CancelSourceBase const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl ? mImpl->doCancel() : just_void();
    }

    [[nodiscard]] bool is_cancel_possible() const noexcept {
        return mImpl;
    }

    [[nodiscard]] bool is_canceled() const noexcept {
        return mImpl && mImpl->doIsCanceled();
    }

    [[nodiscard]] operator bool() const noexcept {
        return is_canceled();
    }

    Expected<> as_expect() {
        if (mImpl->doIsCanceled()) [[unlikely]] {
            return std::errc::operation_canceled;
        }
        return {};
    }

    void *address() const noexcept {
        return mImpl;
    }

    static CancelToken from_address(void *impl) noexcept {
        return CancelToken(static_cast<CancelSourceImpl *>(impl));
    }

    auto repr() const {
        return mImpl;
    }

    /* template <class Awaiter, */
    /*           class Canceller = typename std::decay_t<Awaiter>::Canceller> */
    /* auto guard(Awaiter &&awaiter) const { */
    /*     return CancelSourceImpl::doGuard<Canceller>( */
    /*         mImpl, std::forward<Awaiter>(awaiter)); */
    /* } */
    /*  */
    /* template <class Awaiter, class Canceller> */
    /* auto guard(std::in_place_type_t<Canceller>, Awaiter &&awaiter) const { */
    /*     return CancelSourceImpl::doGuard<Canceller>( */
    /*         mImpl, std::forward<Awaiter>(awaiter)); */
    /* } */

    /* template <class Callback> */
    /* [[nodiscard("capture me in a local variable")]] auto callback(Callback
     * callback) { */
    /*     std::unique_ptr<CancelSource::CancellerCallback<Callback>> canceller;
     */
    /*     if (mImpl) { */
    /*         canceller =
     * std::make_unique<CancelSource::CancellerCallback>(std::move(callback));
     */
    /*         mImpl->doRegister(*canceller); */
    /*     } */
    /*     return canceller; */
    /* } */

    template <class T>
    Expected<> operator()(TaskPromise<T> &promise) const {
        if (is_canceled()) [[unlikely]] {
            return std::errc::operation_canceled;
        }
        return {};
    }

    friend struct CancelSource;

    template <class Callback>
    friend struct CancelCallback;
};

struct CancelSource : private CancelSourceImpl::CancellerBase,
                      public CancelSourceBase {
private:
    Task<> doCancel() override {
        return cancel();
    }

public:
    CancelSource() = default;

    explicit CancelSource(CancelToken cancel) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

    auto repr() {
        return mImpl.get();
    }
};

inline CancelToken CancelSourceBase::token() const {
    return *this;
}

template <class Callback>
struct [[nodiscard]] CancelCallback : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelToken cancel, Callback callback)
        : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    Task<> doCancel() override {
        std::invoke(std::move(mCallback));
        co_return;
    }

    Callback mCallback;
};

template <class Callback>
    requires Awaitable<std::invoke_result_t<Callback>>
struct [[nodiscard]] CancelCallback<Callback>
    : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelToken cancel, Callback callback)
        : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    Task<> doCancel() override {
        co_await std::invoke(std::move(mCallback));
    }

    Callback mCallback;
};

template <class Callback>
CancelCallback(CancelToken, Callback) -> CancelCallback<Callback>;

struct GetThisCancel {
    template <class T>
    ValueAwaiter<CancelToken> operator()(TaskPromise<T> &promise) const {
        return ValueAwaiter<CancelToken>(
            CancelToken::from_address(promise.mLocals.mCancelToken));
    }

    template <class T>
    static T &&bind(CancelToken cancel, T &&task) {
        task.promise().mLocals.mCancelToken = cancel.address();
        return std::forward<T>(task);
    }

    struct DoCancelThis {
        template <class T>
        Task<> operator()(TaskPromise<T> &promise) const {
            co_return co_await CancelToken::from_address(
                promise.mLocals.mCancelToken)
                .cancel();
        }
    };

    static DoCancelThis cancel() {
        return {};
    }

    /* struct DoExpectCancel { */
    /*     template <class T> */
    /*     Expected<> operator()(TaskPromise<T> &promise) const { */
    /*         return CancelToken(promise.mLocals.mCancelToken).expect(); */
    /*     } */
    /* }; */
    /*  */
    /* static DoExpectCancel expect() { */
    /*     return {}; */
    /* } */
};

inline constexpr GetThisCancel co_cancel;

} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {
struct BasicMutex {
    ConditionVariable mReady;
    std::atomic_bool mLocked;

    bool try_lock() {
        bool expect = false;
        return mLocked.compare_exchange_strong(
            expect, true, std::memory_order_acquire, std::memory_order_relaxed);
    }

    Task<> lock() {
        while (!try_lock()) {
            co_await mReady.wait();
        }
    }

    void unlock() {
        mLocked.store(false, std::memory_order_release);
        mReady.notify_one();
    }
};

struct BasicTimedMutex {
    TimedConditionVariable mReady;
    std::atomic_bool mLocked;

    bool try_lock() {
        bool expect = false;
        return mLocked.compare_exchange_strong(
            expect, true, std::memory_order_acquire, std::memory_order_relaxed);
    }

    Task<> lock() {
        while (!try_lock()) {
            co_await mReady.wait();
        }
    }

    Task<bool> try_lock(std::chrono::steady_clock::duration timeout) {
        return try_lock(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_lock(std::chrono::steady_clock::time_point expires) {
        while (!try_lock()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mReady.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }

    void unlock() {
        mLocked.store(false, std::memory_order_release);
        mReady.notify_one();
    }
};

template <class M, class T>
struct alignas(hardware_destructive_interference_size) MutexImpl {
private:
    M mMutex;
    T mValue;

public:
    struct Locked {
    private:
        explicit Locked(MutexImpl *impl) noexcept : mImpl(impl) {}

        friend MutexImpl;

    public:
        Locked() noexcept : mImpl(nullptr) {}

        T &operator*() const {
            return mImpl->unsafe_access();
        }

        T *operator->() const {
            return std::addressof(mImpl->unsafe_access());
        }

        explicit operator bool() const noexcept {
            return mImpl != nullptr;
        }

        void unlock() {
            if (mImpl) {
                mImpl->mMutex.unlock();
                mImpl = nullptr;
            }
        }

        Locked(Locked &&that) noexcept
            : mImpl(std::exchange(that.mImpl, nullptr)) {}

        Locked &operator=(Locked &&that) noexcept {
            std::swap(mImpl, that.mImpl);
            return *this;
        }

        ~Locked() {
            unlock();
        }

    private:
        MutexImpl *mImpl;
    };

    Locked try_lock() {
        if (auto e = mMutex.try_lock()) {
            return Locked(this);
        } else {
            return Locked();
        }
    }

    Task<Locked> lock() {
        co_await mMutex.lock();
        co_return Locked(this);
    }

    Task<Locked> try_lock_for(std::chrono::steady_clock::duration timeout) {
        if (!co_await mMutex.try_lock_for(timeout)) {
            co_return Locked();
        }
        co_return Locked(this);
    }

    Task<Locked> try_lock_until(std::chrono::steady_clock::time_point expires) {
        if (!co_await mMutex.try_lock_for(expires)) {
            co_return Locked();
        }
        co_return Locked(this);
    }

    T &unsafe_access() {
        return mValue;
    }

    T const &unsafe_access() const {
        return mValue;
    }

    M &unsafe_basic_mutex() {
        return mMutex;
    }

    M const &unsafe_basic_mutex() const {
        return mMutex;
    }
};

template <class M>
struct MutexImpl<M, void> : MutexImpl<M, Void> {};

template <class T = void>
struct Mutex : MutexImpl<BasicMutex, T> {};

template <class T = void>
struct TimedMutex : MutexImpl<BasicTimedMutex, T> {};

struct CallOnce {
private:
    std::atomic_bool mCalled{false};
    Mutex<> mMutex;

public:
    struct Locked {
    private:
        explicit Locked(Mutex<>::Locked locked, CallOnce *impl) noexcept
            : mLocked(std::move(locked)),
              mImpl(impl) {}

        friend CallOnce;

    public:
        Locked() noexcept : mLocked(), mImpl(nullptr) {}

        explicit operator bool() const noexcept {
            return (bool)mLocked;
        }

        void set_ready() const {
            mImpl->mCalled.store(true, std::memory_order_relaxed);
        }

    private:
        Mutex<>::Locked mLocked;
        CallOnce *mImpl;
    };

    Task<Locked> call_once() {
        if (mCalled.load(std::memory_order_relaxed)) {
            co_return Locked();
        }
        Locked locked(co_await mMutex.lock(), this);
        if (mCalled.load(std::memory_order_relaxed)) {
            co_return Locked();
        }
        co_return std::move(locked);
    }
};
} // namespace co_async

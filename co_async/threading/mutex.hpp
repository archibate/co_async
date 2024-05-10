#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/threading/condition_variable.hpp>
#include <co_async/utils/non_void_helper.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {

struct BasicMutex {
    ConditionList mReady;
    std::mutex mMutex;

    bool try_lock() {
        return mMutex.try_lock();
    }

    Task<> lock() {
        while (!mMutex.try_lock()) {
            co_await mReady.wait();
        }
    }

    void unlock() {
        mMutex.unlock();
        mReady.notify_one();
    }
};

struct BasicTimedMutex {
    ConditionVariable mReady;
    std::mutex mMutex;

    bool try_lock() {
        return mMutex.try_lock();
    }

    Task<> lock() {
        while (!mMutex.try_lock()) {
            co_await mReady.wait();
        }
    }

    Task<bool> try_lock_for(std::chrono::nanoseconds timeout) {
        auto expires = std::chrono::steady_clock::now() + timeout;
        while (!mMutex.try_lock()) {
            if (std::chrono::steady_clock::now() > expires || !co_await mReady.wait_until(expires)) [[unlikely]] {
                co_return false;
            }
        }
        co_return true;
    }

    Task<bool> try_lock_until(std::chrono::steady_clock::time_point expires) {
        while (!mMutex.try_lock()) {
            if (std::chrono::steady_clock::now() > expires || !co_await mReady.wait_until(expires)) [[unlikely]] {
                co_return false;
            }
        }
        co_return true;
    }

    void unlock() {
        mMutex.unlock();
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

        Locked(Locked &&that) noexcept : mImpl(std::exchange(that.mImpl, nullptr)) {}

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

    Task<Locked> try_lock_for(std::chrono::nanoseconds timeout) {
        if (!co_await mMutex.try_lock_for(timeout)) co_return Locked();
        co_return Locked(this);
    }

    Task<Locked> try_lock_until(std::chrono::steady_clock::time_point expires) {
        if (!co_await mMutex.try_lock_for(expires)) co_return Locked();
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
struct MutexImpl<M, void> : MutexImpl<M, Void> {
};

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
        explicit Locked(Mutex<>::Locked locked, CallOnce *impl) noexcept : mLocked(std::move(locked)), mImpl(impl) {}

        friend CallOnce;

    public:
        Locked() noexcept : mLocked(), mImpl(nullptr) {}

        explicit operator bool() const noexcept {
            return (bool)mLocked;
        }

        void set_ready() {
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

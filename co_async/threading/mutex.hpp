#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/threading/condition_variable.hpp>

namespace co_async {

template <class T = void>
struct Mutex;

template <class T = void>
struct TimedMutex;

template <>
struct Mutex<void> {
    std::mutex mMutex;
    ConditionVariable mReady;

    Expected<> try_lock() {
        if (mMutex.try_lock()) {
            return Unexpected{std::make_error_code(std::errc::stream_timeout)};
        }
        return {};
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

template <>
struct TimedMutex<void> {
    std::mutex mMutex;
    ConditionTimed mReady;

    Expected<> try_lock() {
        if (mMutex.try_lock()) {
            return Unexpected{std::make_error_code(std::errc::stream_timeout)};
        }
        return {};
    }

    Task<> lock() {
        while (!mMutex.try_lock()) {
            co_await mReady.wait();
        }
    }

    Task<Expected<>> try_lock_for(std::chrono::nanoseconds timeout) {
        auto expires = std::chrono::system_clock::now() + timeout;
        while (!mMutex.try_lock()) {
            if (std::chrono::system_clock::now() > expires || !co_await mReady.wait_until(expires)) [[unlikely]] {
                co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
            }
        }
        co_return {};
    }

    Task<Expected<>> try_lock_until(std::chrono::system_clock::time_point expires) {
        while (!mMutex.try_lock()) {
            if (std::chrono::system_clock::now() > expires || !co_await mReady.wait_until(expires)) [[unlikely]] {
                co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
            }
        }
        co_return {};
    }

    void unlock() {
        mMutex.unlock();
        mReady.notify_one();
    }
};

template <class M, class T>
struct MutexImpl {
private:
    M mMutex;
    T mValue;

public:
    struct Locked {
    private:
        explicit Locked(MutexImpl *that) : mImpl(that) {}

        friend MutexImpl;

    public:
        T &operator*() const {
            return mImpl->unsafe_value();
        }

        void unlock() {
            if (mImpl) {
                mImpl->mMutex.unlock();
                mImpl = nullptr;
            }
        }

        Locked(Locked &&that) : mImpl(std::exchange(that.mImpl, nullptr)) {}

        Locked &operator=(Locked &&that) {
            std::swap(mImpl, that.mImpl);
        }

        ~Locked() {
            unlock();
        }

    private:
        MutexImpl *mImpl;
    };

    Expected<Locked> try_lock() {
        auto e = mMutex.try_lock();
        if (e.has_error()) return Unexpected{e.error()};
        return Locked(this);
    }

    Task<Locked> lock() {
        co_await mMutex.lock();
        co_return Locked(this);
    }

    Task<Expected<Locked>> try_lock_for(std::chrono::nanoseconds timeout) {
        co_await mMutex.try_lock_for(timeout);
        co_return Locked(this);
    }

    Task<Expected<Locked>> try_lock_until(std::chrono::system_clock::time_point expires) {
        co_await mMutex.try_lock_until(expires);
        co_return Locked(this);
    }

    T &unsafe_value() {
        return mValue;
    }

    T const &unsafe_value() const {
        return mValue;
    }
};

template <class T>
struct Mutex : MutexImpl<Mutex<>, T> {};

template <class T>
struct TimedMutex : MutexImpl<TimedMutex<>, T> {};

} // namespace co_async

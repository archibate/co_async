#pragma once

#include <co_async/std.hpp>
#include <co_async/threading/generic_io.hpp>
#include <co_async/system/platform_io.hpp>

namespace co_async {

struct IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::jthread mThread;
    bool mStarted = false;

    struct IOContextGuard {
        explicit IOContextGuard(IOContext *that) {
            if (GenericIOContext::instance || PlatformIOContext::instance)
                [[unlikely]] {
                throw std::logic_error(
                    "each thread may contain only one IOContextGuard");
            }
            GenericIOContext::instance = &that->mGenericIO;
            PlatformIOContext::instance = &that->mPlatformIO;
        }

        ~IOContextGuard() {
            GenericIOContext::instance = nullptr;
            PlatformIOContext::instance = nullptr;
        }

        IOContextGuard(IOContextGuard &&) = delete;
    };

public:
    IOContext() = default;
    IOContext(IOContext &&) = delete;

    void startHere(std::stop_token stop, PlatformIOContextOptions options) {
        mStarted = true;
        IOContextGuard guard(this);
        PlatformIOContext::instance->startMain(stop, options);
    }

    void start(PlatformIOContextOptions options = {}) {
        mStarted = true;
        mThread = std::jthread(
            [this, options = std::move(options)](std::stop_token stop) {
                this->startHere(stop, options);
            });
    }

    void detach(std::coroutine_handle<> coroutine) {
        mGenericIO.enqueueJob(coroutine);
    }

    template <class T, class P>
    void detach(Task<T, P> task) {
        if (!mStarted) [[unlikely]] {
            start();
        }
        auto wrapped = coSpawnStarter(std::move(task));
        auto coroutine = wrapped.get();
        mGenericIO.enqueueJob(coroutine);
        wrapped.release();
    }

    template <class T, class P>
    T join(Task<T, P> task) {
        std::condition_variable cv;
        std::mutex mtx;
        Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
        std::exception_ptr exception;
#endif
        this->detach(joinHelper(std::move(task), cv, result
#if CO_ASYNC_EXCEPT
                                ,
                                exception
#endif
                                ));
        std::unique_lock lck(mtx);
        cv.wait(lck);
        lck.unlock();
#if CO_ASYNC_EXCEPT
        if (exception) [[unlikely]] {
            std::rethrow_exception(exception);
        }
#endif
        if constexpr (!std::is_void_v<T>) {
            return result.moveValue();
        }
    }

private:
    template <class T, class P>
    static Task<> joinHelper(Task<T, P> task, std::condition_variable &cv,
                             Uninitialized<T> &result
#if CO_ASYNC_EXCEPT
                             ,
                             std::exception_ptr exception
#endif
    ) {
#if CO_ASYNC_EXCEPT
        try {
#endif
            result.putValue((co_await task, Void()));
#if CO_ASYNC_EXCEPT
        } catch (...) {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: exception occurred in IOContext::join\n";
#endif
            exception = std::current_exception();
        }
#endif
        cv.notify_one();
    }
};

inline auto co_resume_on(IOContext &context) {
    struct ResumeOnAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mContext.detach(coroutine);
        }

        void await_resume() const noexcept {}

        IOContext &mContext;
    };

    return ResumeOnAwaiter(context);
}

} // namespace co_async

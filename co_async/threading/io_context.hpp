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

    void start_worker_inplace(std::stop_token stop, PlatformIOContextOptions options) {
        mStarted = true;
        IOContextGuard guard(this);
        PlatformIOContext::instance->startMain(stop, options);
    }

    void start_worker(PlatformIOContextOptions options = {}) {
        mThread = std::jthread(
            [this, options = std::move(options)](std::stop_token stop) { this->start_worker_inplace(stop, options); });
        mStarted = true;
    }

    template <class T, class P>
    void detach(Task<T, P> task) {
        if (!mStarted) [[unlikely]] {
            start_worker();
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
            std::cerr << "WARNING: exception occurred in co_synchronize\n";
#endif
            exception = std::current_exception();
        }
#endif
        cv.notify_one();
    }
};

} // namespace co_async

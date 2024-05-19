#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {
struct alignas(hardware_destructive_interference_size) IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::jthread mThread;

    struct IOContextGuard {
        explicit IOContextGuard(IOContext *that) {
            if (IOContext::instance || GenericIOContext::instance ||
                PlatformIOContext::instance) [[unlikely]] {
                throw std::logic_error(
                    "each thread may contain only one IOContextGuard");
            }
            IOContext::instance = that;
            GenericIOContext::instance = &that->mGenericIO;
            PlatformIOContext::instance = &that->mPlatformIO;
        }

        ~IOContextGuard() {
            IOContext::instance = nullptr;
            GenericIOContext::instance = nullptr;
            PlatformIOContext::instance = nullptr;
        }

        IOContextGuard(IOContextGuard &&) = delete;
    };

public:
    explicit IOContext(std::in_place_t) {}

    explicit IOContext(PlatformIOContextOptions options = {}) {
        start(options);
    }

    IOContext(IOContext &&) = delete;

    void startHere(std::stop_token stop, PlatformIOContextOptions options,
                   std::span<IOContext> peerContexts) {
        IOContextGuard guard(this);
        if (options.threadAffinity) {
            PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
        }
        auto maxSleep = options.maxSleep;
        while (!stop.stop_requested()) [[likely]] {
            auto duration = GenericIOContext::instance->runDuration();
            if (GenericIOContext::instance->runMTQueue()) {
                continue;
            }
            if (!duration || *duration > maxSleep) {
                duration = maxSleep;
            }
            bool hasEvent =
                PlatformIOContext::instance->waitEventsFor(1, duration);
            if (hasEvent) {
                auto t = maxSleep + options.maxSleepInc;
                if (t > options.maxSleepLimit) {
                    t = options.maxSleepLimit;
                }
                maxSleep = t;
            } else {
                maxSleep = options.maxSleep;
            }
#if CO_ASYNC_STEAL
            if (!hasEvent && !peerContexts.empty()) {
                for (IOContext *p = peerContexts.data();
                     p != peerContexts.data() + peerContexts.size(); ++p) {
                    if (p->mGenericIO.runComputeOnly()) {
                        break;
                    }
                }
            }
#endif
        }
    }

    void start(PlatformIOContextOptions options = {},
               std::span<IOContext> peerContexts = {}) {
        mThread = std::jthread([this, options = std::move(options),
                                peerContexts](std::stop_token stop) {
            this->startHere(stop, options, peerContexts);
        });
    }

    void spawn(std::coroutine_handle<> coroutine) {
        mGenericIO.enqueueJob(coroutine);
    }

    void spawn_mt(std::coroutine_handle<> coroutine) /* MT-safe */ {
        mGenericIO.enqueueJobMT(coroutine);
    }

    template <class T, class P>
    void spawn(Task<T, P> task) {
        auto wrapped = coSpawnStarter(std::move(task));
        auto coroutine = wrapped.get();
        mGenericIO.enqueueJob(coroutine);
        wrapped.release();
    }

    template <class T, class P>
    T join(Task<T, P> task) {
        return contextJoin(*this, std::move(task));
    }

    static inline thread_local IOContext *instance;
};

template <class T, class P>
inline Task<> contextJoinHelper(Task<T, P> task, std::condition_variable &cv,
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
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: exception occurred in IOContext::join\n";
# endif
        exception = std::current_exception();
    }
#endif
    cv.notify_one();
}

template <class T, class P>
T contextJoin(IOContext &context, Task<T, P> task) {
    std::condition_variable cv;
    std::mutex mtx;
    Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
    std::exception_ptr exception;
#endif
    context.spawn(contextJoinHelper(std::move(task), cv, result
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

inline auto co_resume_on(IOContext &context) {
    struct ResumeOnAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mContext.spawn(coroutine);
        }

        void await_resume() const noexcept {}

        IOContext &mContext;
    };

    return ResumeOnAwaiter(context);
}
} // namespace co_async

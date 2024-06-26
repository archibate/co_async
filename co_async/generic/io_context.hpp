#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {
struct IOContextOptions {
    // std::chrono::steady_clock::duration maxSleep =
    // std::chrono::milliseconds(100);
    std::optional<std::size_t> threadAffinity = std::nullopt;
    std::size_t queueEntries = 512;
};

struct alignas(hardware_destructive_interference_size) IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::jthread mThread;

    struct IOContextGuard;

public:
    explicit IOContext(std::in_place_t) {}

    explicit IOContext(IOContextOptions options = {}) {
        start(options);
    }

    IOContext(IOContext &&) = delete;

    [[gnu::hot]] void startHere(std::stop_token stop, IOContextOptions options,
                                std::span<IOContext> peerContexts);

    void start(IOContextOptions options = {},
               std::span<IOContext> peerContexts = {});

    [[gnu::hot]] void spawn(std::coroutine_handle<> coroutine) {
        mGenericIO.enqueueJob(coroutine);
        wakeUp();
    }

    template <class T, class P>
    void spawn(Task<T, P> task) {
        auto wrapped = coSpawnStarter(std::move(task));
        mGenericIO.enqueueJob(wrapped.get());
        wrapped.release();
        wakeUp();
    }

    template <class T, class P>
    T join(Task<T, P> task) {
        return contextJoin(*this, std::move(task));
    }

    static thread_local IOContext *instance;
#if CO_ASYNC_ALLOC
    static thread_local std::pmr::memory_resource *currentAllocator;
#endif

private:
    void wakeUp();
    Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>> watchDogTask();
    std::atomic<std::uint8_t> mWake{0};
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
        result.emplace((co_await task, Void()));
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
        return result.move();
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

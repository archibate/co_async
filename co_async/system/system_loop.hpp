#pragma once

#include <co_async/std.hpp>
#include <co_async/system/uring_loop.hpp>
#include <co_async/threading/basic_loop.hpp>
#include <co_async/threading/cancel.hpp>
#include <co_async/awaiter/task.hpp>
#if defined(__linux__) && defined(_GLIBCXX_HAS_GTHREADS)
#include <pthread.h>
#elif defined(_WIN32) && defined(_EXPORT_STD)
#include <winbase.h>
#endif

namespace co_async {

struct SystemLoop {
    bool is_started() const noexcept {
        return mThreads != nullptr;
    }

    static bool is_this_thread_worker() noexcept {
        return BasicLoop::tlsInstance != nullptr;
    }

    std::size_t this_thread_worker_id() const noexcept {
        return BasicLoop::tlsInstance - mBasicLoops.get();
    }

    std::size_t num_workers() const noexcept {
        return mNumWorkers;
    }

    struct StartOptions {
        std::size_t numWorkers = 0;
        /* std::size_t numBatchWait = 0; */
        /* std::chrono::steady_clock::duration batchTimeout = std::chrono::milliseconds(40); */
        /* std::chrono::steady_clock::duration batchTimeoutDelta = std::chrono::milliseconds(20); */
    };

    void start() {
        return start({});
    }

    void start(StartOptions options) {
        if (mThreads) [[unlikely]] {
            throw std::runtime_error("loop already started");
        }
        bool setAffinity = false;
        if (options.numWorkers < 1) [[unlikely]] {
            options.numWorkers = std::thread::hardware_concurrency();
            setAffinity = true;
        }
        mThreads = std::make_unique<std::thread[]>(options.numWorkers);
        mBasicLoops = std::make_unique<BasicLoop[]>(options.numWorkers);
        mUringLoops = std::make_unique<UringLoop[]>(options.numWorkers);
        mNumWorkers = options.numWorkers;
        for (std::size_t i = 0; i < options.numWorkers; ++i) {
            mThreads[i] =
                std::thread(&SystemLoop::threadEntry, this, i, options);
#if defined(__linux__) && defined(_GLIBCXX_HAS_GTHREADS)
            if (setAffinity) {
                pthread_t h = mThreads[i].native_handle();
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(i, &cpuset);
                pthread_setaffinity_np(h, sizeof(cpuset), &cpuset);
            }
#elif defined(_WIN32) && defined(_EXPORT_STD)
            if (setAffinity && numWorkers <= 64) {
                SetThreadAffinityMask(mThreads[i].native_handle(), 1ull << i);
            }
#endif
        }
    }

    void threadEntry(std::size_t i, StartOptions options) {
        auto &thisBasicLoop = mBasicLoops[i];
        auto &thisUringLoop = mUringLoops[i];
        BasicLoop::tlsInstance = &thisBasicLoop;
        UringLoop::tlsInstance = &thisUringLoop;
        auto stealWork = [&] {
            for (std::size_t j = 1; j < options.numWorkers; ++j) {
                auto other = (i + j) % options.numWorkers;
                if (mBasicLoops[other].run()) {
                    return true;
                }
            }
            return false;
        };
    compute:
        if (thisBasicLoop.run())
            goto compute;
        if (auto n = thisUringLoop.hasAnyEvent()) {
            thisUringLoop.runBatchedNoWait(n);
            goto compute;
        }
        if (stealWork())
            goto compute;
        if (thisBasicLoop.run())
            goto compute;
        while (!mStop.stop_requested()) [[likely]] {
            struct __kernel_timespec ts, *tsp;
            if (auto timeout = thisBasicLoop.runTimers()) {
                tsp = &(ts = durationToKernelTimespec(*timeout));
            } else {
                tsp = nullptr;
            }
            if (thisUringLoop.runBatchedWait(1, tsp))
                goto compute;
            if (thisBasicLoop.run())
                goto compute;
            if (stealWork())
                goto compute;
        }
    }

    void stop() {
        if (mThreads) {
            mStop.request_stop();
            for (std::size_t i = 0; i < mNumWorkers; ++i) {
                mThreads[i].join();
            }
            mThreads.reset();
            mBasicLoops.reset();
            mUringLoops.reset();
        }
    }

    ~SystemLoop() {
        stop();
    }

    SystemLoop() = default;
    SystemLoop(SystemLoop &&) = delete;

    BasicLoop &getAnyWorkingLoop() {
#if CO_ASYNC_DEBUG
        if (is_this_thread_worker()) [[unlikely]] {
            throw std::logic_error("cannot be called on a worker thread");
        }
        if (!mBasicLoops) [[unlikely]] {
            throw std::logic_error("loop not started");
        }
#endif
        return mBasicLoops[0];
    }

    BasicLoop &currentWorkerLoop() {
#if CO_ASYNC_DEBUG
        if (!is_this_thread_worker()) [[unlikely]] {
            throw std::logic_error("currently not on worker thread");
        }
#endif
        return *BasicLoop::tlsInstance;
    }

    BasicLoop &nthWorkerLoop(std::size_t index) {
#if CO_ASYNC_DEBUG
        if (!mBasicLoops) [[unlikely]] {
            throw std::logic_error("loop not started");
        }
#endif
        return mBasicLoops[index];
    }

private:
    std::unique_ptr<BasicLoop[]> mBasicLoops;
    std::unique_ptr<UringLoop[]> mUringLoops;
    std::unique_ptr<std::thread[]> mThreads;
    std::size_t mNumWorkers;
    std::stop_source mStop;
};

inline SystemLoop globalSystemLoop;

template <Awaitable A>
inline void co_spawn(A awaitable) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopEnqueueDetached(*BasicLoop::tlsInstance, std::move(awaitable));
}

template <Awaitable A>
inline void co_spawn(std::size_t hintWorkerId, A awaitable) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopEnqueueDetached(globalSystemLoop.nthWorkerLoop(hintWorkerId),
                               std::move(awaitable));
}

inline void co_spawn(std::coroutine_handle<> coroutine) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopEnqueueHandle(*BasicLoop::tlsInstance, std::move(coroutine));
}

template <class T, class P>
inline auto co_synchronize(Task<T, P> task) {
#if CO_ASYNC_DEBUG
    if (globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("cannot be called on a worker thread");
    }
#endif
    if (!globalSystemLoop.is_started())
        globalSystemLoop.start();
    return loopEnqueueSynchronized(globalSystemLoop.getAnyWorkingLoop(),
                                   std::move(task));
}

inline Task<Expected<>, BasicLoop::TimerNode> co_sleep(std::chrono::steady_clock::time_point expires) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopWaitTimer(*BasicLoop::tlsInstance, expires);
}

inline Task<Expected<>, BasicLoop::TimerNode> co_sleep(std::chrono::nanoseconds timeout) {
    return co_sleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<Expected<>, BasicLoop::TimerNode> co_sleep(std::chrono::steady_clock::time_point expires, CancelToken cancel) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopWaitTimer(*BasicLoop::tlsInstance, expires, cancel);
}

inline Task<Expected<>, BasicLoop::TimerNode> co_sleep(std::chrono::nanoseconds timeout, CancelToken cancel) {
    return co_sleep(std::chrono::steady_clock::now() + timeout, cancel);
}

} // namespace co_async

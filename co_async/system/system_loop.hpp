#pragma once

#include <co_async/std.hpp>
#include <co_async/system/uring_loop.hpp>
#include <co_async/threading/basic_loop.hpp>
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

    void start(std::size_t numWorkers = 0, std::size_t numBatchWait = 1,
               std::chrono::system_clock::duration batchTimeout =
                   std::chrono::milliseconds(40),
               std::chrono::system_clock::duration batchTimeoutDelta =
                   std::chrono::milliseconds(20)) {
        if (mThreads) [[unlikely]] {
            throw std::runtime_error("loop already started");
        }
        bool setAffinity = false;
        if (numWorkers < 1) [[unlikely]] {
            numWorkers = std::thread::hardware_concurrency();
            setAffinity = true;
        }
        mThreads = std::make_unique<std::thread[]>(numWorkers);
        mBasicLoops = std::make_unique<BasicLoop[]>(numWorkers);
        mUringLoops = std::make_unique<UringLoop[]>(numWorkers);
        mNumWorkers = numWorkers;
        for (std::size_t i = 0; i < numWorkers; ++i) {
            mThreads[i] =
                std::thread(&SystemLoop::threadEntry, this, i, numWorkers,
                            numBatchWait, batchTimeout, batchTimeoutDelta);
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

    void threadEntry(std::size_t i, std::size_t numWorkers,
                     std::size_t numBatchWait,
                     std::chrono::system_clock::duration batchTimeout,
                     std::chrono::system_clock::duration batchTimeoutDelta) {
        auto &thisBasicLoop = mBasicLoops[i];
        auto &thisUringLoop = mUringLoops[i];
        BasicLoop::tlsInstance = &thisBasicLoop;
        UringLoop::tlsInstance = &thisUringLoop;
        auto stealWork = [&] {
            for (std::size_t j = 1; j < numWorkers; ++j) {
                auto other = (i + j) % numWorkers;
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
        auto timeout = batchTimeout;
        while (!mStop.stop_requested()) [[likely]] {
            auto ts = durationToKernelTimespec(timeout);
            if (thisUringLoop.runBatchedWait(numBatchWait, &ts))
                goto compute;
            if (thisBasicLoop.run())
                goto compute;
            if (stealWork())
                goto compute;
            timeout += batchTimeoutDelta;
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

template <class T, class P>
inline void co_spawn(Task<T, P> &&task) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopEnqueueDetached(*BasicLoop::tlsInstance, std::move(task));
}

template <class T, class P>
inline void co_spawn(std::size_t hintWorkerId, Task<T, P> task) {
#if CO_ASYNC_DEBUG
    if (!globalSystemLoop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loopEnqueueDetached(globalSystemLoop.nthWorkerLoop(hintWorkerId),
                               std::move(task));
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

template <class F, class... Args>
    requires (Awaitable<std::invoke_result_t<F, Args...>>)
inline auto co_bind(F &&f, Args &&...args) {
    return [](auto f) mutable -> std::invoke_result_t<F, Args...> {
        co_return co_await std::move(f)();
    }(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}

} // namespace co_async

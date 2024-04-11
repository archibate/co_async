#pragma once /*{export module co_async:system.system_loop;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/system/uring_loop.hpp>   /*{import :system.uring_loop;}*/
#include <co_async/threading/basic_loop.hpp>/*{import :threading.basic_loop;}*/
#include <co_async/awaiter/task.hpp>        /*{import :awaiter.task;}*/

namespace co_async {

struct SystemLoop {
    bool is_started() const noexcept {
        return mThreads != nullptr;
    }

    static bool is_this_thread_worker() noexcept {
        return tlsThisBasicLoop != nullptr;
    }

    int this_thread_worker_id() const noexcept {
        return tlsThisBasicLoop - mBasicLoops.get();
    }

    void start(std::size_t numWorkers = 0, std::size_t numBatchFetch = 1,
               std::size_t numBatchWait = 1,
               std::chrono::system_clock::duration batchTimeout =
                   std::chrono::milliseconds(15),
               std::chrono::system_clock::duration batchTimeoutDelta =
                   std::chrono::milliseconds(12)) {
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
            mThreads[i] = std::thread(&SystemLoop::threadEntry, this, i,
                                      numWorkers, numBatchFetch, numBatchWait,
                                      batchTimeout, batchTimeoutDelta);
#if defined(__linux__) && defined(_GLIBCXX_HAS_GTHREADS) && defined(CPU_ZERO)
            if (setAffinity) {
                pthread_t h = mThreads[i].native_handle();
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(i, &cpuset);
                pthread_setaffinity_np(h, sizeof(cpuset), &cpuset);
            }
#endif
        }
    }

    void threadEntry(int i, int numWorkers, int numBatchFetch, int numBatchWait,
                     std::chrono::system_clock::duration batchTimeout,
                     std::chrono::system_clock::duration batchTimeoutDelta) {
        auto &thisBasicLoop = mBasicLoops[i];
        auto &thisUringLoop = mUringLoops[i];
        tlsThisBasicLoop = &thisBasicLoop;
        tlsThisUringLoop = &thisUringLoop;
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
        thisBasicLoop.run();
    event:
        if (thisUringLoop.hasAnyEvent()) {
            thisUringLoop.runBatchedNoWait(thisBasicLoop, numBatchFetch);
            goto compute;
        }
        if (stealWork())
            goto compute;
        auto timeout = batchTimeout;
        while (!mStop.stop_requested()) [[likely]] {
            auto ts = durationToKernelTimespec(timeout);
            if (thisUringLoop.runBatchedWait(thisBasicLoop, numBatchWait, &ts))
                goto compute;
            if (thisBasicLoop.run())
                goto event;
            if (stealWork())
                goto compute;
            timeout += batchTimeoutDelta;
        }
    }

    void stop() {
        if (mThreads) {
            mStop.request_stop();
            for (int i = 0; i < mNumWorkers; ++i) {
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

#if defined(__GNUC__) && __has_attribute(const) && \
    __has_attribute(always_inline)
    __attribute__((const, always_inline))
#endif
    operator BasicLoop &() {
#if CO_ASYNC_DEBUG
        if (!tlsThisBasicLoop) [[unlikely]] {
            throw std::logic_error("not a worker thread");
        }
#endif
        return *tlsThisBasicLoop;
    }

#if defined(__GNUC__) && __has_attribute(const) && \
    __has_attribute(always_inline)
    __attribute__((const, always_inline))
#endif
    operator UringLoop &() {
#if CO_ASYNC_DEBUG
        if (!tlsThisUringLoop) [[unlikely]] {
            throw std::logic_error("not a worker thread");
        }
#endif
        return *tlsThisUringLoop;
    }

    BasicLoop &getWorkerLoop() {
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

private:
    std::unique_ptr<BasicLoop[]> mBasicLoops;
    std::unique_ptr<UringLoop[]> mUringLoops;
    std::unique_ptr<std::thread[]> mThreads;
    std::size_t mNumWorkers;
    std::stop_source mStop;
    static inline thread_local BasicLoop *tlsThisBasicLoop;
    static inline thread_local UringLoop *tlsThisUringLoop;
};

/*[export]*/ inline SystemLoop loop;

template <class T>
/*[export]*/ inline Future<T> make_future() {
    return Future<T>(loop);
}

template <class T, class P>
/*[export]*/ inline void co_spawn(Task<T, P> &&task) {
#if CO_ASYNC_DEBUG
    if (!loop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loop_enqueue_detach(loop, std::move(task));
}

template <class T, class P>
/*[export]*/ inline Future<T> co_future(Task<T, P> &&task) {
#if CO_ASYNC_DEBUG
    if (!loop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("not a worker thread");
    }
#endif
    return loop_enqueue_future(loop, std::move(task));
}

template <class T, class P>
/*[export]*/ inline auto co_synchronize(Task<T, P> task) {
#if CO_ASYNC_DEBUG
    if (loop.is_this_thread_worker()) [[unlikely]] {
        throw std::logic_error("cannot be called on a worker thread");
    }
#endif
    if (!loop.is_started())
        loop.start();
    return loop_enqueue_synchronized(loop.getWorkerLoop(), std::move(task));
}

template <class F, class... Args>
    requires std::is_invocable_r_v<Task<>, F, Args...>
/*[export]*/ inline void co_spawn(F &&f, Args &&...args) {
    Task<> task = [](auto f) mutable -> Task<> {
        co_await std::move(f)();
    }(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    return co_spawn(std::move(task));
}

} // namespace co_async

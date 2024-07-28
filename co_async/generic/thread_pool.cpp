#include <co_async/awaiter/task.hpp>
#include <co_async/generic/thread_pool.hpp>
#include <co_async/platform/futex.hpp>

namespace co_async {

struct ThreadPool::Thread {
    std::function<void()> mTask;
    std::condition_variable mCV;
    std::mutex mMutex;
    ThreadPool *mPool;
    std::jthread mThread;

    ~Thread() {
        mThread.request_stop();
        mCV.notify_one();
        mThread.join();
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            std::unique_lock lock(mMutex);
            mCV.wait(lock,
                     [&] { return mTask != nullptr || stop.stop_requested(); });
            if (stop.stop_requested()) [[unlikely]] {
                return;
            }
            auto task = std::exchange(mTask, nullptr);
            lock.unlock();
            task();
            lock.lock();
            mTask = nullptr;
            std::unique_lock workingLock(mPool->mWorkingMutex);
            mPool->mWorkingThreads.remove(this);
            workingLock.unlock();
            std::lock_guard freeLock(mPool->mFreeMutex);
            mPool->mFreeThreads.push_back(this);
        }
    }

    explicit Thread(ThreadPool *pool) {
        mPool = pool;
        mThread =
            std::jthread([this](std::stop_token stop) { startMain(stop); });
    }

    Thread(Thread &&) = delete;

    void workOn(std::function<void()> func) {
        std::lock_guard lock(mMutex);
        mTask = std::move(func);
        mCV.notify_one();
    }
};

ThreadPool::Thread *ThreadPool::submitJob(std::function<void()> func) {
    std::unique_lock freeLock(mFreeMutex);
    if (mFreeThreads.empty()) {
        freeLock.unlock();
        std::unique_lock threadsLock(mThreadsMutex);
        // list 保证插入后元素的指针和引用永远不会失效
        // 所以在这里获取地址并存入其他链表是安全的：
        Thread *newThread = &mThreads.emplace_back(this);
        threadsLock.unlock();
        newThread->workOn(std::move(func));
        std::lock_guard workingLock(mWorkingMutex);
        mWorkingThreads.push_back(newThread);
        return newThread;
    } else {
        Thread *freeThread = std::move(mFreeThreads.front());
        mFreeThreads.pop_front();
        freeLock.unlock();
        freeThread->workOn(std::move(func));
        std::lock_guard workingLock(mWorkingMutex);
        mWorkingThreads.push_back(freeThread);
        return freeThread;
    }
}

Task<Expected<>> ThreadPool::rawRun(std::function<void()> func) {
    auto ready = std::make_shared<FutexAtomic<bool>>(false);
    std::exception_ptr ep;
    submitJob([ready, func = std::move(func), &ep]() mutable {
        try {
            func();
        } catch (...) {
            ep = std::current_exception();
        }
        ready->store(true, std::memory_order_release);
        (void)futex_notify_sync(ready.get());
    });
    while (ready->load(std::memory_order_acquire) == false) {
        (void)co_await futex_wait(ready.get(), false);
    }
    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    co_return {};
}

Task<Expected<>> ThreadPool::rawRun(std::function<void(std::stop_token)> func,
                                    CancelToken cancel) {
    auto ready = std::make_shared<FutexAtomic<bool>>(false);
    std::stop_source stop;
    bool stopped = false;
    std::exception_ptr ep;
    submitJob([ready, func = std::move(func), stop = stop.get_token(),
               &ep]() mutable {
        try {
            func(stop);
        } catch (...) {
            ep = std::current_exception();
        }
        ready->store(true, std::memory_order_release);
        (void)futex_notify_sync(ready.get());
    });

    {
        CancelCallback _(cancel, [&] {
            stopped = true;
            stop.request_stop();
        });
        while (ready->load(std::memory_order_acquire) == false) {
            (void)co_await futex_wait(ready.get(), false);
        }
    }

    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    if (stopped) {
        co_return std::errc::operation_canceled;
    }
    co_return {};
}

std::size_t ThreadPool::threads_count() {
    std::lock_guard lock(mThreadsMutex);
    return mThreads.size();
}

std::size_t ThreadPool::working_threads_count() {
    std::lock_guard lock(mWorkingMutex);
    return mWorkingThreads.size();
}

ThreadPool::ThreadPool() = default;
ThreadPool::~ThreadPool() = default;

} // namespace co_async

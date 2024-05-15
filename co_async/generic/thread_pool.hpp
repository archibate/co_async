#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {

struct ThreadPool {
private:
    struct Thread {
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
                mCV.wait(lock, [&] {
                    return mTask != nullptr || stop.stop_requested();
                });
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

    std::mutex mWorkingMutex;
    std::list<Thread *> mWorkingThreads;
    std::mutex mFreeMutex;
    std::list<Thread *> mFreeThreads;
    std::mutex mThreadsMutex;
    // deque 保证插入后元素的指针和引用永远不会失效：
    std::deque<Thread> mThreads;

public:
    Task<> run(std::function<void()> func) {
        std::unique_lock freeLock(mFreeMutex);
        if (mFreeThreads.empty()) {
            freeLock.unlock();
            std::unique_lock threadsLock(mThreadsMutex);
            // 所以在这里获取地址并存入链表是安全的：
            Thread *newThread = &mThreads.emplace_back(this);
            threadsLock.unlock();
            newThread->workOn(std::move(func));
            std::lock_guard workingLock(mWorkingMutex);
            mWorkingThreads.push_back(std::move(newThread));
        } else {
            Thread *freeThread = std::move(mFreeThreads.front());
            mFreeThreads.pop_front();
            freeLock.unlock();
            freeThread->workOn(std::move(func));
            std::lock_guard workingLock(mWorkingMutex);
            mWorkingThreads.push_back(std::move(freeThread));
        }
        co_return;
    }

    std::size_t threads_count() const {
        std::lock_guard lock(mThreadsMutex);
        return mThreads.size();
    }

    std::size_t working_threads_count() const {
        std::lock_guard lock(mWorkingMutex);
        return mWorkingThreads.size();
    }

    ThreadPool &operator=(ThreadPool &&) = delete;
};

}

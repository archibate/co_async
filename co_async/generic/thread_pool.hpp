#pragma once

#include <co_async/std.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>

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

    Thread *submitJob(std::function<void()> func) {
        std::unique_lock freeLock(mFreeMutex);
        if (mFreeThreads.empty()) {
            freeLock.unlock();
            std::unique_lock threadsLock(mThreadsMutex);
            // 所以在这里获取地址并存入链表是安全的：
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

public:
    Task<> run(std::function<void()> func) {
        auto cv = std::make_shared<ConditionVariable>();
        submitJob([cv, ctx = IOContext::instance, func = std::move(func)] () mutable {
            func();
            if (auto coroutine = cv->notify_pop_coroutine()) [[likely]] {
                debug(), coroutine;
                ctx->spawn_mt(coroutine);
            }
        });
        co_await *cv;
        co_return;
    }

    Task<> run(std::function<void(std::stop_token)> func, CancelToken cancel) {
        auto cv = std::make_shared<ConditionVariable>();
        std::stop_source stop;
        submitJob([cv, ctx = IOContext::instance, func = std::move(func), stop = stop.get_token()] () mutable {
            func(stop);
            if (auto p = cv->notify_pop_coroutine()) [[likely]] {
                ctx->spawn_mt(p);
            }
        });
        struct Awaitable {
            ConditionVariable &cv;
            std::stop_source &stop;

            auto operator co_await() const noexcept {
                return cv.operator co_await();
            }

            struct Canceller {
                using OpType = Awaitable;

                static Task<> doCancel(OpType *op) {
                    op->stop.request_stop();
                    co_return;
                }
            };
        };
        co_await cancel.guard(Awaitable(*cv, stop));
        co_return;
    }

    std::size_t threads_count() {
        std::lock_guard lock(mThreadsMutex);
        return mThreads.size();
    }

    std::size_t working_threads_count() {
        std::lock_guard lock(mWorkingMutex);
        return mWorkingThreads.size();
    }

    ThreadPool &operator=(ThreadPool &&) = delete;
};

}

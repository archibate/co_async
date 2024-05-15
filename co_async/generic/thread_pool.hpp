#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/generic/io_context.hpp>

namespace co_async {

struct ThreadPool {
private:
    struct Thread;

    std::mutex mWorkingMutex;
    std::list<Thread *> mWorkingThreads;
    std::mutex mFreeMutex;
    std::list<Thread *> mFreeThreads;
    std::mutex mThreadsMutex;
    std::list<Thread> mThreads;

    Thread *submitJob(std::function<void()> func);

public:
    Task<> run(std::function<void()> func) /* MT-safe */;

    Task<Expected<>> run(std::function<void(std::stop_token)> func,
                         CancelToken cancel) /* MT-safe */;

    template <class T>
    Task<Expected<T>> run(std::function<T()> func) /* MT-safe */ {
        std::optional<T> res;
        auto e = co_await run(
            [&res, func = std::move(func)]() mutable { res = func(); });
        if (e.has_error()) {
            co_return Unexpected{e.error()};
        }
        if (!res) [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        co_return std::move(*res);
    }

    template <class T>
    Task<Expected<T>> run(std::function<T(std::stop_token)> func,
                          CancelToken cancel) /* MT-safe */ {
        std::optional<T> res;
        auto e = co_await run(
            [&res, func = std::move(func)](std::stop_token stop) mutable {
                res = func(stop);
            });
        if (e.has_error()) {
            co_return Unexpected{e.error()};
        }
        if (!res) [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        co_return std::move(*res);
    }

    std::size_t threads_count() /* MT-safe */;
    std::size_t working_threads_count() /* MT-safe */;

    ThreadPool();
    ~ThreadPool();
    ThreadPool &operator=(ThreadPool &&) = delete;
};

} // namespace co_async

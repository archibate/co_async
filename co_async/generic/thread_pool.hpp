#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/utils/spin_mutex.hpp>

namespace co_async {

struct ThreadPool {
private:
    struct Thread;

    SpinMutex mWorkingMutex;
    std::list<Thread *> mWorkingThreads;
    SpinMutex mFreeMutex;
    std::list<Thread *> mFreeThreads;
    SpinMutex mThreadsMutex;
    std::list<Thread> mThreads;

    Thread *submitJob(std::function<void()> func);

public:
    Task<Expected<>> rawRun(std::function<void()> func) /* MT-safe */;
    Task<Expected<>> rawRun(std::function<void(std::stop_token)> func,
                            CancelToken cancel) /* MT-safe */;

    auto run(std::invocable auto func) /* MT-safe */
        -> Task<Expected<std::invoke_result_t<decltype(func)>>>
        requires(!std::invocable<decltype(func), std::stop_token>)
    {
        std::optional<Avoid<std::invoke_result_t<decltype(func)>>> res;
        co_await co_await rawRun([&res, func = std::move(func)]() mutable {
            res = (func(), Void());
        });
        if (!res) [[unlikely]] {
            co_return std::errc::operation_canceled;
        }
        co_return std::move(*res);
    }

    auto run(std::invocable<std::stop_token> auto func,
             CancelToken cancel) /* MT-safe */
        -> Task<
            Expected<std::invoke_result_t<decltype(func), std::stop_token>>> {
        std::optional<
            Avoid<std::invoke_result_t<decltype(func), std::stop_token>>>
            res;
        auto e = co_await rawRun(
            [&res, func = std::move(func)](std::stop_token stop) mutable {
                res = (func(stop), Void());
            });
        if (e.has_error()) {
            co_return CO_ASYNC_ERROR_FORWARD(e);
        }
        if (!res) {
            co_return std::errc::operation_canceled;
        }
        co_return std::move(*res);
    }

    auto run(std::invocable<std::stop_token> auto func) /* MT-safe */
        -> Task<
            Expected<std::invoke_result_t<decltype(func), std::stop_token>>> {
        co_return co_await run(func, co_await co_cancel);
    }

    std::size_t threads_count() /* MT-safe */;
    std::size_t working_threads_count() /* MT-safe */;

    ThreadPool();
    ~ThreadPool();
    ThreadPool &operator=(ThreadPool &&) = delete;
};

} // namespace co_async

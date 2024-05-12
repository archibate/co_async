#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/threading/future.hpp>

namespace co_async {

struct FutureGroup {
    std::vector<FutureSource<Expected<>>> mFutures;

    FutureGroup &add(FutureSource<Expected<>> future) {
        mFutures.push_back(std::move(future));
        return *this;
    }

    FutureGroup &add(Task<Expected<>> task) {
        add(co_future(std::move(task)));
        return *this;
    }

    template <class F, class... Args>
        requires(
            std::same_as<std::invoke_result_t<F, Args...>, Task<Expected<>>>)
    FutureGroup &add(F &&f, Args &&...args) {
        add(co_future(
            co_bind(std::forward<F>(f), std::forward<Args>(args)...)));
        return *this;
    }

    Task<Expected<>> wait() {
        for (auto &result: co_await when_all(mFutures)) {
            co_await std::move(result);
        }
        mFutures.clear();
        co_return {};
    }

    FutureGroup() = default;
    FutureGroup(FutureGroup &&) = default;
    FutureGroup &operator=(FutureGroup &&) = default;
};

struct TaskGroup {
    std::vector<Task<>> mTasks;

    TaskGroup &add(Task<> task) {
        mTasks.push_back(std::move(task));
        return *this;
    }

    template <class F, class... Args>
        requires(
            std::same_as<std::invoke_result_t<F, Args...>, Task<>>)
    TaskGroup &add(F &&f, Args &&...args) {
        add(co_bind(std::forward<F>(f), std::forward<Args>(args)...));
        return *this;
    }

    Task<> wait() {
        co_await when_all(mTasks);
        mTasks.clear();
    }

    TaskGroup() = default;
    TaskGroup(TaskGroup &&) = default;
    TaskGroup &operator=(TaskGroup &&) = default;
};

} // namespace co_async

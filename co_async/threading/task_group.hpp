#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>

namespace co_async {

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

}

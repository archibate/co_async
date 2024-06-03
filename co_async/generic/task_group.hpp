#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/generic/future.hpp>

namespace co_async {
template <class T = void>
struct TaskGroup {
    std::vector<FutureSource<T>> mTasks;

    TaskGroup &add(FutureSource<T> future) {
        mTasks.push_back(std::move(future));
        return *this;
    }

    template <class P>
    TaskGroup &add(Task<T, P> task) {
        add(co_future(std::move(task)));
        return *this;
    }

    template <class F, class... Args>
        requires(std::same_as<std::invoke_result_t<F, Args...>, Task<T>>)
    TaskGroup &add(F &&f, Args &&...args) {
        add(co_bind(std::forward<F>(f), std::forward<Args>(args)...));
        return *this;
    }

    Task<std::conditional_t<!std::is_void_v<T>, std::vector<T>, void>> wait() {
        auto ret = (co_await when_all(mTasks), Void());
        mTasks.clear();
        co_return Void() | std::move(ret);
    }

    auto operator co_await() {
        return wait().operator co_await();
    }

    TaskGroup() = default;
    TaskGroup(TaskGroup &&) = default;
    TaskGroup &operator=(TaskGroup &&) = default;
};
} // namespace co_async

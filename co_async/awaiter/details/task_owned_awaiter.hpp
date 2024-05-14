#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {
template <class T = void, class P = TaskPromise<T>>
struct TaskOwnedAwaiter : Task<T, P>::Awaiter {
private:
    Task<T, P> mTask;

public:
    explicit TaskOwnedAwaiter(Task<T, P> task)
        : Task<T, P>::Awaiter(task.operator co_await()),
          mTask(std::move(task)) {}
};
template <class T, class P>
TaskOwnedAwaiter(Task<T, P>) -> TaskOwnedAwaiter<T, P>;
} // namespace co_async

#pragma once /*{export module co_async:awaiter.details.return_previous;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/awaiter/task.hpp>             /*{import :awaiter.task;}*/

namespace co_async {

struct ReturnPreviousPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() {
        throw;
    }

    void return_value(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    auto get_return_object() {
        return std::coroutine_handle<ReturnPreviousPromise>::from_promise(
            *this);
    }

    std::coroutine_handle<> mPrevious;

    ReturnPreviousPromise &operator=(ReturnPreviousPromise &&) = delete;
};

struct [[nodiscard]] ReturnPreviousTask {
    using promise_type = ReturnPreviousPromise;

    ReturnPreviousTask(std::coroutine_handle<promise_type> coroutine) noexcept
        : mCoroutine(coroutine) {}

    ReturnPreviousTask(ReturnPreviousTask &&) = delete;

    ~ReturnPreviousTask() {
        mCoroutine.destroy();
    }

    std::coroutine_handle<promise_type> mCoroutine;
};

} // namespace co_async

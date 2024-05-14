#pragma once
#include <co_async/std.hpp>

namespace co_async {
struct PreviousAwaiter {
    std::coroutine_handle<> mPrevious;

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    void await_resume() const noexcept {}
};
} // namespace co_async

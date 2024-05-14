#pragma once
#include <co_async/std.hpp>

namespace co_async {
struct CurrentCoroutineAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) noexcept {
        mCurrent = coroutine;
        return coroutine;
    }

    auto await_resume() const noexcept {
        return mCurrent;
    }

    std::coroutine_handle<> mCurrent;
};
} // namespace co_async

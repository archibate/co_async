#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {
template <class T>
struct ValueAwaiter {
    std::coroutine_handle<> mPrevious;
    Uninitialized<T> mValue;

    template <class... Args>
    explicit ValueAwaiter(std::in_place_t, Args &&...args) {
        mValue.putValue(std::forward<Args>(args)...);
    }

    explicit ValueAwaiter(std::in_place_t)
        requires(std::is_void_v<T>)
    {
        mValue.putValue(Void());
    }

    explicit ValueAwaiter(std::coroutine_handle<> previous)
        : mPrevious(previous) {}

    bool await_ready() const noexcept {
        return mPrevious == nullptr;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    T await_resume() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return mValue.moveValue();
        }
    }
};
} // namespace co_async

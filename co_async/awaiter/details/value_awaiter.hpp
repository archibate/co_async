#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {
template <class T = void>
struct ValueAwaiter {
private:
    Avoid<T> mValue;

public:
    ValueAwaiter(Avoid<T> value) : mValue(std::move(value)) {}

    bool await_ready() const noexcept {
        return true;
    }

    void await_suspend(std::coroutine_handle<>) const noexcept {}

    T await_resume() {
        if constexpr (!std::is_void_v<T>) {
            return std::move(mValue);
        }
    }
};

template <class T = void>
struct ValueOrReturnAwaiter {
private:
    std::coroutine_handle<> mPrevious;
    Uninitialized<T> mValue;

public:
    template <class... Args>
    ValueOrReturnAwaiter(std::in_place_t, Args &&...args) : mPrevious() {
        mValue.emplace(std::forward<Args>(args)...);
    }

    ValueOrReturnAwaiter(std::coroutine_handle<> previous)
        : mPrevious(previous) {}

    bool await_ready() const noexcept {
        return !mPrevious;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    T await_resume() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return mValue.move();
        }
    }
};
} // namespace co_async

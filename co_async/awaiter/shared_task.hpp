#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/just.hpp>

namespace co_async {

template <class T = void, class P = Promise<T>>
struct [[nodiscard]] SharedTask {
    using promise_type = P;

    SharedTask(Task<T, P> &&task) noexcept {
#if CO_ASYNC_DEBUG
        mCoroutine = std::exchange(
            const_cast<std::coroutine_handle<P> &>(
                static_cast<std::coroutine_handle<P> const &>(task)),
            std::coroutine_handle<P>::from_address(nullptr));
#else
        mCoroutine = std::exchange(
            const_cast<std::coroutine_handle<P> &>(
                static_cast<std::coroutine_handle<P> const &>(task)),
            std::coroutine_handle<P>::from_address(
                std::noop_coroutine().address()));
#endif
    }

    SharedTask(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    SharedTask(SharedTask &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    SharedTask &operator=(SharedTask &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~SharedTask() {
        if (!mCoroutine)
            return;
#if CO_ASYNC_DEBUG
        if (!mCoroutine.done()) [[unlikely]] {
#if CO_ASYNC_PERF
            auto &perf = mCoroutine.promise().mPerf;
            std::cerr << "WARNING: task (" << perf.file << ":" << perf.line
                      << ") destroyed undone\n";
#else
            std::cerr << "WARNING: task destroyed undone\n";
#endif
        }
#endif
        mCoroutine.destroy();
    }

    auto operator co_await() const noexcept {
        return TaskAwaiter<T, P>(mCoroutine);
    }

    operator std::coroutine_handle<promise_type>() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> get() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> release() noexcept {
        return std::exchange(mCoroutine, nullptr);
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

} // namespace co_async

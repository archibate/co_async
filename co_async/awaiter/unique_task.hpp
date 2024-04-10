#pragma once /*{export module co_async:awaiter.unique_task;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/utils/uninitialized.hpp>/*{import :utils.uninitialized;}*/
#include <co_async/awaiter/task.hpp>       /*{import :awaiter.task;}*/
#include <co_async/awaiter/concepts.hpp>   /*{import :awaiter.concepts;}*/
#include <co_async/awaiter/just.hpp>   /*{import :awaiter.just;}*/

namespace co_async {

/*[export]*/ template <class T = void, class P = Promise<T>>
struct [[nodiscard]] UniqueTask {
    using promise_type = P;

    UniqueTask(Task<T, P> &&task) noexcept {
#if CO_ASYNC_DEBUG
        mCoroutine = std::exchange(
            const_cast<std::coroutine_handle<P> &>(
                static_cast<std::coroutine_handle<P> const &>(task)),
            std::coroutine_handle<P>::from_address(nullptr));
#else
        mCoroutine = std::exchange(
            const_cast<std::coroutine_handle<P> &>(
                static_cast<std::coroutine_handle<P> const &>(task)),
            std::coroutine_handle<P>::from_address(std::noop_coroutine().address()));
#endif
    }

    UniqueTask(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    UniqueTask(UniqueTask &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    UniqueTask &operator=(UniqueTask &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~UniqueTask() {
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

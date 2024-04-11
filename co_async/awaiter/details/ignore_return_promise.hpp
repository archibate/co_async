#pragma once /*{export module co_async:awaiter.details.ignore_return_promise;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

namespace co_async {

template <class FinalAwaiter = std::suspend_always>
struct IgnoreReturnPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void unhandled_exception() noexcept {
#if CO_ASYNC_EXCEPT
#if CO_ASYNC_DEBUG
        try {
            throw;
        } catch (std::exception const &e) {
            std::cerr
                << "WARNING: exception occurred in co_spawn'ed coroutine: "
                << e.what() << "\n";
        } catch (...) {
            std::cerr
                << "WARNING: exception occurred in co_spawn'ed coroutine\n";
        }
#endif
#else
        std::terminate();
#endif
    }

    void return_void() noexcept {}

    auto get_return_object() {
        return std::coroutine_handle<IgnoreReturnPromise>::from_promise(*this);
    }

    void setPrevious(std::coroutine_handle<>) noexcept {}

    IgnoreReturnPromise &operator=(IgnoreReturnPromise &&) = delete;
};

struct AutoDestroyFinalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        coroutine.destroy();
    }

    void await_resume() const noexcept {}
};

} // namespace co_async

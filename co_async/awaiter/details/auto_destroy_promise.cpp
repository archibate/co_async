export module co_async:awaiter.details.auto_destroy_promise;

import std;

namespace co_async {

struct AutoDestroyPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        std::coroutine_handle<AutoDestroyPromise>::from_promise(*this).destroy();
        return std::suspend_always();
    }

    void unhandled_exception() noexcept {
    }

    void return_void() noexcept {}

    auto get_return_object() {
        return std::coroutine_handle<AutoDestroyPromise>::from_promise(*this);
    }

    void setPrevious(std::coroutine_handle<>) noexcept {
    }

    AutoDestroyPromise &operator=(AutoDestroyPromise &&) = delete;
};

} // namespace co_async

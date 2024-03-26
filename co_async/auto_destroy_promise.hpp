#pragma once

#include <coroutine>
#include <co_async/task.hpp>

namespace co_async {

struct AutoDestroyPromise : Promise<void> {
    auto final_suspend() noexcept {
        auto ret = Promise<void>::final_suspend();
        std::coroutine_handle<AutoDestroyPromise>::from_promise(*this).destroy();
        return ret;
    }

    auto get_return_object() {
        return std::coroutine_handle<AutoDestroyPromise>::from_promise(*this);
    }
};

} // namespace co_async

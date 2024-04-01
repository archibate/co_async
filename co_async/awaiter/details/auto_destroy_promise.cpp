export module co_async:awaiter.details.auto_destroy_promise;

import std;
import :awaiter.task;

namespace co_async {

struct AutoDestroyPromise : Promise<void> {
    auto final_suspend() noexcept {
        auto ret = Promise<void>::final_suspend();
        std::coroutine_handle<AutoDestroyPromise>::from_promise(*this)
            .destroy();
        return ret;
    }

    auto get_return_object() {
        return std::coroutine_handle<AutoDestroyPromise>::from_promise(*this);
    }
};

} // namespace co_async

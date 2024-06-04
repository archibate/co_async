#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/io_context.hpp>

namespace co_async {
inline Task<Expected<>, GenericIOContext::TimerNode>
coSleep(std::chrono::steady_clock::time_point expires) {
    co_return co_await GenericIOContext::TimerNode::Awaiter(expires);
}

inline Task<Expected<>>
co_sleep(std::chrono::steady_clock::time_point expires) {
    co_return co_await (co_await co_cancel).guard(std::in_place_type<GenericIOContext::TimerNode::Canceller>,
        coSleep(expires));
}

inline Task<Expected<>, GenericIOContext::TimerNode>
coSleep(std::chrono::steady_clock::duration timeout) {
    return coSleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<Expected<>>
co_sleep(std::chrono::steady_clock::duration timeout) {
    return co_sleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<> coForever() {
    co_await std::suspend_always();
#if defined(__GNUC__) && defined(__has_builtin)
# if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
# endif
#endif
}

inline Task<> co_forever() {
    struct ForeverAwaiter {
        struct Canceller {
            using OpType = ForeverAwaiter;

            static Task<> doCancel(OpType *op) {
                co_spawn(op->mPrevious);
                co_return;
            }

            static void earlyCancelValue(OpType *op) noexcept {}
        };

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) noexcept {
            mPrevious = coroutine;
        }

        void await_resume() const noexcept {}

        std::coroutine_handle<> mPrevious;
    };

    co_return co_await (co_await co_cancel).guard(ForeverAwaiter());
}
} // namespace co_async

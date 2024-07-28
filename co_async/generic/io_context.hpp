#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {
struct IOContextOptions {
    std::chrono::steady_clock::duration maxSleep =
        std::chrono::milliseconds(200);
    std::optional<std::size_t> threadAffinity = std::nullopt;
    std::size_t queueEntries = 512;
};

struct alignas(hardware_destructive_interference_size) IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::chrono::steady_clock::duration mMaxSleep;

public:
    explicit IOContext(IOContextOptions options = {});
    IOContext(IOContext &&) = delete;
    ~IOContext();

    [[gnu::hot]] void run();
    [[gnu::hot]] bool runOnce();

    // [[gnu::hot]] void spawn(std::coroutine_handle<> coroutine) {
    //     coroutine.resume();
    // }
    //
    // template <class T, class P>
    // void spawn(Task<T, P> task) {
    //     auto wrapped = coSpawnStarter(std::move(task));
    //     wrapped.get().resume();
    //     wrapped.release();
    // }
    //
    // template <class T, class P>
    // T join(Task<T, P> task) {
    //     return contextJoin(*this, std::move(task));
    // }

    static thread_local IOContext *instance;
};

inline Task<> co_catch(Task<Expected<>> task) {
    auto ret = co_await task;
    if (ret.has_error()) {
        std::cerr << ret.error().category().name() << " error: " << ret.error().message() << " (" << ret.error().value() << ")\n";
    }
    co_return;
}

inline void co_main(Task<Expected<>> main) {
    IOContext ctx;
    co_spawn(co_catch(std::move(main)));
    ctx.run();
}

inline void co_main(Task<> main) {
    IOContext ctx;
    co_spawn(std::move(main));
    ctx.run();
}

// template <class T, class P>
// inline Task<> contextJoinHelper(Task<T, P> task, std::condition_variable &cv,
//                                 Uninitialized<T> &result
// #if CO_ASYNC_EXCEPT
//                                 ,
//                                 std::exception_ptr exception
// #endif
// ) {
// #if CO_ASYNC_EXCEPT
//     try {
// #endif
//         result.emplace((co_await task, Void()));
// #if CO_ASYNC_EXCEPT
//     } catch (...) {
// # if CO_ASYNC_DEBUG
//         std::cerr << "WARNING: exception occurred in IOContext::join\n";
// # endif
//         exception = std::current_exception();
//     }
// #endif
//     cv.notify_one();
// }
//
// template <class T, class P>
// T contextJoin(IOContext &context, Task<T, P> task) {
//     std::condition_variable cv;
//     std::mutex mtx;
//     Uninitialized<T> result;
// #if CO_ASYNC_EXCEPT
//     std::exception_ptr exception;
// #endif
//     context.spawn(contextJoinHelper(std::move(task), cv, result
// #if CO_ASYNC_EXCEPT
//                                     ,
//                                     exception
// #endif
//                                     ));
//     std::unique_lock lck(mtx);
//     cv.wait(lck);
//     lck.unlock();
// #if CO_ASYNC_EXCEPT
//     if (exception) [[unlikely]] {
//         std::rethrow_exception(exception);
//     }
// #endif
//     if constexpr (!std::is_void_v<T>) {
//         return result.move();
//     }
// }
//
// inline auto co_resume_on(IOContext &context) {
//     struct ResumeOnAwaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             mContext.spawn(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         IOContext &mContext;
//     };
//
//     return ResumeOnAwaiter(context);
// }
} // namespace co_async

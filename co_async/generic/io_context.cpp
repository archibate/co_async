#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/platform/futex.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {

IOContext::IOContext(IOContextOptions options) {
    if (instance) {
        throw std::logic_error("each thread may create only one IOContext");
    }
    instance = this;
    GenericIOContext::instance = &mGenericIO;
    PlatformIOContext::instance = &mPlatformIO;
    if (options.threadAffinity) {
        PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
    }
    mPlatformIO.setup(options.queueEntries);
    mMaxSleep = options.maxSleep;
}

IOContext::~IOContext() {
    IOContext::instance = nullptr;
    GenericIOContext::instance = nullptr;
    PlatformIOContext::instance = nullptr;
}

void IOContext::run() {
    while (runOnce())
        ;
}

bool IOContext::runOnce() {
    auto duration = mGenericIO.runDuration();
    if (!duration && !mPlatformIO.hasPendingEvents()) [[unlikely]] {
        return false;
    }
    if (!duration || *duration > mMaxSleep) {
        duration = mMaxSleep;
    }
    mPlatformIO.waitEventsFor(duration);
    return true;
}

thread_local IOContext *IOContext::instance;

// void IOContext::wakeUp() {
//     if (mWake.fetch_add(1, std::memory_order_relaxed) == 0)
//         futex_notify_sync(&mWake, 1);
// }
//
// Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
// IOContext::watchDogTask() {
//     // helps wake up main loop when IOContext::spawn called
//     while (true) {
//         while (mWake.load(std::memory_order_relaxed) == 0)
//             (void)co_await futex_wait(&mWake, 0);
//         mWake.store(0, std::memory_order_relaxed);
//     }
// }

} // namespace co_async

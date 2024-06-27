#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/futex.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {

struct IOContext::IOContextGuard {
    explicit IOContextGuard(IOContext *that) {
        if (IOContext::instance || GenericIOContext::instance ||
            PlatformIOContext::instance) [[unlikely]] {
            throw std::logic_error(
                "each thread may contain only one IOContextGuard");
        }
        IOContext::instance = that;
        GenericIOContext::instance = &that->mGenericIO;
        PlatformIOContext::instance = &that->mPlatformIO;
    }

    ~IOContextGuard() {
        IOContext::instance = nullptr;
        GenericIOContext::instance = nullptr;
        PlatformIOContext::instance = nullptr;
    }

    IOContextGuard(IOContextGuard &&) = delete;
};

void IOContext::startHere(std::stop_token stop,
                          IOContextOptions options,
                          std::span<IOContext> peerContexts) {
    IOContextGuard guard(this);

    // genericIO->enqueueJob(watchDogTask().release());

    while (!stop.stop_requested()) [[likely]] {
        auto duration = mGenericIO.runDuration();
        if (!duration || *duration > options.maxSleep) {
            duration = options.maxSleep;
        }
#if !CO_ASYNC_STEAL
        mPlatformIO.waitEventsFor(duration);
#else
        bool hasEvent = mPlatformIO.waitEventsFor(duration);
        if (!hasEvent && !peerContexts.empty()) {
            for (IOContext *p = peerContexts.data();
                 p != peerContexts.data() + peerContexts.size(); ++p) {
                if (p->mGenericIO.runComputeOnly()) {
                    break;
                }
            }
        }
#endif
    }
}

void IOContext::start(IOContextOptions options,
                      std::span<IOContext> peerContexts) {
    mThread = std::jthread([this, options = std::move(options),
                            peerContexts](std::stop_token stop) {
        mPlatformIO.setup(options.queueEntries);
        if (options.threadAffinity) {
            PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
        }
        this->startHere(stop, options, peerContexts);
    });
}

thread_local IOContext *IOContext::instance;

// void IOContext::wakeUp() {
//     if (mWake.fetch_add(1, std::memory_order_relaxed) == 0)
//         futex_notify_sync(&mWake, 1);
// }
//
// Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>> IOContext::watchDogTask() {
//     // helps wake up main loop when IOContext::spawn called
//     while (true) {
//         while (mWake.load(std::memory_order_relaxed) == 0)
//             (void)co_await futex_wait(&mWake, 0);
//         mWake.store(0, std::memory_order_relaxed);
//     }
// }

} // namespace co_async

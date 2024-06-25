#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
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
                          PlatformIOContextOptions options,
                          std::span<IOContext> peerContexts) {
    IOContextGuard guard(this);
    if (options.threadAffinity) {
        PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
    }
    auto maxSleep = options.maxSleep;
    auto *genericIO = GenericIOContext::instance;
    auto *platformIO = PlatformIOContext::instance;
    while (!stop.stop_requested()) [[likely]] {
        auto duration = genericIO->runDuration();
        if (!duration || *duration > maxSleep) {
            duration = maxSleep;
        }
        bool hasEvent = platformIO->waitEventsFor(1, duration);
        if (hasEvent) {
            auto t = maxSleep + options.maxSleepInc;
            if (t > options.maxSleepLimit) {
                t = options.maxSleepLimit;
            }
            maxSleep = t;
        } else {
            maxSleep = options.maxSleep;
        }
#if CO_ASYNC_STEAL
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

void IOContext::start(PlatformIOContextOptions options,
                      std::span<IOContext> peerContexts) {
    mThread = std::jthread([this, options = std::move(options),
                            peerContexts](std::stop_token stop) {
        this->startHere(stop, options, peerContexts);
    });
}

thread_local IOContext *IOContext::instance;
} // namespace co_async

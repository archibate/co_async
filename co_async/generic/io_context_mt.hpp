#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {
struct IOContextMT {
private:
    std::unique_ptr<IOContext[]> mWorkers;
    std::size_t mNumWorkers = 0;

public:
    explicit IOContextMT(std::in_place_t) {
        if (IOContextMT::instance) [[unlikely]] {
            throw std::logic_error(
                "each process may contain only one IOContextMT");
        }
        IOContextMT::instance = this;
    }

    ~IOContextMT() {
        IOContextMT::instance = nullptr;
    }

    explicit IOContextMT(PlatformIOContextOptions options = {},
                         std::size_t numWorkers = 0)
        : IOContextMT(std::in_place) {
        start(options, numWorkers);
    }

    static std::size_t get_worker_id(IOContext const &context) noexcept {
        return static_cast<std::size_t>(&context - instance->mWorkers.get());
    }

    static std::size_t this_worker_id() noexcept {
        return get_worker_id(*IOContext::instance);
    }

    static IOContext &nth_worker(std::size_t index) noexcept {
        return instance->mWorkers[index];
    }

    static std::size_t num_workers() noexcept {
        return instance->mNumWorkers;
    }

    static void start(PlatformIOContextOptions options = {},
                      std::size_t numWorkers = 0) {
        bool setAffinity;
        if (numWorkers == 0) {
            setAffinity = true;
            numWorkers = std::thread::hardware_concurrency();
            if (!numWorkers) [[unlikely]] throw std::logic_error(
                "failed to detect number of hardware threads");
        } else {
            setAffinity = false;
        }
        instance->mWorkers = std::make_unique<IOContext[]>(numWorkers);
        instance->mNumWorkers = numWorkers;
        std::span<IOContext> peerSpan(instance->mWorkers.get(),
                                      instance->mNumWorkers);
        for (std::size_t i = 0; i < instance->mNumWorkers; ++i) {
            if (setAffinity) {
                options.threadAffinity = i;
            }
            instance->mWorkers[i].start(options, peerSpan);
        }
    }

    static void spawn(std::coroutine_handle<> coroutine) {
        instance->mWorkers[0].spawn(coroutine);
    }

    static void spawn_mt(std::coroutine_handle<> coroutine) {
        instance->mWorkers[0].spawn_mt(coroutine);
    }

    template <class T, class P>
    static T join(Task<T, P> task) {
        return instance->mWorkers[0].join(std::move(task));
    }

    static inline IOContextMT *instance;
};
} // namespace co_async

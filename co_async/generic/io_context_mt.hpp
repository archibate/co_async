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
    IOContextMT();
    IOContextMT(IOContext &&) = delete;
    ~IOContextMT();

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

    static void run(std::size_t numWorkers = 0);

    // static void spawn(std::coroutine_handle<> coroutine,
    //                   std::size_t index = 0) {
    //     instance->mWorkers[index].spawn(coroutine);
    // }
    //
    // template <class T, class P>
    // static T join(Task<T, P> task, std::size_t index = 0) {
    //     return instance->mWorkers[index].join(std::move(task));
    // }

    static IOContextMT *instance;
};
} // namespace co_async

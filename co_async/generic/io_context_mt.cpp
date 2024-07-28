#include <co_async/std.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/generic/io_context_mt.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/cacheline.hpp>

namespace co_async {
IOContextMT::IOContextMT() {
    if (IOContextMT::instance) [[unlikely]] {
        throw std::logic_error("each process may contain only one IOContextMT");
    }
    IOContextMT::instance = this;
}

IOContextMT::~IOContextMT() {
    IOContextMT::instance = nullptr;
}

void IOContextMT::run(std::size_t numWorkers) {
    // if (numWorkers == 0) {
    //     setAffinity = true;
    //     numWorkers = std::thread::hardware_concurrency();
    //     if (!numWorkers) [[unlikely]] {
    //         throw std::logic_error(
    //             "failed to detect number of hardware threads");
    //     }
    // } else {
    //     setAffinity = false;
    // }
    instance->mWorkers = std::make_unique<IOContext[]>(numWorkers);
    instance->mNumWorkers = numWorkers;
    for (std::size_t i = 0; i < instance->mNumWorkers; ++i) {
        instance->mWorkers[i].run();
    }
}

IOContextMT *IOContextMT::instance;
} // namespace co_async

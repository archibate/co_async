#include <co_async/awaiter/task.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/platform_io.hpp>
#include <fcntl.h>
#include <liburing.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
void PlatformIOContext::schedSetThreadAffinity(size_t cpu) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    throwingErrorErrno(
        sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpu_set));
}

bool PlatformIOContext::ioUringIsOpCodeSupported(int op) noexcept {
    struct io_uring_probe *probe = io_uring_get_probe();
    if (!probe) [[unlikely]]
        return false;

    // Available since 6.0.
    bool res = io_uring_opcode_supported(probe, op);
    io_uring_free_probe(probe);
    return res;
}

PlatformIOContext::PlatformIOContext(std::size_t entries) {
    unsigned int flags = IORING_SETUP_SQPOLL;
    throwingError(io_uring_queue_init((unsigned int)entries, &mRing, flags));
}

PlatformIOContext::~PlatformIOContext() {
    io_uring_queue_exit(&mRing);
}

thread_local PlatformIOContext *PlatformIOContext::instance;

FILE *out = fopen("/tmp/rec.txt", "wb");

bool PlatformIOContext::waitEventsFor(std::optional<std::chrono::steady_clock::duration> timeout) {
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    unsigned int numBatch = io_uring_cq_ready(&mRing);
    if (numBatch == 0) {
        numBatch = 1;
    } else {
        fprintf(out, "%d\n", numBatch);
    }
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(
        &mRing, &cqe, numBatch, tsp, nullptr);
    if (res == -ETIME) {
        return false;
    } else if (res < 0) [[unlikely]] {
        if (res == -EINTR) {
            return false;
        }
        throw std::system_error(-res, std::system_category());
    }
    unsigned head, numGot = 0;
    io_uring_for_each_cqe(&mRing, head, cqe) {
        auto *op = reinterpret_cast<UringOp *>(cqe->user_data);
        op->mRes = cqe->res;
        GenericIOContext::instance->enqueueJob(op->mPrevious);
        ++numGot;
    }
    io_uring_cq_advance(&mRing, numGot);
    return true;
}

// bool PlatformIOContext::pollEvents(std::optional<std::chrono::steady_clock::duration> timeout) {
//     struct io_uring_cqe *cqe;
//     struct __kernel_timespec ts, *tsp;
//     unsigned int numBatch = io_uring_cq_ready(&mRing);
//     if (numBatch == 0) {
//         numBatch = 1;
//     } else {
//         fprintf(out, "%d\n", numBatch);
//     }
//     if (timeout) {
//         tsp = &(ts = durationToKernelTimespec(*timeout));
//     } else {
//         tsp = nullptr;
//     }
//     int res = io_uring_submit(&mRing);
//     if (res == -ETIME) {
//         return false;
//     } else if (res < 0) [[unlikely]] {
//         if (res == -EINTR) {
//             return false;
//         }
//         throw std::system_error(-res, std::system_category());
//     }
//     unsigned head, numGot = 0;
//     io_uring_for_each_cqe(&mRing, head, cqe) {
//         auto *op = reinterpret_cast<UringOp *>(cqe->user_data);
//         op->mRes = cqe->res;
//         GenericIOContext::instance->enqueueJob(op->mPrevious);
//         ++numGot;
//     }
//     io_uring_cq_advance(&mRing, numGot);
//     return true;
// }

} // namespace co_async

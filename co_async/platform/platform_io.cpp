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

PlatformIOContext::PlatformIOContext(std::size_t entries) {
    throwingError(io_uring_queue_init((unsigned int)entries, &mRing, 0));
}

PlatformIOContext::~PlatformIOContext() {
    io_uring_queue_exit(&mRing);
}

thread_local PlatformIOContext *PlatformIOContext::instance;

bool PlatformIOContext::waitEventsFor(
    std::size_t numBatch,
    std::optional<std::chrono::steady_clock::duration> timeout) {
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(
        &mRing, &cqe, (unsigned int)numBatch, tsp, nullptr);
    if (res == -EINTR) [[unlikely]] {
        return false;
    }
    if (res == -ETIME) {
        return false;
    }
    throwingError(res);
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
} // namespace co_async

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
    if (!probe) [[unlikely]] {
        return false;
    }
    bool res = io_uring_opcode_supported(probe, op);
    io_uring_free_probe(probe);
    return res;
}

void PlatformIOContext::dumpIOUringDiagnostics() {
    struct io_uring_probe *probe = io_uring_get_probe();
    if (!probe) [[unlikely]] {
        return;
    }
    for (int op = IORING_OP_NOP; op < IORING_OP_LAST; ++op) {
        bool ok = io_uring_opcode_supported(probe, op);
        std::cerr << "opcode " << op << (ok ? "" : " not") << " supported"
                  << '\n';
    }
    io_uring_free_probe(probe);
}

PlatformIOContext::PlatformIOContext() noexcept {
    mRing.ring_fd = -1;
}

void PlatformIOContext::setup(std::size_t entries) {
    struct io_uring_params params = {};
    // params.flags = IORING_SETUP_SQPOLL;
    // params.sq_thread_cpu = 1;
    // params.sq_thread_idle = 1000;
    throwingError(
        io_uring_queue_init_params((unsigned int)entries, &mRing, &params));
}

PlatformIOContext::~PlatformIOContext() {
    if (mRing.ring_fd != -1) {
        io_uring_queue_exit(&mRing);
    }
}

thread_local PlatformIOContext *PlatformIOContext::instance;

bool PlatformIOContext::waitEventsFor(
    std::optional<std::chrono::steady_clock::duration> timeout) {
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(&mRing, &cqe, 1, tsp, nullptr);
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
} // namespace co_async

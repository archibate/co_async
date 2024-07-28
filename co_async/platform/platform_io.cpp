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

PlatformIOContext::IOUringProbe::IOUringProbe() {
    mRing = nullptr;
    // mProbe = io_uring_get_probe();
    mProbe = nullptr;
    if (!mProbe) {
        mRing = new struct io_uring;
        throwingError(io_uring_queue_init(8, mRing, 0));
    }
}

PlatformIOContext::IOUringProbe::~IOUringProbe() {
    if (mProbe) {
        io_uring_free_probe(mProbe);
    }
    if (mRing) {
        io_uring_queue_exit(mRing);
        delete mRing;
    }
}

bool PlatformIOContext::IOUringProbe::isSupported(int op) noexcept {
    if (mProbe) {
        return io_uring_opcode_supported(mProbe, op);
    }
    if (mRing) {
        struct io_uring_sqe *sqe = io_uring_get_sqe(mRing);
        io_uring_prep_rw(op, sqe, -1, nullptr, 0, 0);
        struct io_uring_cqe *cqe;
        throwingError(io_uring_submit(mRing));
        throwingError(io_uring_wait_cqe(mRing, &cqe));
        int res = cqe->res;
        io_uring_cqe_seen(mRing, cqe);
        return res != ENOSYS;
    }
    return false;
}

void PlatformIOContext::IOUringProbe::dumpDiagnostics() {
    static char const *ops[IORING_OP_LAST + 1] = {
        "IORING_OP_NOP",
        "IORING_OP_READV",
        "IORING_OP_WRITEV",
        "IORING_OP_FSYNC",
        "IORING_OP_READ_FIXED",
        "IORING_OP_WRITE_FIXED",
        "IORING_OP_POLL_ADD",
        "IORING_OP_POLL_REMOVE",
        "IORING_OP_SYNC_FILE_RANGE",
        "IORING_OP_SENDMSG",
        "IORING_OP_RECVMSG",
        "IORING_OP_TIMEOUT",
        "IORING_OP_TIMEOUT_REMOVE",
        "IORING_OP_ACCEPT",
        "IORING_OP_ASYNC_CANCEL",
        "IORING_OP_LINK_TIMEOUT",
        "IORING_OP_CONNECT",
        "IORING_OP_FALLOCATE",
        "IORING_OP_OPENAT",
        "IORING_OP_CLOSE",
        "IORING_OP_FILES_UPDATE",
        "IORING_OP_STATX",
        "IORING_OP_READ",
        "IORING_OP_WRITE",
        "IORING_OP_FADVISE",
        "IORING_OP_MADVISE",
        "IORING_OP_SEND",
        "IORING_OP_RECV",
        "IORING_OP_OPENAT2",
        "IORING_OP_EPOLL_CTL",
        "IORING_OP_SPLICE",
        "IORING_OP_PROVIDE_BUFFERS",
        "IORING_OP_REMOVE_BUFFERS",
        "IORING_OP_TEE",
        "IORING_OP_SHUTDOWN",
        "IORING_OP_RENAMEAT",
        "IORING_OP_UNLINKAT",
        "IORING_OP_MKDIRAT",
        "IORING_OP_SYMLINKAT",
        "IORING_OP_LINKAT",
        "IORING_OP_MSG_RING",
        "IORING_OP_FSETXATTR",
        "IORING_OP_SETXATTR",
        "IORING_OP_FGETXATTR",
        "IORING_OP_GETXATTR",
        "IORING_OP_SOCKET",
        "IORING_OP_URING_CMD",
        "IORING_OP_SEND_ZC",
        "IORING_OP_SENDMSG_ZC",
        "IORING_OP_READ_MULTISHOT",
        "IORING_OP_WAITID",
        "IORING_OP_FUTEX_WAIT",
        "IORING_OP_FUTEX_WAKE",
        "IORING_OP_FUTEX_WAITV",
        "IORING_OP_FIXED_FD_INSTALL",
        "IORING_OP_FTRUNCATE",
        "IORING_OP_LAST",
    };
    for (int op = IORING_OP_NOP; op < IORING_OP_LAST; ++op) {
        bool ok = isSupported(op);
        std::cerr << "opcode " << ops[op] << (ok ? "" : " not") << " supported"
                  << '\n';
    }
}

PlatformIOContext::PlatformIOContext() noexcept {
    mRing.ring_fd = -1;
}

void PlatformIOContext::setup(std::size_t entries) {
    unsigned int flags = 0;
#if CO_ASYNC_DIRECT
    flags |= IORING_SETUP_IOPOLL;
#endif
    throwingError(
        io_uring_queue_init(static_cast<unsigned int>(entries), &mRing, flags));
}

void PlatformIOContext::reserveBuffers(std::size_t nbufs) {
    auto oldBuf = std::move(mBuffers);
    mBuffers = std::make_unique<struct iovec[]>(nbufs);
    if (mCapBufs) {
        throwingError(io_uring_unregister_buffers(&mRing));
    }
    mCapBufs = static_cast<unsigned int>(nbufs);
    std::memcpy(mBuffers.get(), oldBuf.get(), sizeof(struct iovec) * mNumBufs);
    throwingError(io_uring_register_buffers_sparse(
        &mRing, static_cast<unsigned int>(nbufs)));
    std::vector<__u64> tags(mNumBufs, 0);
    throwingError(io_uring_register_buffers_update_tag(
        &mRing, 0, mBuffers.get(), tags.data(),
        static_cast<unsigned int>(mNumBufs)));
}

std::size_t
PlatformIOContext::addBuffers(std::span<std::span<char> const> bufs) {
    if (mNumBufs >= mCapBufs) {
        reserveBuffers(mCapBufs * 2 + 1);
    }
    auto outP = mBuffers.get() + mNumBufs;
    for (auto const &buf: bufs) {
        struct iovec iov;
        iov.iov_base = buf.data();
        iov.iov_len = buf.size();
        *outP++ = iov;
    }
    std::vector<__u64> tags(bufs.size(), 0);
    throwingError(io_uring_register_buffers_update_tag(
        &mRing, mNumBufs, mBuffers.get() + mNumBufs, tags.data(),
        static_cast<unsigned int>(bufs.size())));
    size_t ret = mNumBufs;
    mNumBufs += static_cast<unsigned int>(bufs.size());
    return ret;
}

void PlatformIOContext::reserveFiles(std::size_t nfiles) {
    auto oldBuf = std::move(mBuffers);
    mBuffers = std::make_unique<struct iovec[]>(nfiles);
    if (mCapFiles) {
        throwingError(io_uring_unregister_files(&mRing));
    }
    mCapFiles = static_cast<unsigned int>(nfiles);
    std::memcpy(mBuffers.get(), oldBuf.get(), sizeof(struct iovec) * mNumBufs);
    throwingError(io_uring_register_files_sparse(
        &mRing, static_cast<unsigned int>(nfiles)));
    std::vector<__u64> tags(mNumFiles, 0);
    throwingError(io_uring_register_files_update_tag(&mRing, 0, mFiles.get(),
                                                     tags.data(), mNumFiles));
}

std::size_t PlatformIOContext::addFiles(std::span<int const> files) {
    if (mNumFiles >= mCapFiles) {
        reserveBuffers(mCapFiles * 2 + 1);
    }
    auto outP = mFiles.get() + mNumFiles;
    for (auto const &file: files) {
        *outP++ = file;
    }
    std::vector<__u64> tags(files.size(), 0);
    throwingError(io_uring_register_files_update_tag(
        &mRing, mNumFiles, mFiles.get() + mNumFiles, tags.data(),
        static_cast<unsigned int>(files.size())));
    size_t ret = mNumFiles;
    mNumFiles += static_cast<unsigned int>(files.size());
    return ret;
}

PlatformIOContext::~PlatformIOContext() {
    if (mRing.ring_fd != -1) {
        io_uring_queue_exit(&mRing);
    }
}

thread_local PlatformIOContext *PlatformIOContext::instance;

bool PlatformIOContext::waitEventsFor(
    std::optional<std::chrono::steady_clock::duration> timeout) {
    // debug(), "wait", this, mNumSqesPending;
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
    std::vector<std::coroutine_handle<>> tasks;
    io_uring_for_each_cqe(&mRing, head, cqe) {
#if CO_ASYNC_INVALFIX
        if (cqe->user_data == LIBURING_UDATA_TIMEOUT) [[unlikely]] {
            ++numGot;
            continue;
        }
#endif
        auto *op = reinterpret_cast<UringOp *>(cqe->user_data);
        op->mRes = cqe->res;
        tasks.push_back(op->mPrevious);
        ++numGot;
    }
    io_uring_cq_advance(&mRing, numGot);
    mNumSqesPending -= static_cast<std::size_t>(numGot);
    for (auto const &task: tasks) {
#if CO_ASYNC_DEBUG
        if (!task) [[likely]] {
            std::cerr << "null coroutine pushed into task queue\n";
        }
        if (task.done()) [[likely]] {
            std::cerr << "done coroutine pushed into task queue\n";
        }
#endif
        task.resume();
    }
    return true;
}
} // namespace co_async

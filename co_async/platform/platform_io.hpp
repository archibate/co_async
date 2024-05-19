#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/generic_io.hpp>
#include <co_async/platform/error_handling.hpp>
#include <fcntl.h>
#include <liburing.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
template <class Rep, class Period>
struct __kernel_timespec
durationToKernelTimespec(std::chrono::duration<Rep, Period> dur) {
    struct __kernel_timespec ts;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);
    ts.tv_sec = static_cast<__kernel_time64_t>(secs.count());
    ts.tv_nsec = static_cast<__kernel_time64_t>(nsecs.count());
    return ts;
}

template <class Clk, class Dur>
struct __kernel_timespec
timePointToKernelTimespec(std::chrono::time_point<Clk, Dur> tp) {
    return durationToKernelTimespec(tp.time_since_epoch());
}

struct PlatformIOContextOptions {
    std::chrono::steady_clock::duration maxSleep =
        std::chrono::milliseconds(5);
    std::chrono::steady_clock::duration maxSleepInc =
        std::chrono::milliseconds(8);
    std::chrono::steady_clock::duration maxSleepLimit =
        std::chrono::milliseconds(100);
    std::optional<std::size_t> threadAffinity = std::nullopt;
};

struct PlatformIOContext {
    static void schedSetThreadAffinity(size_t cpu);
    bool
    waitEventsFor(std::size_t numBatch,
                  std::optional<std::chrono::steady_clock::duration> timeout);

    std::size_t pendingEventCount() const {
        return io_uring_cq_ready(&mRing);
    }

    struct io_uring *getRing() {
        return &mRing;
    }

    PlatformIOContext &operator=(PlatformIOContext &&) = delete;
    explicit PlatformIOContext(std::size_t entries = 512);
    ~PlatformIOContext();
    static thread_local PlatformIOContext *instance;

private:
    struct io_uring mRing;
};

struct [[nodiscard]] UringOp {
    UringOp() {
        struct io_uring *ring = PlatformIOContext::instance->getRing();
        mSqe = io_uring_get_sqe(ring);
        if (!mSqe) [[unlikely]] {
            throw std::bad_alloc();
        }
        io_uring_sqe_set_data(mSqe, this);
    }

    UringOp(UringOp &&) = delete;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) {
            mOp->mPrevious = coroutine;
            mOp->mRes = -ENOSYS;
        }

        int await_resume() const noexcept {
            return mOp->mRes;
        }

        UringOp *mOp;
    };

    Awaiter operator co_await() {
        return Awaiter{this};
    }

    static UringOp &link_ops(UringOp &&lhs, UringOp &&rhs) {
        lhs.mSqe->flags |= IOSQE_IO_LINK;
        rhs.mPrevious = std::noop_coroutine();
        return lhs;
    }

    struct io_uring_sqe *getSqe() const noexcept {
        return mSqe;
    }

private:
    std::coroutine_handle<> mPrevious;

    union {
        int mRes;
        struct io_uring_sqe *mSqe;
    };

    friend PlatformIOContext;

public:
    UringOp &&prep_nop() && {
        io_uring_prep_nop(mSqe);
        return std::move(*this);
    }

    UringOp &&prep_openat(int dirfd, char const *path, int flags,
                          mode_t mode) && {
        io_uring_prep_openat(mSqe, dirfd, path, flags, mode);
        return std::move(*this);
    }

    UringOp &&prep_openat_direct(int dirfd, char const *path, int flags,
                                 mode_t mode, unsigned int file_index) && {
        io_uring_prep_openat_direct(mSqe, dirfd, path, flags, mode, file_index);
        return std::move(*this);
    }

    UringOp &&prep_socket(int domain, int type, int protocol,
                          unsigned int flags) && {
        io_uring_prep_socket(mSqe, domain, type, protocol, flags);
        return std::move(*this);
    }

    UringOp &&prep_accept(int fd, struct sockaddr *addr, socklen_t *addrlen,
                          int flags) && {
        io_uring_prep_accept(mSqe, fd, addr, addrlen, flags);
        return std::move(*this);
    }

    UringOp &&prep_connect(int fd, const struct sockaddr *addr,
                           socklen_t addrlen) && {
        io_uring_prep_connect(mSqe, fd, addr, addrlen);
        return std::move(*this);
    }

    UringOp &&prep_mkdirat(int dirfd, char const *path, mode_t mode) && {
        io_uring_prep_mkdirat(mSqe, dirfd, path, mode);
        return std::move(*this);
    }

    UringOp &&prep_linkat(int olddirfd, char const *oldpath, int newdirfd,
                          char const *newpath, int flags) && {
        io_uring_prep_linkat(mSqe, olddirfd, oldpath, newdirfd, newpath, flags);
        return std::move(*this);
    }

    UringOp &&prep_renameat(int olddirfd, char const *oldpath, int newdirfd,
                            char const *newpath, unsigned int flags) && {
        io_uring_prep_renameat(mSqe, olddirfd, oldpath, newdirfd, newpath,
                               flags);
        return std::move(*this);
    }

    UringOp &&prep_unlinkat(int dirfd, char const *path, int flags = 0) && {
        io_uring_prep_unlinkat(mSqe, dirfd, path, flags);
        return std::move(*this);
    }

    UringOp &&prep_symlinkat(char const *target, int newdirfd,
                             char const *linkpath) && {
        io_uring_prep_symlinkat(mSqe, target, newdirfd, linkpath);
        return std::move(*this);
    }

    UringOp &&prep_statx(int dirfd, char const *path, int flags,
                         unsigned int mask, struct statx *statxbuf) && {
        io_uring_prep_statx(mSqe, dirfd, path, flags, mask, statxbuf);
        return std::move(*this);
    }

    UringOp &&prep_read(int fd, std::span<char> buf, std::uint64_t offset) && {
        io_uring_prep_read(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                           offset);
        return std::move(*this);
    }

    UringOp &&prep_write(int fd, std::span<char const> buf,
                         std::uint64_t offset) && {
        io_uring_prep_write(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                            offset);
        return std::move(*this);
    }

    UringOp &&prep_read_fixed(int fd, std::span<char> buf, std::uint64_t offset,
                              int buf_index) && {
        io_uring_prep_read_fixed(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                                 offset, buf_index);
        return std::move(*this);
    }

    UringOp &&prep_write_fixed(int fd, std::span<char const> buf,
                               std::uint64_t offset, int buf_index) && {
        io_uring_prep_write_fixed(mSqe, fd, buf.data(),
                                  (unsigned int)buf.size(), offset, buf_index);
        return std::move(*this);
    }

    UringOp &&prep_readv(int fd, std::span<struct iovec const> buf,
                         std::uint64_t offset, int flags) && {
        io_uring_prep_readv2(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                             offset, flags);
        return std::move(*this);
    }

    UringOp &&prep_writev(int fd, std::span<struct iovec const> buf,
                          std::uint64_t offset, int flags) && {
        io_uring_prep_writev2(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                              offset, flags);
        return std::move(*this);
    }

    UringOp &&prep_recv(int fd, std::span<char> buf, int flags) && {
        io_uring_prep_recv(mSqe, fd, buf.data(), buf.size(), flags);
        return std::move(*this);
    }

    UringOp &&prep_send(int fd, std::span<char const> buf, int flags) && {
        io_uring_prep_send(mSqe, fd, buf.data(), buf.size(), flags);
        return std::move(*this);
    }

    UringOp &&prep_recvmsg(int fd, struct msghdr *msg, unsigned int flags) && {
        io_uring_prep_recvmsg(mSqe, fd, msg, flags);
        return std::move(*this);
    }

    UringOp &&prep_sendmsg(int fd, struct msghdr *msg, unsigned int flags) && {
        io_uring_prep_sendmsg(mSqe, fd, msg, flags);
        return std::move(*this);
    }

    UringOp &&prep_close(int fd) && {
        io_uring_prep_close(mSqe, fd);
        return std::move(*this);
    }

    UringOp &&prep_shutdown(int fd, int how) && {
        io_uring_prep_shutdown(mSqe, fd, how);
        return std::move(*this);
    }

    UringOp &&prep_fsync(int fd, unsigned int flags) && {
        io_uring_prep_fsync(mSqe, fd, flags);
        return std::move(*this);
    }

    UringOp &&prep_ftruncate(int fd, loff_t len) && {
        io_uring_prep_ftruncate(mSqe, fd, len);
        return std::move(*this);
    }

    UringOp &&prep_cancel(UringOp *op, int flags) && {
        io_uring_prep_cancel(mSqe, op, flags);
        return std::move(*this);
    }

    UringOp &&prep_cancel_fd(int fd, unsigned int flags) && {
        io_uring_prep_cancel_fd(mSqe, fd, flags);
        return std::move(*this);
    }

    UringOp &&prep_waitid(idtype_t idtype, id_t id, siginfo_t *infop,
                          int options, unsigned int flags) && {
        io_uring_prep_waitid(mSqe, idtype, id, infop, options, flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout(struct __kernel_timespec *ts, unsigned int count,
                           unsigned int flags) && {
        io_uring_prep_timeout(mSqe, ts, count, flags);
        return std::move(*this);
    }

    UringOp &&prep_link_timeout(struct __kernel_timespec *ts,
                                unsigned int flags) && {
        io_uring_prep_link_timeout(mSqe, ts, flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout_update(UringOp *op, struct __kernel_timespec *ts,
                                  unsigned int flags) && {
        io_uring_prep_timeout_update(
            mSqe, ts, reinterpret_cast<std::uintptr_t>(op), flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout_remove(UringOp *op, unsigned int flags) && {
        io_uring_prep_timeout_remove(mSqe, reinterpret_cast<std::uintptr_t>(op),
                                     flags);
        return std::move(*this);
    }

    UringOp &&prep_splice(int fd_in, std::int64_t off_in, int fd_out,
                          std::int64_t off_out, std::size_t nbytes,
                          unsigned int flags) && {
        io_uring_prep_splice(mSqe, fd_in, off_in, fd_out, off_out,
                             (unsigned int)nbytes, flags);
        return std::move(*this);
    }
};

struct UringOpCanceller {
    using OpType = UringOp;

    static Task<> doCancel(OpType *op) {
        co_await UringOp().prep_cancel(op, IORING_ASYNC_CANCEL_ALL);
    }

    static int earlyCancelValue(OpType *op) noexcept {
        return -ECANCELED;
    }
};
} // namespace co_async

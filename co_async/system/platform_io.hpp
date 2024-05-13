#pragma once

#include <co_async/std.hpp>
#include <co_async/threading/generic_io.hpp>

#ifdef __linux__

#include <liburing.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
/* #include <signal.h> */
#include <co_async/system/error_handling.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {

inline void schedSetThreadAffinity(int cpu) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    throwingErrorErrno(sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpu_set));
}

template <class Rep, class Period>
struct __kernel_timespec
durationToKernelTimespec(std::chrono::duration<Rep, Period> dur) {
    struct __kernel_timespec ts;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);
    ts.tv_sec = static_cast<std::uint64_t>(secs.count());
    ts.tv_nsec = static_cast<std::uint64_t>(nsecs.count());
    return ts;
}

template <class Clk, class Dur>
struct __kernel_timespec
timePointToKernelTimespec(std::chrono::time_point<Clk, Dur> tp) {
    return durationToKernelTimespec(tp.time_since_epoch());
}

struct PlatformIOContextOptions {
    std::optional<std::chrono::steady_clock::duration> maxSleep = std::chrono::milliseconds(5);
    std::chrono::steady_clock::duration maxSleepInc = std::chrono::milliseconds(5);
    std::optional<std::size_t> threadAffinity = std::nullopt;
};

struct PlatformIOContext {
    inline bool waitEventsFor(std::size_t numBatch, std::optional<std::chrono::steady_clock::duration> timeout);

    std::size_t pendingEventCount() const {
        return io_uring_cq_ready(&mRing);
    }

    struct io_uring *getRing() {
        return &mRing;
    }

    PlatformIOContext &operator=(PlatformIOContext &&) = delete;

    explicit PlatformIOContext(std::size_t entries = 512) {
        throwingError(io_uring_queue_init(entries, &mRing, 0));
    }

    ~PlatformIOContext() {
        io_uring_queue_exit(&mRing);
    }

    void startMain(std::stop_token stop, PlatformIOContextOptions options) {
        if (options.threadAffinity) {
            schedSetThreadAffinity(*options.threadAffinity);
        }
        auto maxSleep = options.maxSleep;
        while (!stop.stop_requested()) [[likely]] {
            auto duration = GenericIOContext::instance->runDuration();
            if (maxSleep && (!duration || *duration > maxSleep)) {
                duration = maxSleep;
            }
            bool hasEvent = waitEventsFor(1, duration);
            if (options.maxSleep) {
                if (hasEvent) {
                    maxSleep = *options.maxSleep + options.maxSleepInc;
                } else {
                    maxSleep = options.maxSleep;
                }
            }
        }
    }

    static inline thread_local PlatformIOContext *instance;

private:
    struct io_uring mRing;
};

struct [[nodiscard]] UringOp {
    explicit UringOp(auto const &func) {
        struct io_uring *ring = PlatformIOContext::instance->getRing();
        mSqe = io_uring_get_sqe(ring);
        if (!mSqe) [[unlikely]] {
            throw std::bad_alloc();
        }
        io_uring_sqe_set_data(mSqe, this);
        func(mSqe);
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

    friend UringOp &uring_join(UringOp &&lhs, UringOp &&rhs) {
        lhs.mSqe->flags |= IOSQE_IO_LINK;
        rhs.mPrevious = std::noop_coroutine();
        return lhs;
    }

private:
    std::coroutine_handle<> mPrevious;

    union {
        int mRes;
        io_uring_sqe *mSqe;
    };

    friend PlatformIOContext;
};

inline UringOp uring_cancel(UringOp *op, unsigned int flags);

struct UringOpCanceller {
    using OpType = UringOp;

    static Task<> doCancel(OpType *op) {
        co_await uring_cancel(op, IORING_ASYNC_CANCEL_ALL);
    }

    static int earlyCancelValue(OpType *op) noexcept {
        return -ECANCELED;
    }
};

bool PlatformIOContext::waitEventsFor(std::size_t numBatch, std::optional<std::chrono::steady_clock::duration> timeout) {
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(&mRing, &cqe, numBatch, tsp, nullptr);
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

inline UringOp uring_nop() {
    return UringOp([&](io_uring_sqe *sqe) { io_uring_prep_nop(sqe); });
}

inline UringOp uring_openat(int dirfd, char const *path, int flags,
                            mode_t mode) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_openat(sqe, dirfd, path, flags, mode);
    });
}

inline UringOp uring_openat_direct(int dirfd, char const *path, int flags,
                                   mode_t mode, int file_index) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_openat_direct(sqe, dirfd, path, flags, mode, file_index);
    });
}

inline UringOp uring_socket(int domain, int type, int protocol,
                            unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_socket(sqe, domain, type, protocol, flags);
    });
}

inline UringOp uring_accept(int fd, struct sockaddr *addr, socklen_t *addrlen,
                            int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
    });
}

inline UringOp uring_connect(int fd, const struct sockaddr *addr,
                             socklen_t addrlen) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_connect(sqe, fd, addr, addrlen);
    });
}

inline UringOp uring_mkdirat(int dirfd, char const *path, mode_t mode) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_mkdirat(sqe, dirfd, path, mode);
    });
}

inline UringOp uring_linkat(int olddirfd, char const *oldpath, int newdirfd,
                            char const *newpath, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_linkat(sqe, olddirfd, oldpath, newdirfd, newpath, flags);
    });
}

inline UringOp uring_renameat(int olddirfd, char const *oldpath, int newdirfd,
                              char const *newpath, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_renameat(sqe, olddirfd, oldpath, newdirfd, newpath,
                               flags);
    });
}

inline UringOp uring_unlinkat(int dirfd, char const *path, int flags = 0) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_unlinkat(sqe, dirfd, path, flags);
    });
}

inline UringOp uring_symlinkat(char const *target, int newdirfd,
                               char const *linkpath) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_symlinkat(sqe, target, newdirfd, linkpath);
    });
}

inline UringOp uring_statx(int dirfd, char const *path, int flags,
                           unsigned int mask, struct statx *statxbuf) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_statx(sqe, dirfd, path, flags, mask, statxbuf);
    });
}

inline UringOp uring_read(int fd, std::span<char> buf, std::uint64_t offset) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_read(sqe, fd, buf.data(), buf.size(), offset);
    });
}

inline UringOp uring_write(int fd, std::span<char const> buf,
                           std::uint64_t offset) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_write(sqe, fd, buf.data(), buf.size(), offset);
    });
}

inline UringOp uring_read_fixed(int fd, std::span<char> buf,
                                std::uint64_t offset, int buf_index) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_read_fixed(sqe, fd, buf.data(), buf.size(), offset,
                                 buf_index);
    });
}

inline UringOp uring_write_fixed(int fd, std::span<char const> buf,
                                 std::uint64_t offset, int buf_index) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_write_fixed(sqe, fd, buf.data(), buf.size(), offset,
                                  buf_index);
    });
}

inline UringOp uring_readv(int fd, std::span<struct iovec const> buf,
                           std::uint64_t offset, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_readv2(sqe, fd, buf.data(), buf.size(), offset, flags);
    });
}

inline UringOp uring_writev(int fd, std::span<struct iovec const> buf,
                            std::uint64_t offset, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_writev2(sqe, fd, buf.data(), buf.size(), offset, flags);
    });
}

inline UringOp uring_recv(int fd, std::span<char> buf, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_recv(sqe, fd, buf.data(), buf.size(), flags);
    });
}

inline UringOp uring_send(int fd, std::span<char const> buf, int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_send(sqe, fd, buf.data(), buf.size(), flags);
    });
}

inline UringOp uring_recvmsg(int fd, struct msghdr *msg, unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_recvmsg(sqe, fd, msg, flags); });
}

inline UringOp uring_sendmsg(int fd, struct msghdr *msg, unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_sendmsg(sqe, fd, msg, flags); });
}

inline UringOp uring_close(int fd) {
    return UringOp([&](io_uring_sqe *sqe) { io_uring_prep_close(sqe, fd); });
}

inline UringOp uring_shutdown(int fd, int how) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_shutdown(sqe, fd, how); });
}

inline UringOp uring_fsync(int fd, unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_fsync(sqe, fd, flags); });
}

inline UringOp uring_ftruncate(int fd, loff_t len) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_ftruncate(sqe, fd, len); });
}

inline UringOp uring_cancel(UringOp *op, unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_cancel(sqe, op, flags); });
}

inline UringOp uring_cancel_fd(int fd, unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_cancel_fd(sqe, fd, flags); });
}

inline UringOp uring_waitid(idtype_t idtype, id_t id, siginfo_t *infop,
                            int options, unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_waitid(sqe, idtype, id, infop, options, flags);
    });
}

inline UringOp uring_timeout(struct __kernel_timespec *ts, unsigned int count,
                             unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_timeout(sqe, ts, count, flags);
    });
}

inline UringOp uring_link_timeout(struct __kernel_timespec *ts,
                                  unsigned int flags) {
    return UringOp(
        [&](io_uring_sqe *sqe) { io_uring_prep_link_timeout(sqe, ts, flags); });
}

inline UringOp uring_timeout_update(UringOp *op, struct __kernel_timespec *ts,
                                    unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_timeout_update(
            sqe, ts, reinterpret_cast<std::uintptr_t>(op), flags);
    });
}

inline UringOp uring_timeout_remove(UringOp *op, unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_timeout_remove(sqe, reinterpret_cast<std::uintptr_t>(op),
                                     flags);
    });
}

inline UringOp uring_splice(int fd_in, std::int64_t off_in, int fd_out,
                            std::int64_t off_out, std::size_t nbytes,
                            unsigned int flags) {
    return UringOp([&](io_uring_sqe *sqe) {
        io_uring_prep_splice(sqe, fd_in, off_in, fd_out, off_out, nbytes,
                             flags);
    });
}

} // namespace co_async
#endif

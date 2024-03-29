#pragma once

#include <co_async/error_handling.hpp>
#include <co_async/auto_destroy_promise.hpp>
#include <co_async/concepts.hpp>
#include <co_async/task.hpp>
#include <chrono>
#include <span>
#include <deque>
#include <cerrno>
#include <type_traits>
#include <liburing.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

namespace co_async {

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

struct UringLoop {
    inline void run();
    inline bool runBatched(
        std::size_t numBatch = 128,
        std::chrono::microseconds timeout = std::chrono::milliseconds(10));

    io_uring_sqe *getSqe() {
        io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
        if (!sqe) [[unlikely]] {
            throw std::bad_alloc();
        }
        return sqe;
    }

    UringLoop &operator=(UringLoop &&) = delete;

    explicit UringLoop(std::size_t entries = 512) {
        checkErrorReturn(io_uring_queue_init(entries, &mRing, 0));
    }

    ~UringLoop() {
        io_uring_queue_exit(&mRing);
    }

    io_uring mRing;
    std::deque<std::coroutine_handle<>> mQueue;
};

struct UringAwaiter {
    explicit UringAwaiter(UringLoop &loop, auto const &func) : mLoop(loop) {
        io_uring_sqe *sqe = mLoop.getSqe();
        func(sqe);
        io_uring_sqe_set_data(sqe, this);
        /* io_uring_sync_cancel_reg reg; */
        /* reg.flags; */
        /* io_uring_register_sync_cancel(&mLoop.mRing, &reg); */
    }

    void cancel() {
        io_uring_sqe *sqe = mLoop.getSqe();
        io_uring_prep_cancel(sqe, this, IORING_ASYNC_CANCEL_ALL);
        io_uring_sqe_set_data(sqe, nullptr);
    }

    UringAwaiter(UringAwaiter &&) = delete;

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine) {
        mPrevious = coroutine;
        checkErrorReturn(io_uring_submit(&mLoop.mRing));
    }

    int await_resume() const noexcept {
        return mRes;
    }

    UringLoop &mLoop;
    std::coroutine_handle<> mPrevious;
    int mRes = -ENOSYS;
};

void UringLoop::run() {
    while (!mQueue.empty()) {
        auto coroutine = std::move(mQueue.front());
        mQueue.pop_front();
        coroutine.resume();
    }

    io_uring_cqe *cqe;
    checkErrorReturn(io_uring_wait_cqe(&mRing, &cqe));
    auto *awaiter = reinterpret_cast<UringAwaiter *>(cqe->user_data);
    awaiter->mRes = cqe->res;
    io_uring_cqe_seen(&mRing, cqe);
    awaiter->mPrevious.resume();
}

bool UringLoop::runBatched(std::size_t numBatch,
                           std::chrono::microseconds timeout) {
    while (!mQueue.empty()) {
        auto coroutine = std::move(mQueue.front());
        mQueue.pop_front();
        coroutine.resume();
    }

    io_uring_cqe *cqe;
    auto ts = durationToKernelTimespec(timeout);
    int res = io_uring_wait_cqes(&mRing, &cqe, numBatch, &ts, nullptr);
    if (res == -ETIME) {
        return false;
    }
    checkErrorReturn(res);
    unsigned head, numGot = 0;
    io_uring_for_each_cqe(&mRing, head, cqe) {
        auto *awaiter = reinterpret_cast<UringAwaiter *>(cqe->user_data);
        awaiter->mRes = cqe->res;
        mQueue.push_back(awaiter->mPrevious);
        ++numGot;
    }
    io_uring_cq_advance(&mRing, numGot);
    return true;
}

inline Task<int> uring_openat(UringLoop &loop, int dirfd, char const *path,
                              int flags, mode_t mode) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_openat(sqe, dirfd, path, flags, mode);
        }));
}

inline Task<int> uring_socket(UringLoop &loop, int domain, int type,
                              int protocol, unsigned int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_socket(sqe, domain, type, protocol, flags);
        }));
}

inline Task<int> uring_accept(UringLoop &loop, int fd, struct sockaddr *addr,
                              socklen_t *addrlen, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
        }));
}

inline Task<int> uring_connect(UringLoop &loop, int fd,
                               const struct sockaddr *addr, socklen_t addrlen) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_connect(sqe, fd, addr, addrlen);
        }));
}

inline Task<int> uring_mkdirat(UringLoop &loop, int dirfd, char const *path,
                               mode_t mode) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_mkdirat(sqe, dirfd, path, mode);
        }));
}

inline Task<int> uring_linkat(UringLoop &loop, int olddirfd,
                              char const *oldpath, int newdirfd,
                              char const *newpath, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_linkat(sqe, olddirfd, oldpath, newdirfd, newpath,
                                 flags);
        }));
}

inline Task<int> uring_renameat(UringLoop &loop, int olddirfd,
                              char const *oldpath, int newdirfd,
                              char const *newpath, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_renameat(sqe, olddirfd, oldpath, newdirfd, newpath,
                                   flags);
        }));
}

inline Task<int> uring_unlinkat(UringLoop &loop, int dirfd, char const *path,
                                int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_unlinkat(sqe, dirfd, path, flags);
        }));
}

inline Task<int> uring_symlinkat(UringLoop &loop, char const *target,
                                 int newdirfd, char const *linkpath) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_symlinkat(sqe, target, newdirfd, linkpath);
        }));
}

inline Task<int> uring_statx(UringLoop &loop, int dirfd, char const *path,
                             int flags, unsigned int mask,
                             struct statx *statxbuf) {
    co_return co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_statx(sqe, dirfd, path, flags, mask, statxbuf);
    });
}

inline Task<std::size_t> uring_read(UringLoop &loop, int fd,
                                    std::span<char> buf, std::uint64_t offset) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_read(sqe, fd, buf.data(), buf.size(), offset);
        }));
}

inline Task<std::size_t> uring_write(UringLoop &loop, int fd,
                                     std::span<char const> buf,
                                     std::uint64_t offset) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_write(sqe, fd, buf.data(), buf.size(), offset);
        }));
}

inline Task<std::size_t> uring_readv(UringLoop &loop, int fd,
                                     std::span<iovec const> buf,
                                     std::uint64_t offset, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_readv2(sqe, fd, buf.data(), buf.size(), offset,
                                 flags);
        }));
}

inline Task<std::size_t> uring_writev(UringLoop &loop, int fd,
                                      std::span<iovec const> buf,
                                      std::uint64_t offset, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_writev2(sqe, fd, buf.data(), buf.size(), offset,
                                  flags);
        }));
}

inline Task<std::size_t> uring_recv(UringLoop &loop, int fd,
                                    std::span<char> buf, int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_recv(sqe, fd, buf.data(), buf.size(), flags);
        }));
}

inline Task<std::size_t> uring_send(UringLoop &loop, int fd,
                                    std::span<char const> buf, int flags, int zc_flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_send_zc(sqe, fd, buf.data(), buf.size(), flags, zc_flags);
        }));
}

inline Task<std::size_t> uring_recvmsg(UringLoop &loop, int fd,
                                       struct msghdr *msg, unsigned int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_recvmsg(sqe, fd, msg, flags);
        }));
}

inline Task<std::size_t> uring_sendmsg(UringLoop &loop, int fd,
                                       struct msghdr *msg, unsigned int flags) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_sendmsg_zc(sqe, fd, msg, flags);
        }));
}

inline Task<int> uring_close(UringLoop &loop, int fd) {
    co_return checkErrorReturn(co_await UringAwaiter(
        loop, [&](io_uring_sqe *sqe) { io_uring_prep_close(sqe, fd); }));
}

inline Task<int> uring_shutdown(UringLoop &loop, int fd, int how) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_shutdown(sqe, fd, how);
        }));
}

inline Task<int> uring_fsync(UringLoop &loop, int fd, unsigned int flags) {
    co_return checkErrorReturn(co_await UringAwaiter(
        loop, [&](io_uring_sqe *sqe) { io_uring_prep_fsync(sqe, fd, flags); }));
}

inline Task<int> uring_cancel_fd(UringLoop &loop, int fd, unsigned int flags) {
    co_return co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_cancel_fd(sqe, fd, flags);
    });
}

inline Task<int> uring_timeout(UringLoop &loop, struct __kernel_timespec ts,
                               unsigned int count, unsigned int flags) {
    int res = co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_timeout(sqe, &ts, count, flags);
    });
    if (res == -ETIME) [[likely]]
        res = 0;
    co_return checkErrorReturn(res);
}

template <class Rep, class Period>
inline Task<int> uring_timeout(UringLoop &loop,
                               std::chrono::duration<Rep, Period> dur,
                               std::size_t count) {
    return uring_timeout(loop, durationToKernelTimespec(dur), count,
                         IORING_TIMEOUT_ETIME_SUCCESS |
                             IORING_TIMEOUT_REALTIME);
}

template <class Clk, class Dur>
inline Task<int> uring_timeout(UringLoop &loop,
                               std::chrono::time_point<Clk, Dur> tp,
                               std::size_t count) {
    return uring_timeout(loop, timePointToKernelTimespec(tp), count,
                         IORING_TIMEOUT_ETIME_SUCCESS | IORING_TIMEOUT_ABS |
                             IORING_TIMEOUT_REALTIME);
}

template <Awaitable A>
inline Task<void, AutoDestroyPromise> uringEnqueueHelper(A a) {
    co_return co_await std::move(a);
}

template <class A>
    requires(!Awaitable<A> && std::invocable<A> &&
             Awaitable<std::invoke_result_t<A>>)
inline Task<void, AutoDestroyPromise> uringEnqueueHelper(A a) {
    return uringEnqueueHelper(std::move(a)());
}

inline void uring_enqueue(UringLoop &loop, auto task) {
    auto t = uringEnqueueHelper(std::move(task));
    auto coroutine = t.operator co_await().await_suspend(std::noop_coroutine());
    loop.mQueue.push_back(coroutine);
    t.release();
}

} // namespace co_async

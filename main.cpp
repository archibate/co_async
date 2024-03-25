#include <co_async/non_void_helper.hpp>
#include <co_async/uninitialized.hpp>
#include <co_async/error_handling.hpp>
#include <co_async/concepts.hpp>
#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <chrono>
#include <span>
#include <deque>
#include <cerrno>
#include <liburing.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

using namespace co_async;
using namespace std::literals;

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
    inline bool runBatched(std::size_t numBatch = 128, std::chrono::microseconds timeout = std::chrono::milliseconds(10));

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
        auto coroutine = mQueue.front();
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

bool UringLoop::runBatched(std::size_t numBatch, std::chrono::microseconds timeout) {
    while (!mQueue.empty()) {
        auto coroutine = mQueue.front();
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

Task<int> async_openat(UringLoop &loop, int dirfd, char const *path, int flags,
                       mode_t mode = 0644) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_openat(sqe, dirfd, path, flags, mode);
        }));
}

Task<int> async_open(UringLoop &loop, char const *path, int flags,
                     mode_t mode = 0644) {
    return async_openat(loop, AT_FDCWD, path, flags, mode);
}

Task<int> async_socket(UringLoop &loop, int domain, int type, int protocol,
                       unsigned int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_socket(sqe, domain, type, protocol, flags);
        }));
}

struct SockAddr {
    struct sockaddr_storage addr;
    socklen_t addrlen;
};

Task<int> async_accept(UringLoop &loop, int fd, SockAddr &addr, int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_accept(sqe, fd, (struct sockaddr *)&addr.addr,
                                 &addr.addrlen, flags);
        }));
}

Task<int> async_connect(UringLoop &loop, int fd, SockAddr &addr) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_connect(sqe, fd, (struct sockaddr *)&addr.addr,
                                  addr.addrlen);
        }));
}

using FileStat = struct statx;

Task<FileStat> async_stat(UringLoop &loop, int dirfd, char const *path,
                          unsigned int mask = STATX_ALL, int flags = 0) {
    FileStat statxbuf;
    co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_statx(sqe, dirfd, path, flags, mask, &statxbuf);
    });
    co_return statxbuf;
}

Task<FileStat> async_stat(UringLoop &loop, char const *path,
                          unsigned int mask = STATX_ALL, int flags = 0) {
    return async_stat(loop, AT_FDCWD, path, mask, flags);
}

Task<std::size_t> async_read(UringLoop &loop, int fd, std::span<char> buf,
                             std::uint64_t offset = -1) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_read(sqe, fd, buf.data(), buf.size(), offset);
        }));
}

Task<std::size_t> async_write(UringLoop &loop, int fd,
                              std::span<char const> buf,
                              std::uint64_t offset = -1) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_write(sqe, fd, buf.data(), buf.size(), offset);
        }));
}

Task<std::size_t> async_readv(UringLoop &loop, int fd,
                              std::span<iovec const> buf,
                              std::uint64_t offset = -1, int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_readv2(sqe, fd, buf.data(), buf.size(), offset,
                                 flags);
        }));
}

Task<std::size_t> async_writev(UringLoop &loop, int fd,
                               std::span<iovec const> buf,
                               std::uint64_t offset = -1, int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_writev2(sqe, fd, buf.data(), buf.size(), offset,
                                  flags);
        }));
}

Task<std::size_t> async_recv(UringLoop &loop, int fd, std::span<char> buf,
                             int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_recv(sqe, fd, buf.data(), buf.size(), flags);
        }));
}

Task<std::size_t> async_send(UringLoop &loop, int fd, std::span<char const> buf,
                             int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_send(sqe, fd, buf.data(), buf.size(), flags);
        }));
}

Task<std::size_t> async_recvmsg(UringLoop &loop, int fd, struct msghdr *msg,
                                unsigned int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_recvmsg(sqe, fd, msg, flags);
        }));
}

Task<std::size_t> async_sendmsg(UringLoop &loop, int fd, struct msghdr *msg,
                                unsigned int flags = 0) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_sendmsg(sqe, fd, msg, flags);
        }));
}

Task<int> async_close(UringLoop &loop, int fd) {
    co_return checkErrorReturn(co_await UringAwaiter(
        loop, [&](io_uring_sqe *sqe) { io_uring_prep_close(sqe, fd); }));
}

Task<int> async_shutdown(UringLoop &loop, int fd, int how) {
    co_return checkErrorReturn(
        co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
            io_uring_prep_shutdown(sqe, fd, how);
        }));
}

Task<int> async_fsync(UringLoop &loop, int fd, unsigned int flags) {
    co_return checkErrorReturn(co_await UringAwaiter(
        loop, [&](io_uring_sqe *sqe) { io_uring_prep_fsync(sqe, fd, flags); }));
}

Task<int> async_cancel_fd(UringLoop &loop, int fd,
                          unsigned int flags = IORING_ASYNC_CANCEL_FD) {
    co_return co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_cancel_fd(sqe, fd, flags);
    });
}

Task<int> async_timeout(UringLoop &loop, struct __kernel_timespec ts,
                        unsigned int count, unsigned int flags) {
    int res = co_await UringAwaiter(loop, [&](io_uring_sqe *sqe) {
        io_uring_prep_timeout(sqe, &ts, count, flags);
    });
    if (res == -ETIME) [[likely]]
        res = 0;
    co_return checkErrorReturn(res);
}

template <class Rep, class Period>
Task<int> async_timeout(UringLoop &loop, std::chrono::duration<Rep, Period> dur,
                        std::size_t count = 1) {
    return async_timeout(loop, durationToKernelTimespec(dur), count,
                         IORING_TIMEOUT_ETIME_SUCCESS |
                             IORING_TIMEOUT_REALTIME);
}

template <class Clk, class Dur>
Task<int> async_timeout(UringLoop &loop, std::chrono::time_point<Clk, Dur> tp,
                        std::size_t count = 1) {
    return async_timeout(loop, timePointToKernelTimespec(tp), count,
                         IORING_TIMEOUT_ETIME_SUCCESS | IORING_TIMEOUT_ABS |
                             IORING_TIMEOUT_REALTIME);
}

Task<> enqueueHelper(Awaitable auto task) {
    debug(), 0;
    co_await std::move(task);
    debug(), 1;
}

void enqueue(UringLoop &loop, Awaitable auto task) {
    auto t = enqueueHelper(std::move(task));
    auto awaiter = t.operator co_await();
    awaiter.await_suspend(std::noop_coroutine());
    loop.mQueue.push_back(t.release());
}

template <class T>
Task<T> operator*(Task<T> &&task) {
    auto t0 = std::chrono::high_resolution_clock::now();
    T ret = co_await std::move(task);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto dt = t1 - t0;
    std::cout << dt << '\n';
    co_return ret;
}

UringLoop loop;

Task<> amain() {
    int fd = co_await *async_open(loop, "/tmp/a.cpp", O_WRONLY | O_CREAT);
    char buf[] = "#include <cstdio>\nint main() {\n\tputs(\"This is co_async!\");\n}\n";
    co_await *async_write(loop, fd, buf);
    enqueue(loop, *async_close(loop, fd));
    fd = co_await *async_open(loop, "/tmp/a.cpp", O_RDONLY);
    /* co_await *async_read(loop, fd, buf); */
    /* co_await *async_write(loop, STDOUT_FILENO, buf); */
}

int main() {
    std::ios::sync_with_stdio(false);
    run_task(loop, amain());
    return 0;
}

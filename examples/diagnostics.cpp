#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#include <netinet/in.h>
#include <string.h>
#include <liburing.h>
#include <stdio.h>

static struct io_uring_sqe *sqe;
static int res;
static uint32_t futex;
static socklen_t addrlen;

#define P(name, ...) \
    [] { \
        io_uring_prep_##name(sqe __VA_OPT__(,) __VA_ARGS__); \
        return #name; \
    }

struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(1),
    .sin_addr = {
        .s_addr = htonl(INADDR_LOOPBACK),
    },
};

const char *(*(tests[]))() = {
    P(nop),
    P(read, 0, nullptr, 0, 0),
    P(write, 0, nullptr, 0, 0),
    P(socket, AF_INET, SOCK_STREAM, 0, 0),
    P(connect, res, (struct sockaddr *)&addr, sizeof(addr)),
    P(socket, AF_INET, SOCK_STREAM, 0, 0),
    P(accept, (listen(res, 2), res), (struct sockaddr *)&addr, &addrlen, 0),
    P(send, 0, nullptr, 0, 0),
    P(recv, 0, nullptr, 0, 0),
    P(openat, AT_FDCWD, ".", O_DIRECTORY, 0),
    P(close, res),
    P(futex_wait, &futex, 42, 1, 0, 0),
};

int main() {
    struct io_uring ring;
    res = io_uring_queue_init(8, &ring, 0);
    if (res < 0) {
        printf("io_uring_queue_init: %s\n", strerror(-res));
        return -1;
    }
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        sqe = io_uring_get_sqe(&ring);
        auto name = tests[i]();
        struct __kernel_timespec ts = {
            .tv_sec = 0,
            .tv_nsec = 1000000,
        };
        sqe->flags |= IOSQE_IO_LINK;
        io_uring_prep_link_timeout(io_uring_get_sqe(&ring), &ts, 0);
        struct io_uring_cqe *cqe;
        if (io_uring_submit(&ring) >= 0) {
            if (io_uring_wait_cqe_nr(&ring, &cqe, 2) == 0) {
                res = cqe->res;
                io_uring_cqe_seen(&ring, cqe);
                printf("[%5d] %s: %s\n", res, name, res > 0 ? "Success" : strerror(-res));
                unsigned head, n = 0;
                io_uring_for_each_cqe(&ring, head, cqe) {
                    ++n;
                }
                io_uring_cq_advance(&ring, n);
            }
        }
    }
    io_uring_queue_exit(&ring);
    return 0;
}

#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> handle_connection(SocketHandle income) {
    BytesBuffer buf(4096);
    while (true) {
        co_await UringOp::link_ops(UringOp().prep_recv(income.fileNo(), buf, 0),
            UringOp().prep_send(income.fileNo(), "HTTP/1.1 200 OK\r\nContent-length: 2\r\n\r\nOK"sv, 0));
        // co_await co_await socket_read(income, buf);
        // co_await co_await socket_write(income, "HTTP/1.1 200 OK\r\nContent-length: 2\r\n\r\nOK"sv);
    }
    co_return {};
}

static Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(co_await AddressResolver().host(serveAt).resolve_one());

#if 0
    ConcurrentQueue<OwningStream> incoming;
    incoming.set_max_size(1024);
    
    for (std::size_t i = 0; i < IOContextMT::num_workers(); ++i) {
        IOContextMT::nth_worker(i).spawn(co_bind([&]() -> Task<> {
            while (true) {
                if (auto income = co_await incoming.pop()) [[likely]] {
                    co_spawn(handle_connection(std::move(*income)));
                }
            }
        }));
    }

    while (true) {
        co_await co_await listener_accept(listener);
        co_await co_await incoming.push(std::move(*income));
    }
#else
    std::size_t i = 0;
    while (true) {
        auto income = co_await co_await listener_accept(listener);
        IOContextMT::nth_worker(i).spawn(handle_connection(std::move(income)));
        if (++i >= IOContextMT::num_workers()) {
            i = 0;
        }
    }
#endif
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "http://127.0.0.1:8080";
    if (argc > 1) {
        serveAt = argv[1];
    }
    IOContextMT().join(amain(serveAt)).value();
    return 0;
}

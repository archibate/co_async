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

    while (true) {
        auto income = co_await co_await listener_accept(listener);
        co_spawn(handle_connection(std::move(income)));
    }
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "http://127.0.0.1:8080";
    if (argc > 1) {
        serveAt = argv[1];
    }
    co_main(amain(serveAt));
    return 0;
}

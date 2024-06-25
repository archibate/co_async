#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(co_await SocketAddress::parse(serveAt, 80));

    // ConcurrentRobinhoodQueue<SocketHandle> incoming(IOContextMT::num_workers());
    // ConcurrentStealingQueue<SocketHandle> incoming(IOContextMT::num_workers());
    ConcurrentQueue<SocketHandle> incoming;
    incoming.set_max_size(512);

    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });

    std::size_t i = 0;
    while (true) {
        if (auto income = co_await listener_accept(listener)) [[likely]] {
            IOContextMT::nth_worker(i).spawn(server.handle_http(std::move(*income)));
            ++i;
            if (i >= IOContextMT::num_workers()) {
                i = 0;
            }
        }
    }
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:5000";
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (auto e = IOContextMT().join(amain(serveAt)); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

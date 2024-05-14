#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(
        SocketAddress::parseCommaSeperated(serveAt, 80));
    for (std::size_t i = 0; i < IOContextMT::num_workers(); ++i) {
        IOContextMT::nth_worker(i).spawn(co_bind([&]() -> Task<> {
            HTTPServer server;
            server.route("GET", "/", HTTPRouteMode::SuffixPath, [](HTTPServer::IO &io, std::string_view suffix) -> Task<Expected<>> {
                /* co_await co_await HTTPServerUtils::make_response_from_path(io, make_path(".", suffix)); */
                co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
                co_return {};
            });
            while (true) {
                if (auto income = co_await listener_accept(listener)) [[likely]] {
                    co_spawn(server.handle_http(std::move(*income)));
                }
            }
        }));
    }
    co_await co_forever();
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8080";
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (auto e = IOContextMT().join(amain(serveAt)); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

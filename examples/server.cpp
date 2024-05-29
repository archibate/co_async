#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    co_await co_await stdio().putline("listening at: 127.0.0.1:5000"sv);
    auto listener = co_await co_await listener_bind({"127.0.0.1", 5000});
    HTTPServer server;
    server.enableLogRequests();
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });

    while (true) {
        if (auto income = co_await listener_accept(listener)) {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}

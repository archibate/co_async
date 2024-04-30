#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    auto listener = co_await co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer server;
    server.route("GET", "/", HTTPRouteMode::SuffixPath, [](HTTPServer::IO &io, std::string_view suffix) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_response_from_path(io, make_path(".", suffix));
        co_return {};
    });

    while (1) {
        auto income = co_await co_await listener_accept(listener);
        co_spawn(server.handle_http(std::move(income)));
    }
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

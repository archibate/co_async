#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer server(listener);
    server.route("GET", "/", HTTPRouteMode::SuffixPath, [](HTTPServer::IO &io, std::string_view suffix) -> Task<> {
        co_await HTTPServerUtils::make_response_from_path(io, make_path(".", suffix));
    });

    while (1) {
        auto conn = co_await server.accept_http();
        co_spawn(server.process_connection(std::move(conn)));
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    HTTPServer server;
    co_await server.bind({"127.0.0.1", 8080});
    server.route("GET", "/", HTTPRouteMode::SuffixPath, [](HTTPServer::Protocol &http, HTTPRequest const &request, std::string_view suffix) -> Task<> {
        co_await HTTPServerUtils::make_response_from_path(http, request, make_path(".", suffix));
    });

    while (1) {
        auto conn = co_await server.accept();
        co_spawn(server.process_connection(std::move(conn)));
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

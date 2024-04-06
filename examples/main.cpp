#include "debug.hpp"

import co_async;
import std;

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto serv = co_await server_bind({"127.0.0.1", 8080});
    HTTPServer http;
    http.route("/", [] (HTTPRequest const &request) -> Task<HTTPResponse> {
        if (request.method != "GET")
            co_return HTTPServer::make_error_response(405);
        co_return {
            .status = 200,
            .headers = {
                {"content-type", "text/plain;charset=utf-8"},
            },
            .body = "<h1>It works!</h1>",
        };
    });

    co_await stdio().putline("正在监听: " + serv.address().toString());
    while (1) {
        auto conn = co_await server_accept(serv);
        co_await stdio().putline("收到请求: " + serv.address().toString());
        co_spawn(http.process_connection(FileStream(std::move(conn))));
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    co_spawn_and_wait(amain());
    return 0;
}

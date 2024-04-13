#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer http;
    http.route("GET", "/", [](HTTPRequest const &request) -> Task<HTTPResponse> {
        co_return co_await HTTPServer::make_response_from_directory(make_path("."));
    });
    http.prefix_route("GET", "/", HTTPServer::SuffixPath, [](HTTPRequest const &request, std::string_view suffix) -> Task<HTTPResponse> {
        co_return co_await HTTPServer::make_response_from_file_or_directory(make_path(suffix));
    });

    co_await stdio().putline("正在监听: " + listener.address().toString());
    while (1) {
        auto conn = co_await listener_accept(listener);
        co_await stdio().putline("线程 " + to_string(globalSystemLoop.this_thread_worker_id()) + " 收到请求: " + listener.address().toString());
        co_spawn(http.process_connection(SocketStream(std::move(conn))));
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

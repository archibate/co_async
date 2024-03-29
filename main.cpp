#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <co_async/uring_loop.hpp>
#include <co_async/filesystem.hpp>
#include <co_async/socket.hpp>
#include <co_async/stream.hpp>
#include <co_async/http_server.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    HTTPServer http;
    auto serv = co_await server_bind({"127.0.0.1", 8080});

    debug(), "正在监听", serv.address();
    while (1) {
        auto conn = co_await server_accept(serv);
        debug(), "收到请求", serv.address();
        enqueue(http.processConnection(FileStream(std::move(conn))));
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    run_task_on(loop, amain());
    return 0;
}

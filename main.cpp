#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <co_async/uring_loop.hpp>
#include <co_async/filesystem.hpp>
#include <co_async/socket.hpp>
#include <co_async/stream.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto serv = co_await server_bind({"127.0.0.1", 8080});
    auto conn = co_await server_accept(serv);

    debug(), "收到了来自", serv.address(), "的连接";

    debug(), "发来的消息头如下：";
    FileStream s(std::move(conn));
    std::string l;
    while (true) {
        l.clear();
        co_await s.getline(l, "\r\n");
        if (l.starts_with("Cookie:"))
            l = "Cookie: *******";
        debug(), l;
        if (l.empty()) {
            break;
        }
    }
    co_await s.puts("HTTP/1.1 200 OK\r\n");
    co_await s.puts("Content-Type: text/plain\r\n");
    co_await s.puts("Content-Length: 12\r\n");
    co_await s.puts("\r\n");
    co_await s.puts("Hello, world");
    co_await s.flush();
    co_return;
}

int main() {
    std::ios::sync_with_stdio(false);
    run_task(loop, amain());
    return 0;
}

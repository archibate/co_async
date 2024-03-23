#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <co_async/generator.hpp>
#include <co_async/timer_loop.hpp>
#include <co_async/epoll_loop.hpp>
#include <co_async/async_loop.hpp>
#include <co_async/when_any.hpp>
#include <co_async/when_all.hpp>
#include <co_async/limit_timeout.hpp>
#include <co_async/and_then.hpp>
#include <co_async/socket.hpp>

using namespace std::literals;

co_async::AsyncLoop loop;

co_async::Task<> amain() {
    auto sock = co_async::tcp_socket<co_async::Ipv4Address>();
    co_await socket_connect(loop, sock, co_async::Ipv4Address("142857.red", 8080));
    co_await socket_send(loop, sock, "GET / HTTP/1.1\r\nHost: 142857.red\r\nUser-Agent: co_async\r\n\r\n"sv);
    char buf[4096];
    auto len = co_await socket_recv(loop, sock, buf);
    std::string_view res(buf, len);
    std::cout << res;
    co_return;
}

int main() {
    run_task(loop, amain());
    return 0;
}

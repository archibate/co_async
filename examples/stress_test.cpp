#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

static std::atomic_size_t counter{0};

Task<> amain() {
    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer http;
    http.route("POST", "/", [](HTTPRequest const &request) -> Task<HTTPResponse> {
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", "text/plain;charset=utf-8"},
                },
            .body = request.body,
        };
    });

    while (1) {
        auto conn = co_await listener_accept(listener);
        counter.fetch_add(1, std::memory_order_relaxed);
        co_spawn(and_then(http.process_connection(SocketStream(std::move(conn))), [] () -> Task<> {
            counter.fetch_sub(1, std::memory_order_relaxed);
            co_return;
        }));
    }
}

int main() {
    std::jthread t([] (std::stop_token stop) {
        while (!stop.stop_requested()) {
            std::this_thread::sleep_for(0.1s);
            std::cout << "Connections: " << counter.load(std::memory_order_relaxed) << '\n';
        }
    });
    co_synchronize(amain());
    return 0;
}

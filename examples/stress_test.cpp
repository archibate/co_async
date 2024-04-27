#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

/* static std::atomic_size_t counter{0}; */

Task<> amain() {
    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer http;
    http.route("GET", "/", [](HTTPRequest const &request) -> Task<HTTPResponse> {
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", "text/html;charset=utf-8"},
                },
            .body = "<h1>It works!</h1>",
        };
    });
    http.route("POST", "/post", [](HTTPRequest const &request) -> Task<HTTPResponse> {
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", "text/plain;charset=utf-8"},
                },
            .body = request.body,
        };
    });

    for (std::size_t i = 0; i < globalSystemLoop.num_workers(); ++i) {
        co_spawn(i, co_bind([&listener, &http] () -> Task<> {
            while (1) {
                auto conn = co_await listener_accept(listener);
                /* counter.fetch_add(1, std::memory_order_relaxed); */
                co_spawn(http.process_connection(SocketStream(std::move(conn))));
                /* co_spawn(and_then(http.process_connection(SocketStream(std::move(conn))), co_bind([] () -> Task<> { */
                /*     counter.fetch_sub(1, std::memory_order_relaxed); */
                /*     co_return; */
                /* }))); */
            }
        }));
    }
    co_await std::suspend_always();
}

int main() {
    /* std::jthread t([] (std::stop_token stop) { */
    /*     while (!stop.stop_requested()) { */
    /*         std::this_thread::sleep_for(0.5s); */
    /*         std::cout << "Connections: " << counter.load(std::memory_order_relaxed) << '\n'; */
    /*     } */
    /* }); */
    co_synchronize(amain());
    return 0;
}

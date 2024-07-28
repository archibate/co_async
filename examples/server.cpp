#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    co_await co_await stdio().putline("listening at: 127.0.0.1:8080"sv);
    auto listener = co_await co_await listener_bind(co_await AddressResolver().host("127.0.0.1").port(8080).resolve_one());
    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });

    // Queue<Task<Expected<>>> tasks(512);
    // co_spawn(co_catch(co_bind([&] () -> Task<Expected<>> {
    //     while (true) {
    //         auto task = co_await co_await tasks.pop();
    //         co_spawn(std::move(task));
    //     }
    // })));

    while (true) {
        auto income = co_await co_await listener_accept(listener);
        co_spawn(server.handle_http(std::move(income)));
    }
}

int main() {
    co_main(amain());
    return 0;
}

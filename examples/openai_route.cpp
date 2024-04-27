#include <co_async/co_async.hpp>
#include <co_async/std.hpp>     

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await https_load_ca_certificates();
    /* auto host = "api.openai.com"s; */
    auto host = "man7.org"s;

    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    for (std::size_t i = 0; i < globalSystemLoop.num_workers(); ++i) {
        co_spawn(co_bind([&]() -> Task<> {
            HTTPServer server(listener);
            server.route("GET", "/favicon.ico",
                         [&](HTTPServer::IO &io) -> Task<> {
                             co_await HTTPServer::make_error_response(io, 404);
                         });
            server.route([&](HTTPServer::IO &io) -> Task<> {
                auto connection = co_await http_connect("https://" + host);
                HTTPRequest request = {
                    .method = io.request.method,
                    .uri = io.request.uri,
                    .headers = io.request.headers,
                };
                request.headers.insert_or_assign("host", host);
                FutureSource<HTTPResponse> response;
                auto [r1, w1] = co_await pipe_stream();
                auto [r2, w2] = co_await pipe_stream();
                co_await when_all(
                    co_bind([&]() -> Task<> {
                        co_await io.request_body_stream(w1);
                        co_await w1.close();
                    }),
                    co_bind([&]() -> Task<> {
                        HTTPResponse res = co_await response;
                        co_await io.response(res, r2);
                        co_await r2.close();
                    }),
                    co_bind([&]() -> Task<> {
                        co_await connection.request(request, r1, response.reference(), w2);
                        co_await r1.close();
                        co_await w2.close();
                    }));
            });
            while (1) {
                co_await server.process_connection(
                    co_await server.accept_http());
            }
        }));
    }

    co_await std::suspend_always();
}

int main() {
    globalSystemLoop.start(1);
    co_synchronize(amain());
    return 0;
}

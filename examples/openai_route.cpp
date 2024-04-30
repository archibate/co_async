#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    co_await https_load_ca_certificates();
    auto host = "api.openai.com"s;
    /* auto host = "www.baidu.com"s; */
    /* auto host = "man7.org"s; */

    auto listener = co_await co_await listener_bind({"127.0.0.1", 8080});
    HTTPConnectionPool pool;
    for (std::size_t i = 0; i < globalSystemLoop.num_workers(); ++i) {
        co_spawn(i, co_bind([&]() -> Task<> {
                     HTTPServer server;
                     server.route(
                         "GET", "/favicon.ico",
                         [&](HTTPServer::IO &io) -> Task<Expected<>> {
                             co_return co_await HTTPServer::make_error_response(
                                 io, 404);
                         });
                     server.route([&](HTTPServer::IO &io) -> Task<Expected<>> {
                         auto connection = co_await co_await pool.connect("https://" + host);
                         HTTPRequest request = {
                             .method = io.request.method,
                             .uri = io.request.uri,
                             .headers = io.request.headers,
                         };
                         request.headers.insert_or_assign("host", host);
#if 1
                         auto in = co_await co_await io.request_body();
                         HTTPResponse res;
                         std::string out;
                         co_await co_await connection->request(request, in, res,
                                                               out);
                         co_await co_await io.response(res, out);
#else
                         FutureSource<HTTPResponse> response;
                         auto [r1, w1] = co_await pipe_stream();
                         auto [r2, w2] = co_await pipe_stream();
                         auto [t1, t2, t3] = co_await when_all(
                             co_bind([&]() -> Task<Expected<>> {
                                 co_await co_await io.request_body_stream(w1);
                                 co_await w1.close();
                                 co_return {};
                             }),
                             co_bind([&]() -> Task<Expected<>> {
                                 HTTPResponse res = co_await response;
                                 co_await co_await io.response(res, r2);
                                 co_await r2.close();
                                 co_return {};
                             }),
                             co_bind([&]() -> Task<Expected<>> {
                                 co_await co_await connection->request(
                                     request, r1, response.reference(), w2);
                                 co_await r1.close();
                                 co_await w2.close();
                                 co_return {};
                             }));
                         co_await std::move(t1);
                         co_await std::move(t2);
                         co_await std::move(t3);
#endif
                         co_return {};
                     });
                     while (1) {
                         if (auto income = co_await listener_accept(listener))
                             [[likely]] {
                             co_spawn(server.handle_http(std::move(*income)));
                         }
                     }
                 }));
    }

    co_await std::suspend_always();
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

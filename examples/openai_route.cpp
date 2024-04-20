#include "co_async/utils/debug.hpp"
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>     /*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await HTTPConnectionHTTPS::load_ca_certificates();
    auto host = "api.openai.com"s;

    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    for (std::size_t i = 0; i < globalSystemLoop.num_workers(); ++i) {
        co_spawn(co_bind([&]() -> Task<> {
            HTTPServer server(listener);
            server.route([&](HTTPServer::Protocol &http,
                             HTTPRequest const &request) -> Task<> {
                auto conn = co_await http_connect("https://" + host);
                HTTPRequest req = {
                    .method = request.method,
                    .uri = request.uri,
                    .headers = request.headers,
                };
                req.headers.insert_or_assign("host", host);
                co_await conn->write_header(req);
                if (1) {
                         auto body = co_await http.read_body();
                         debug(), body;
                    co_await conn->write_body(body);
                } else {
                    auto pipe = co_await make_pipe();
                    FileIStream reader(pipe.reader());
                    FileOStream writer(pipe.writer());
                    co_await when_all(and_then(http.read_body_stream(writer),
                                               co_bind([&]() -> Task<> {
                                                   co_await fs_close(
                                                       writer.release());
                                               })),
                                      conn->write_body_stream(reader));
                }
                HTTPResponse response;
                co_await conn->read_header(response);
                HTTPResponse res = {
                    .status = response.status,
                    .headers = response.headers,
                };
                co_await http.write_header(res);
                if (1) {
                         auto body = co_await conn->read_body();
                         debug(), body;
                         http.encoding = HTTPTransferEncoding::Identity;
                    co_await http.write_body(body);
                } else {
                    auto pipe = co_await make_pipe();
                    FileIStream reader(pipe.reader());
                    FileOStream writer(pipe.writer());
                    co_await when_all(and_then(conn->read_body_stream(writer),
                                               co_bind([&]() -> Task<> {
                                                   co_await fs_close(
                                                       writer.release());
                                               })),
                                      http.write_body_stream(reader));
                }
                co_return;
            });
            while (1) {
                co_await server.process_connection(co_await server.accept());
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

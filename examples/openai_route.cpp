#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>     /*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await HTTPConnectionHTTPS::load_ca_certificates();

    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    for (std::size_t i = 0; i < 16; ++i) {
        co_spawn(co_bind([&]() -> Task<> {
            auto conn = co_await http_connect("https://api.openai.com");
            HTTPServer server(listener);
            server.route([&](HTTPServer::Protocol &http,
                             HTTPRequest const &request) -> Task<> {
                {
                    HTTPRequest req = {
                        .method = request.method,
                        .uri = request.uri,
                        .headers = request.headers,
                    };
                    auto pipe = co_await make_pipe();
                    FileStream reader(pipe.reader());
                    FileStream writer(pipe.writer());
                    co_await http.read_body_stream(writer);
                    co_await conn->write_header(req);
                    co_await conn->write_body_stream(writer);
                }
                HTTPResponse response;
                co_await conn->read_header(response);
                {
                    auto pipe = co_await make_pipe();
                    FileStream reader(pipe.reader());
                    FileStream writer(pipe.writer());
                    co_await conn->read_body_stream(writer);
                    HTTPResponse res = {
                        .status = response.status,
                        .headers = response.headers,
                    };
                    co_await http.write_header(res);
                    co_await http.write_body_stream(reader);
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
    co_synchronize(amain());
    return 0;
}

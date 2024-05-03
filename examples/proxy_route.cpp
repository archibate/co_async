#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<void, std::errc>> amain(std::string serveAt,
                                      std::string targetHost) {
    co_await https_load_ca_certificates();

    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(
        SocketAddress::parseCommaSeperated(serveAt, 80));
    HTTPConnectionPool pool;
    for (std::size_t i = 0; i < globalSystemLoop.num_workers(); ++i) {
        co_spawn(
            i, co_bind([&]() -> Task<> {
                HTTPServer server;
                server.route(
                    "GET", "/favicon.ico",
                    [&](HTTPServer::IO &io) -> Task<Expected<>> {
                        co_return co_await HTTPServer::make_error_response(io,
                                                                           404);
                    });
                server.route([&](HTTPServer::IO &io) -> Task<Expected<>> {
                    std::string host = targetHost;
                    if (host.empty()) {
                        if (auto pos = io.request.uri.path.find("://"sv, 1);
                            pos != std::string_view::npos) [[likely]] {
                            if ((pos =
                                     io.request.uri.path.find('/', pos + 3)) !=
                                std::string_view::npos) [[likely]] {
                                host = io.request.uri.path.substr(1, pos - 1);
                                io.request.uri.path =
                                    io.request.uri.path.substr(pos);
                            }
                        }
                        if (host.empty()) {
                            auto thisHost =
                                io.request.headers.get("host"sv).value_or(""s);
                            co_await co_await io.response(
                                HTTPResponse{
                                    .status = 200,
                                    .headers =
                                        {
                                            {"content-type"s,
                                             "text/html;charset=utf-8"s},
                                        },
                                },
                                "<center><h1>SSL Latency Reducing Proxy Server</h1></center><p>This is a proxy server! Send HTTP requests to me and I will forward it as HTTP or HTTPS for you. The target HTTP or HTTPS URL is specified in path (see below). HTTPS connections are automatically keep alive in the background for future reuse, no more repeatitive TCP and SSL handshakes wasting time! Signiticantly reducing overhead for one-shot command line tools like curl. It's tested to reducing latency from ~400ms to ~120ms for https://api.openai.com/, compared to original curl command (won't speed up your OpenAI computation tasks, of course, we only reduce the unnecessary overhead in SSL handshake). Also allowing libraries that only support HTTP to visit HTTPS sites.</p><hr><p>For example, if your curl command was:</p><pre><code>curl https://www.example.com/index.html</code></pre><p>Then you may run this instead for speeding up:</p><pre><code>curl http://"s +
                                    thisHost +
                                    "/https://www.example.com/index.html</code></pre><p>It costs the same as original curl for the first time as SSL tunnel is building. But the later visits would become faster, useful for repeatitive curl commands.</p><hr><center>powered by <a href=\"https://github.com/archibate/co_async\">co_async</a></center>"s);
                            co_return {};
                        }
                    }
                    auto connection = co_await co_await pool.connect(host);
                    HTTPRequest request = {
                        .method = io.request.method,
                        .uri = io.request.uri,
                        .headers = io.request.headers,
                    };
                    request.headers.insert_or_assign(
                        "host"s,
                        std::string(host.substr(host.find("://"sv) + 3)));
                    auto in = co_await co_await io.request_body();
                    #if 0
                    std::string out;
                    HTTPResponse response;
                    co_await co_await connection->request(request, in, response, out);
                    co_await co_await io.response(response, out);
                    #else
                    FutureGroup group;
                    FutureSource<HTTPResponse> response;
                    auto out = co_await co_await pipe_invoke(group, [&](OwningStream &out) -> Task<Expected<>> {
                        co_await co_await connection->request(
                            request, in, response.reference(), out);
                            co_await out.close();
                            co_return {};
                        });
                    co_await co_await io.response(co_await response, out);
                    co_await co_await group.wait();
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

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8080";
    std::string targetHost;
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (argc > 2) {
        targetHost = argv[2];
    }
    if (int err = (int)co_synchronize(amain(serveAt, targetHost)).error()) {
        std::cerr << argv[0] << ": " << std::system_category().message(err)
                  << '\n';
        return err;
    }
    return 0;
}

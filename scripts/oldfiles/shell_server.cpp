#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

namespace {

/* struct ClientMain { */
/*     HTTPConnectionPool pool; */
/*     std::string address; */
/*     std::string id; */
/*  */
/*     explicit ClientMain(std::string address_, std::string id_ = "") */
/*         : pool(32), */
/*           address(address_), */
/*           id(id_) {} */
/*  */
/*     Task<Expected<>> writer() { */
/*         while (true) { */
/*             auto chunk = co_await co_await stdio().getchunk(); */
/*             auto conn = co_await co_await pool.connect("http://" + address); */
/*             HTTPRequest req = { */
/*                 .method = "POST", */
/*                 .uri = */
/*                     URI{ */
/*                         .path = "/write", */
/*                         .params = */
/*                             { */
/*                                 {"id", id}, */
/*                                 {"side", "c"}, */
/*                             }, */
/*                     }, */
/*                 .headers = { */
/*                     {"content-type", "application/json"}, */
/*                     {"cache-control", "no-cache"}, */
/*                 }, */
/*             }; */
/*             co_await co_await conn->request(req, json_encode(chunk)); */
/*         } */
/*     } */
/*  */
/*     Task<Expected<>> reader() { */
/*         while (true) { */
/*             auto conn = co_await co_await pool.connect("http://" + address); */
/*             HTTPRequest req = { */
/*                 .method = "GET", */
/*                 .uri = */
/*                     { */
/*                         .path = "/read", */
/*                         .params = */
/*                             { */
/*                                 {"id", id}, */
/*                                 {"side", "c"}, */
/*                             }, */
/*                     }, */
/*                 .headers = */
/*                     { */
/*                         {"content-type", "application/json"}, */
/*                         {"cache-control", "no-cache"}, */
/*                         {"accept", "text/event-stream"}, */
/*                     }, */
/*             }; */
/*             auto [resp, body] = co_await co_await conn->request_streamed(req); */
/*  */
/*             while (auto tmp = co_await body.getline('\n')) { */
/*                 std::string_view line = *tmp; */
/*                 if (line.starts_with("data: "sv)) { */
/*                     line.remove_prefix(6); */
/*                     if (line == "[DONE]"sv) { */
/*                         break; */
/*                     } */
/*                     auto chunk = co_await json_decode<std::string>(line); */
/*                     co_await co_await stdio().putchunk(chunk); */
/*                 } */
/*             } */
/*             co_await body.dropall(); */
/*             co_await body.close(); */
/*         } */
/*     } */
/*  */
/*     Task<Expected<>> operator()() { */
/*         co_await co_await stdio().putline("connecting to: http://"s + address); */
/*         co_await co_await when_all(reader(), writer()); */
/*         co_return {}; */
/*     } */
/* }; */

Task<Expected<>> server_main(std::string address) {
    co_await co_await stdio().putline("listening at: http://"s + address);
    auto listener = co_await co_await listener_bind(
        co_await SocketAddress::parse(address, 80));

    struct Pipe {
        std::array<std::array<OwningStream, 2>, 2> endpoints;
    };

    SimpleMap<std::string, Pipe> pipes;

    HTTPServer server;
    /* server.enableLogRequests(); */

    server.route("GET", "/", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        std::string host = io.request.headers.get("host").value_or(address);
        std::string page = R"html(
<h1>Server side:</h1>
<pre><code>
curl -sSL )html" + host + R"html(/server | bash
</pre></code>
<h1>Client side:</h1>
<pre><code>
curl -sSL )html" + host + R"html(/client | bash
</pre></code>)html";
        co_await co_await HTTPServerUtils::make_ok_response(
            io, page, "text/html;charset=utf-8");
        co_return {};
    });

    server.route("GET", "/server", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        std::string host = io.request.headers.get("host").value_or(address);
        auto cmd = io.request.uri.params.get("cmd").value_or("bash");
        if (io.request.uri.params.get("err").value_or("1") == "1") {
            cmd += " 2>&1";
        }
        std::string script = R"bash(set -e
A=")bash" + host + R"bash("
F="/tmp/.shell_server_in.$RANDOM"
L="/tmp/.shell_server_lock.$RANDOM"
touch "$L"
(while test -f "$L"; do
    curl -sSLN -X GET "$A/read?id=&side=s" -H "Accept: application/octet-stream" || break
done) | ()bash" + cmd + R"bash(; rm "$L") | (while true; do
    dd bs=1024 count=1 of="$F" 2>/dev/null
    < "$F"
    curl -sSL -X POST "$A/write?id=&side=s" -H "Content-Type: application/octet-stream" --data-binary "@$F" > /dev/null || break
done)
rm -f "$F" "$L")bash";
        co_await co_await HTTPServerUtils::make_ok_response(
            io, script, "text/plain;charset=utf-8");
        co_return {};
    });

    server.route("GET", "/client", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        std::string host = io.request.headers.get("host").value_or(address);
        std::string script = R"bash(set -e
A=")bash" + host + R"bash("
F="/tmp/.shell_client_in.$RANDOM"
L="/tmp/.shell_client_lock.$RANDOM"
touch "$L"
(while test -f "$L"; do
    curl -sSLN -X GET "$A/read?id=&side=c" -H "Accept: application/octet-stream" || break
done) | (bash 2>&1; rm "$L") | (while true; do
    dd bs=1024 count=1 of="$F" 2>/dev/null
    < "$F"
    curl -sSL -X POST "$A/write?id=&side=c" -H "Content-Type: application/octet-stream" --data-binary "@$F" > /dev/null || break
done)
rm -f "$F" "$L")bash";
        co_await co_await HTTPServerUtils::make_ok_response(
            io, script, "text/plain;charset=utf-8");
        co_return {};
    });

    enum Side : std::size_t {
        Client = 0,
        Server = 1,
    };

    auto side_handler = [&](HTTPServer::IO &io) -> Task<Expected<Side>> {
        auto oside = io.request.uri.params.get("side");
        if (oside == "s") {
            co_return Server;
        } else if (oside == "c") {
            co_return Client;
        } else {
            co_await co_await io.response(
                HTTPResponse{
                    .status = 400,
                    .headers =
                        {
                            {"x-accel-buffering", "no"},
                            {"content-type", "text/plain;charset=utf-8"},
                            {"cache-control", "no-cache"},
                            {"keep-alive", "timeout=10, max=1000"},
                        },
                },
                "[ERROR: side argument must be either c or s]");
            co_return std::errc::invalid_argument;
        }
    };

    server.route("GET", "/read", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        auto id = io.request.uri.params.get("id").value_or("");
        auto side = co_await co_await side_handler(io);
        std::string type = "text/event-stream";
        if (io.request.headers.get("accept") != type) {
            type = "application/octet-stream";
        }

        auto pipe = pipes.at(id);
        if (!pipe) {
            pipe = &pipes.insert(id, {pipe_stream(), pipe_stream()});
        }

        auto &file = pipe->endpoints[static_cast<std::size_t>(side)][0];
        auto [reader, writer] = pipe_stream();
        co_await co_await when_all(
            pipe_bind(
                std::move(reader),
                [&](OwningStream &reader) -> Task<Expected<>> {
                    co_await co_await io.response(
                        HTTPResponse{
                            .status = 200,
                            .headers =
                                {
                                    {"x-accel-buffering", "no"},
                                    {"content-type", type},
                                    {"cache-control", "no-cache"},
                                    {"keep-alive", "timeout=10, max=1000"},
                                },
                        },
                        reader);
                    co_return {};
                }),
            pipe_bind(std::move(writer),
                      [&](OwningStream &writer) -> Task<Expected<>> {
                          while (auto res =
                                     co_await co_timeout(file.getchunk(), 500ms)) {
                              if (type == "text/event-stream") {
                                  co_await co_await writer.puts("data: ");
                                  co_await co_await writer.puts(
                                      json_encode(*res));
                                  co_await co_await writer.puts("\n\n");
                              } else {
                                  co_await co_await writer.puts(*res);
                              }
                              co_await co_await writer.flush();
                          }
                          if (type == "text/event-stream") {
                              co_await co_await writer.puts("data: [DONE]\n\n");
                              co_await co_await writer.flush();
                          }
                          co_return {};
                      }));

        co_return {};
    });

    server.route("POST", "/write", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        auto id = io.request.uri.params.get("id").value_or("");
        auto side = co_await co_await side_handler(io);

        auto body = co_await co_await io.request_body();
        if (io.request.headers.get("content-type") == "application/json") {
            body = co_await json_decode<std::string>(body);
        }

        auto pipe = pipes.at(id);
        if (!pipe) {
            pipe = &pipes.insert(id, {pipe_stream(), pipe_stream()});
        }

        auto &file = pipe->endpoints[1 - static_cast<std::size_t>(side)][1];
        if (!body.empty()) {
            co_await co_await file.putchunk(body);
        }

        co_await co_await io.response(
            HTTPResponse{
                .status = 200,
                .headers =
                    {
                        {"x-accel-buffering", "no"},
                        {"content-type", "text/plain;charset=utf-8"},
                        {"cache-control", "no-cache"},
                        {"keep-alive", "timeout=10, max=1000"},
                    },
            },
            "[WRITTEN]");

        co_return {};
    });

    while (true) {
        co_await co_await co_cancel;
        if (auto income = co_await listener_accept(listener)) [[likely]] {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
    co_return {};
}

Task<Expected<>> amain(std::string address) {
    /* (void)raw_stdio(); */
    /* ClientMain client_main(address); */
    /* auto fut = co_future(client_main()); */
    auto ret = co_await server_main(address);
    /* (void)co_await fut; */
    co_return ret;
}

} // namespace

int main(int argc, char **argv) {
    std::string address = "0.0.0.0:8888";
    if (argc > 1) {
        address = argv[1];
    }
    if (auto e = IOContext().join(amain(address)); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

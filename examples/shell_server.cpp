#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(
        co_await SocketAddress::parse(serveAt, 80));

    SimpleMap<std::string, std::array<OwningStream, 2>> pipes;

    HTTPServer server;
    server.route("GET", "/", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        std::string page = "Hello, world!";
        co_await co_await HTTPServerUtils::make_ok_response(
            io, page, "text/plain;charset=utf-8");
        co_return {};
    });
    server.route("GET", "/pipe", [&](HTTPServer::IO &io) -> Task<Expected<>> {
        std::array<OwningStream, 2> *pipe = nullptr;
        enum Side : std::size_t {
            Client = 0,
            Server = 1,
        } side = Client;
        if (io.request.uri.params.get("side") == "s") {
            side = Server;
        }
        enum Mode {
            Read = 0,
            Write = 1,
        } mode = Read;
        if (io.request.uri.params.get("mode") == "w") {
            mode = Write;
        }
        auto id = io.request.uri.params.get("id").value_or("");
        if (auto o_pipe = pipes.at(id)) {
            pipe = &*o_pipe;
        } else {
            pipe = &pipes.insert(id, pipe_stream());
        }
        if (!pipe) [[unlikely]] {
            co_await co_await HTTPServerUtils::make_error_response(io, 400);
        } else {
            auto &our_side_pipe = (*pipe)[static_cast<std::size_t>(side)];
            if (mode == Write) {
                auto body = co_await co_await io.request_body();
                co_await co_await our_side_pipe.putchunk(body);
            } else {
                auto chunk = co_await co_await our_side_pipe.getchunk();
                co_await co_await HTTPServerUtils::make_ok_response(
                    io, chunk, "text/plain;charset=utf-8");
            }
        }
        co_return {};
    });

    while (true) {
        if (auto income = co_await listener_accept(listener)) [[likely]] {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8848";
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (auto e = IOContextMT().join(amain(serveAt)); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

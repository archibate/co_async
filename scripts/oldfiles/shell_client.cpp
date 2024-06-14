#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

namespace {

struct ClientMain {
    std::string id = "insecure-shell";
    std::string address;
    HTTPConnectionPool pool;

    explicit ClientMain(std::string address_) : address(address_) {}

    Task<Expected<>> writer() {
        while (true) {
            auto chunk = co_await co_await raw_stdio().getchunk();
            auto conn = co_await co_await pool.connect(address);
            co_await co_await conn->request(
                {
                    .method = "POST",
                    .uri =
                    {
                        .path = "/write",
                        .params =
                        {
                            {"id", id},
                            {"side", "0"},
                        },
                    },
                    .headers =
                    {
                        {"content-type", "application/json"},
                        {"accept", "text/event-stream"},
                    },
                },
                json_encode(chunk));
        }
    }

    Task<Expected<>> reader() {
        while (true) {
            auto conn = co_await co_await pool.connect(address);
            auto [resp, body] = co_await co_await conn->request_streamed(
                {
                    .method = "GET",
                    .uri =
                        {
                            .path = "/read",
                            .params =
                                {
                                    {"id", id},
                                    {"side", "0"},
                                },
                        },
                    .headers =
                        {
                            {"content-type", "application/json"},
                            {"cache-control", "no-cache"},
                            {"accept", "text/event-stream"},
                        },
                });

            while (auto tmp = co_await body.getline('\n')) {
                std::string_view line = *tmp;
                if (line.starts_with("data: "sv)) {
                    line.remove_prefix(6);
                    if (line == "[DONE]"sv) {
                        break;
                    }
                    auto chunk = co_await json_decode<std::string>(line);
                    co_await co_await raw_stdio().putchunk(chunk);
                }
            }
            co_await body.dropall();
            co_await body.close();
        }
    }

    Task<Expected<>> operator()() {
        co_await co_await stdio().putline("connecting to: "s + address);
        co_await co_await when_all(reader(), writer());
        co_return {};
    }
};

}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8888";
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (auto e = IOContextMT().join(ClientMain(serveAt)()); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

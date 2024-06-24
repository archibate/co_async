#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> handle_connection(OwningStream income) {
    while (auto line = co_await income.getline("\r\n"sv)) {
        if (line->empty()) {
            co_await co_await income.puts("HTTP/1.1 200 OK\r\nContent-length: 2\r\n\r\nOK"sv);
            co_await co_await income.flush();
        }
    }
    co_return {};
}

static Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(co_await SocketAddress::parse(serveAt, 80));

#if 0
    ConcurrentRobinhoodQueue<OwningStream> incoming(IOContextMT::num_workers());
    incoming.set_max_size(1024);
    
    for (std::size_t i = 0; i < IOContextMT::num_workers(); ++i) {
        IOContextMT::nth_worker(i).spawn(co_bind([incoming = incoming.consumer(i)]() -> Task<> {
            while (true) {
                auto income = co_await incoming.pop();
                co_spawn(handle_connection(std::move(income)));
            }
        }));
    }

    while (true) {
        if (auto income = co_await tcp_accept(listener, std::chrono::seconds(3))) [[likely]] {
            co_await incoming.push(std::move(*income));
        }
    }
#else
    std::size_t i = 0;
    while (true) {
        if (auto income = co_await tcp_accept(listener, std::chrono::seconds(3))) [[likely]] {
            IOContextMT::nth_worker(i).spawn_mt(handle_connection(std::move(*income)));
            ++i;
            if (i >= IOContextMT::num_workers()) {
                i = 0;
            }
        }
    }
#endif
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8080";
    if (argc > 1) {
        serveAt = argv[1];
    }
    if (auto e = IOContextMT().join(amain(serveAt)); e.has_error()) {
        std::cerr << argv[0] << ": " << e.error().message() << '\n';
        return e.error().value();
    }
    return 0;
}

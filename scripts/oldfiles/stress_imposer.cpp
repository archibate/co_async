#include <co_async/co_async.hpp>
#include <co_async/std.hpp>     

using namespace co_async;
using namespace std::literals;

static thread_local std::mt19937 rng{std::random_device{}()};

Task<> stressConnection(SocketStream s) {
    std::size_t n = 8;
    for (std::size_t i = 0; i < n; ++i) {
        HTTPRequest req;
        req.method = "POST";
        req.uri.path = "/post";
        req.body.resize(
            std::uniform_int_distribution<std::size_t>(64, 1024)(rng));
        std::generate(req.body.begin(), req.body.end(),
                      std::bind(std::uniform_int_distribution<char>(' ', '~'),
                                std::ref(rng)));
        co_await req.write_into(s, i != n - 1);
        HTTPResponse res;
        bool keepAlive = co_await res.read_from(s);
        if (res.body != req.body) {
            std::cerr << "response body mismatch\n";
        }
        if (!keepAlive && i != n - 1) {
            std::cerr << "server refuse to keep-alive\n";
            break;
        }
    }
    co_await socket_shutdown(s.get());
    co_await fs_close(s.release());
}

Task<> amain() {
    for (std::size_t i = 0; i < 10; ++i) {
        for (std::size_t j = 0; j < 100; ++j) {
            auto conn = co_await socket_connect({"127.0.0.1", 8080});
            co_spawn(stressConnection(SocketStream(std::move(conn))));
        }
        co_await sleep_for(5s);
        printf("%zd/10\n", i + 1);
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

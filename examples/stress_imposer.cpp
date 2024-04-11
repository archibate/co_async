#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

static thread_local std::mt19937 rng{std::random_device{}()};

Task<> stressConnection(SocketStream s) {
    HTTPRequest req;
    req.method = "POST";
    req.uri.path = "/";
    req.body.resize(std::uniform_int_distribution<std::size_t>(64, 1024)(rng));
    std::generate(req.body.begin(), req.body.end(), std::bind(std::uniform_int_distribution<char>(' ', '~'), std::ref(rng)));
    co_await req.write_into(s);
    HTTPResponse res;
    co_await res.read_from(s);
    if (res.body != req.body) {
        std::cerr << "response body mismatch\n";
    }
}

Task<> amain() {
    for (std::size_t i = 0; i < 100; ++i) {
        for (std::size_t j = 0; j < 800; ++j) {
            auto conn = co_await socket_connect({"127.0.0.1", 8080});
            co_spawn(stressConnection(SocketStream(std::move(conn))));
        }
        co_await sleep_for(1s);
        printf("%zd%%\n", i);
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    co_await https_load_ca_certificates();
    HTTPConnectionPool pool;
    auto conn = co_await pool.connect("https://man7.org");
    HTTPRequest req = {
        .method = "GET",
        .uri = URI::parse("/koru-icon.png"),
    };
    HTTPResponse res;
    std::string body;
    debug(), "req", req;
    co_await co_await conn->request(req, {}, res, body);
    debug(), "res", body.size(), res;
    co_await co_await file_write(make_path("koru-icon.png"), body);
    Pid pid = co_await co_await ProcessBuilder()
        .path("display")
        .arg("koru-icon.png")
        .spawn();
    co_await co_await wait_process(pid);
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

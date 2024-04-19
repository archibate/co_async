#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/
#define CO_ASYNC_PERF 1
#include <co_async/utils/perf.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    HTTPClientPool pool;
    co_await pool.load_ca_certifi();
    Perf _;
    auto conn = co_await pool.connect("https://man7.org");
    HTTPRequest req = {
        .method = "GET",
        .uri = URI::parse("/"),
    };
    Perf _2;
    co_await conn->write_header(req);
    co_await conn->write_nobody();
    Perf _3;
    co_await conn->read_header();
    Perf _4;
    debug(), co_await conn->read_body();
}

int main() {
    co_synchronize(amain());
    return 0;
}

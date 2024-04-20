#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await HTTPConnectionHTTPS::load_ca_certificates();

    auto conn = co_await http_connect("https://man7.org");
    for (int i = 0; i < 4; i++) {
        HTTPRequest req = {
            .method = "GET",
            .uri = URI::parse("/"),
        };
        co_await conn->write_header(req);
        co_await conn->write_nobody();
        HTTPResponse res;
        co_await conn->read_header(res);
        debug(), co_await conn->read_body(), i;
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

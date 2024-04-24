#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await https_load_ca_certificates();

    auto conn = co_await http_connect("https://api.openai.com");
    for (int i = 0; i < 4; i++) {
        HTTPRequest req = {
            .method = "GET",
            .uri = URI::parse("/"),
        };
        HTTPResponse res;
        std::string body;
        co_await conn.request(req, {}, res, body);
        debug(), body;
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

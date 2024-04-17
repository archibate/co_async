#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    SSLClientTrustAnchor ta;
    auto s = co_await SSLClientSocketStream::connect("localhost", 4433, ta);
    HTTPRequest req = {
        .method = "GET",
        .uri = URI::parse("/"),
    };
    co_await req.write_into(s);
    HTTPResponse res;
    co_await res.read_from(s);
    debug(), res;
}

int main() {
    co_synchronize(amain());
    return 0;
}

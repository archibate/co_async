/* #define CO_ASYNC_PERF 1 */
#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    SSLClientTrustAnchor ta;
    ta.add(co_await file_read(make_path("scripts/certificates/cert-root-rsa.pem")));
    HTTPProtocol http(co_await SSLClientSocketStream::connect("localhost", 4433, ta, {}, {}, std::chrono::seconds(5)));
    HTTPRequest req = {
        .method = "GET",
        .uri = URI::parse("/"),
    };
    co_await http.writeRequest(req);
    co_await http.write_nobody(req);
    HTTPResponse res;
    co_await http.readResponse(res);
    auto body = co_await http.readBody(res.encoding);
    debug(), body;
}

int main() {
    co_synchronize(amain());
    return 0;
}

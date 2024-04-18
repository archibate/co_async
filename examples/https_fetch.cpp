/* #define CO_ASYNC_PERF 1 */
#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    SSLClientTrustAnchor ta;
    ta.add(co_await file_read(make_path("/etc/ca-certificates/extracted/tls-ca-bundle.pem")));
    HTTP11 http(co_await SSLClientSocketStream::connect("man7.org", 443, ta));
    HTTPRequest req = {
        .method = "GET",
        .uri = URI::parse("/"),
        .headers = {
            {"host", "man7.org"},
            {"user-agent", "curl/8.7.1"},
            {"accept", "*/*"},
        },
    };
    co_await http.write_header(req);
    co_await http.write_nobody(req);
    HTTPResponse res;
    co_await http.read_header(res);
    debug(), co_await http.read_body(res);
}

int main() {
    co_synchronize(amain());
    return 0;
}

/* #define CO_ASYNC_PERF 1 */
#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <co_async/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    SSLServerCertificate cert;
    SSLPrivateKey pkey;
    pkey.decode(co_await file_read(make_path("scripts/certificates/key-ee-rsa.pem")));
    cert.add(co_await file_read(make_path("scripts/certificates/cert-ee-rsa.pem")));
    cert.add(co_await file_read(make_path("scripts/certificates/cert-ica-rsa.pem")));

    auto listener = co_await listener_bind({"localhost", 4433});
    while (true) {
        auto s = co_await SSLServerSocketStream::accept(listener, cert, pkey);
        HTTPRequest req;
        co_await req.read_from(s);
        HTTPResponse res = {
            .status = 200,
            .headers = {{"content-type", "text/html"}},
            .body = "<h1>It works!</h1>",
            .keepAlive = false,
        };
        co_await res.write_into(s);
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

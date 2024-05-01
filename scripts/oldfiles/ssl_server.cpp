/* #define CO_ASYNC_PERF 1 */
#include <co_async/utils/debug.hpp>
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    SSLServerCertificate cert;
    SSLPrivateKey pkey;
    pkey.set(co_await file_read(make_path("scripts/certificates/key-ee-rsa.pem")));
    cert.add(co_await file_read(make_path("scripts/certificates/cert-ee-rsa.pem")));
    cert.add(co_await file_read(make_path("scripts/certificates/cert-ica-rsa.pem")));

    auto listener = co_await listener_bind({"localhost", 4433});
    while (true) {
        HTTPProtocol http(co_await SSLServerSocketStream::accept(listener, cert, pkey));
        HTTPRequest req;
        co_await http.readRequest(req);
        co_await http.write_nobody(req);
        HTTPResponse res = {
            .status = 200,
            .headers = {{"content-type", "text/html"}},
        };
        co_await http.writeResponse(res);
        co_await http.writeBody(res.encoding, "<h1>It works!</h1>");
    }
}

int main() {
    co_synchronize(amain());
    return 0;
}

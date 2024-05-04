
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<void, std::errc>> amain() {
    co_await https_load_ca_certificates();
    HTTPConnectionPool pool;
    for (std::size_t i = 0; i < 3; ++i) {
        auto conn = co_await co_await pool.connect("https://api.openai.com");
        HTTPRequest req = {
            .method = "GET",
            .uri = URI::parse("/v1/models"),
            .headers = {
                {"authorization", "Bearer " + std::string(std::getenv("OPENAI_API_KEY") ?: "")},
            },
        };
        HTTPResponse res;
        std::string body;
        co_await co_await conn->request(req, {}, res, body);
        debug(), body;
    }
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

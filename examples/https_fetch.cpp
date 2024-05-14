#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    HTTPConnectionPool pool;
    std::vector<FutureSource<Expected<>>> res;
    for (std::string path: {"index.html", "style.css", "koru-icon.png", "mtk_2021_200.png"}) {
        res.push_back(co_future(co_bind([&, path] () -> Task<Expected<>> {
            auto conn = co_await co_await pool.connect("https://man7.org");
            HTTPRequest req = {
                .method = "GET",
                .uri = URI::parse("/" + path),
            };
            debug(), "requesting", req;
            auto [re, body] = co_await co_await conn->request(req, {});
            debug(), "response", res, "with", body.size(), "bytes";
            co_await co_await file_write(make_path("/tmp", path), body);
            co_return {};
        })));
    }
    co_await co_await res.back();
    res.pop_back();
    for (auto &&r: res) {
        co_await co_await r;
    }
    Pid pid = co_await co_await ProcessBuilder()
        .path("display")
        .arg("koru-icon.png")
        .spawn();
    co_await co_await wait_process(pid);
    co_return {};
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}

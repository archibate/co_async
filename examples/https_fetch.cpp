#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    HTTPConnectionPool pool;
    std::vector<Task<Expected<>>> res;
    for (std::string path: {"index.html", "style.css", "koru-icon.png", "mtk_2021_200.png"}) {
        res.push_back(co_bind([&, path] () -> Task<Expected<>> {
            auto conn = co_await co_await pool.connect("https://man7.org");
            HTTPRequest req = {
                .method = "GET",
                .uri = URI::parse("/" + path),
            };
            debug(), "requesting", req;
            auto [res, body] = co_await co_await conn->request_streamed(req, {});
            auto content = co_await co_await body.getall();
            co_await co_await file_write(make_path("/tmp", path), content);
            co_return {};
        }));
    }
    co_await co_await when_all(res);
    Pid pid = co_await co_await ProcessBuilder()
        .path("display")
        .arg("/tmp/koru-icon.png")
        .spawn();
    co_await co_await wait_process(pid);
    co_await co_await co_sleep(1s);
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

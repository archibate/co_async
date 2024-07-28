#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    co_await co_await stdio().putline("starting process: cat");

    auto p = co_await co_await fs_pipe();
    auto pid = co_await co_await ProcessBuilder()
                   .path("cat")
                   .open(0, p.reader())
                   .spawn();
    auto ws = file_from_handle(p.writer());
    co_await co_await ws.putline("Hello, world!");
    co_await ws.close();

    co_await co_await stdio().putline("waiting process...");
    auto res = co_await co_await wait_process(pid);
    co_await co_await stdio().putline("process exited: " + to_string(res.status));

    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

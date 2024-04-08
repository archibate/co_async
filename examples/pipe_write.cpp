#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await stdio().putline("starting process");

    auto p = co_await make_pipe();
    auto pid = co_await ProcessBuilder().path("cat").open(0, p.reader()).close_above().spawn();
    FileOStream ws(p.writer());
    co_await ws.putline("Hello, world!");
    co_await fs_close(ws.release());

    co_await stdio().putline("waiting process");
    auto res = co_await wait_process(pid);
    co_await stdio().putline("process exited: " + to_string(res.status));
}

int main() {
    co_spawn_and_wait(amain());
    return 0;
}

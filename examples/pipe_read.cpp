#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    co_await stdio().putline("starting process");

    auto p = co_await make_pipe();
    auto pid = co_await ProcessBuilder()
                   .path("cat")
                   .arg("CMakeLists.txt")
                   .open(1, p.writer())
                   .close_above()
                   .spawn();
    FileIStream rs(p.reader());
    std::string line;
    while (co_await rs.getline(line, '\n')) {
        co_await stdio().putline("process output: " + line);
        line.clear();
    }
    co_await fs_close(rs.release());

    co_await stdio().putline("waiting process");
    auto res = co_await wait_process(pid);
    co_await stdio().putline("process exited: " + to_string(res.status));
}

int main() {
    co_spawn_and_wait(amain());
    return 0;
}

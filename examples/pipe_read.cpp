#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    co_await co_await stdio().putline("starting process: cat CMakeLists.txt");

    auto p = co_await co_await fs_pipe();
    Pid pid = co_await co_await ProcessBuilder()
                   .path("cat")
                   .arg("CMakeLists.txt")
                   .open(1, p.writer())
                   .spawn();
    auto rs = file_from_handle(p.reader());
    String line;
    while (line.clear(), co_await rs.getline(line, '\n')) {
        co_await co_await stdio().putline("process output: " + line);
    }
    co_await rs.close();

    co_await co_await stdio().putline("waiting process...");
    auto res = co_await co_await wait_process(pid);
    co_await co_await stdio().putline("process exited: " + to_string(res.status));

    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto path = make_path("examples/main.cpp");
    auto w = co_await FileNotifier().add(path, FileNotifier::OnWriteFinished).wait();
    co_await stdio().putline(w.metadata);
}

int main() {
    co_spawn_and_wait(amain());
    return 0;
}

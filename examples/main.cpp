#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto path = make_path(".");
    auto w = co_await FileWatch().watch(path, FileWatch::OnWriteFinished, true).wait();
    co_await stdio().putline(w.path.string());
}

int main() {
    co_synchronize(amain());
    return 0;
}

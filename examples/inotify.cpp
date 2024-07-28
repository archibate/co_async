#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    auto path = make_path(".");
    auto w = co_await co_await FileWatch().watch(/*path=*/path, /*event=*/FileWatch::OnWriteFinished, /*recursive=*/true).wait();
    co_await co_await stdio().putline(w.path.string());
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

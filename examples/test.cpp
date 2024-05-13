#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

Task<int> compute(CancelToken cancel) {
    debug(), co_await co_sleep(200ms, cancel);
    co_return 42;
}

Task<> amain() {
    auto ret = co_await co_timeout(compute, 100ms);
    debug(), ret;
    co_return;
}

int main() {
    std::setlocale(LC_ALL, "");
    IOContext ctx;
    ctx.start_worker();
    ctx.join(amain());
}

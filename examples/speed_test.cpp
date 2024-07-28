#include <co_async/std.hpp>
#include <co_async/co_async.hpp>
#include <co_async/utils/perf.hpp>

using namespace co_async;
using namespace std::literals;

[[gnu::noinline]] static void empty_func() {
    return;
}

static Task<> empty_coroutine() {
    co_return;
}

static Task<> amain() {
    constexpr std::size_t n = 1000;
    for (std::size_t i = 0; i < n; i++) {
        Perf _;
        empty_func();
    }
    for (std::size_t i = 0; i < n; i++) {
        Perf _;
        co_await empty_coroutine();
    }
    for (std::size_t i = 0; i < n; i++) {
        Perf _;
        co_await fs_nop();
    }
}

int main() {
    co_main(amain());
    return 0;
}

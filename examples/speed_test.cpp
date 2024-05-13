#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#include <co_async/utils/perf.hpp>

using namespace co_async;
using namespace std::literals;

[[gnu::noinline]] void empty_func() {
    return;
}

Task<> empty_coroutine() {
    co_return;
}

Task<> amain() {
    constexpr std::size_t n = 1;
    for (std::size_t i = 0; i < n; i++) {
        Perf _;
        empty_func();
    }
    for (std::size_t i = 0; i < n; i++) {
        co_await empty_coroutine();
    }
    for (std::size_t i = 0; i < n; i++) {
        co_await fs_nop();
    }
}

int main() {
    IOContext().join(amain());
    return 0;
}

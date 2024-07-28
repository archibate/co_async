#define DEBUG_LEVEL 1
#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

[[maybe_unused]] static Task<GeneratorResult<int, Expected<>>> task1() {
    for (int i = 9; i <= 69; i++) {
        co_awaits co_sleep(300ms);
        co_yield i;
    }
    co_returns;
}

static Task<Expected<>> amain() {
    auto g = task1();
    while (auto r = co_awaits g) {
        debug(), r;
    }
    debug(), "end";
    co_returns;
}

int main() {
    co_main(amain());
    return 0;
}

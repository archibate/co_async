#define DEBUG_LEVEL 1
#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

[[maybe_unused]] static Task<GeneratorResult<int>> task1() {
    for (int i = 9; i <= 69; i++) {
        co_yield i;
    }
    co_return;
}

static Task<Expected<>> amain() {
    auto g = task1();
    while (auto r = co_await g) {
        debug(), r;
    }
    debug(), "end";
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}

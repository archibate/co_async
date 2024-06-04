#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

[[maybe_unused]] static Task<Expected<int>> task1() {
    co_await co_await co_sleep(100ms);
    co_return 1;
}

[[maybe_unused]] static Task<Expected<int>> task2() {
    co_await co_await co_sleep(200ms);
    co_return 2;
}

static Task<> amain() {
    debug(), co_await when_any(task1(), task2());
}

int main() {
    IOContext().join(amain());
    return 0;
}

/* #define DEBUG_STEPPING_METHOD 1 */
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Queue<int> q(4);

static Task<Expected<>> func() {
    (void)co_await co_sleep(1000ms);
    debug(), "PUSH 42";
    (void)co_await co_sleep(1000ms);
    debug(), "PUSH 64";
    co_return {};
}

static Task<Expected<>> amain() {
    co_spawn(func());
    for (int i = 0; i < 8; i++) {
        debug(), i;
        auto e = co_await co_timeout(q.pop(), 400ms);
        debug(), i, e;
    }
    co_return {};
}

int main() {
    std::setlocale(LC_ALL, "");
    co_main(amain());
    return 0;
}

/* #define DEBUG_STEPPING_METHOD 1 */
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

TimedQueue<int, 15> q;

static Task<Expected<>> func() {
    (void)co_await co_sleep(1000ms);
    debug(), "PUSH 42";
    co_await q.push(42);
    (void)co_await co_sleep(1000ms);
    debug(), "PUSH 64";
    co_await q.push(64);
    co_return {};
}

static Task<Expected<>> amain() {
    TaskGroup<Expected<>> group;
    group.add(func());
    for (int i = 0; i < 8; i++) {
        debug(), i;
        auto e = co_await q.pop(400ms);
        debug(), i, e;
    }
    co_await co_await group.wait();
    co_return {};
}

int main() {
    std::setlocale(LC_ALL, "");
    IOContext().join(amain()).value();
    return 0;
}

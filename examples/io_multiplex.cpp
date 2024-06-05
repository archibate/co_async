#define DEBUG_LEVEL 1
#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

static TimedQueue<char> cv(1);

[[maybe_unused]] static Task<Expected<>> task1() {
    while (true) {
        char c = co_await co_await raw_stdio().getchar();
        cv.try_push(std::move(c));
    }
}

static Task<Expected<>> amain() {
    co_spawn(task1());
    while (true) {
        auto r = co_await when_any(cv.pop(), co_sleep(500ms));
        debug(), r;
    }
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}

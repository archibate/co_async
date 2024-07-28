#include <co_async/std.hpp>
#include <co_async/co_async.hpp>
#include <co_async/utils/debug.hpp>

using namespace co_async;
using namespace std;

static Queue<char> q1(1);
static Queue<char> q2(1);

[[maybe_unused]] static Task<Expected<>> task1() {
    while (true) {
        char c = co_await co_await raw_stdio().getchar();
        co_await co_await q1.push(std::move(c));
    }
}

[[maybe_unused]] static Task<Expected<>> task2() {
    auto f = co_await co_await file_open("/dev/input/mouse0", OpenMode::Read);
    while (true) {
        char c = co_await co_await f.getchar();
        co_await co_await q2.push(std::move(c));
    }
}

static Task<Expected<>> amain() {
    co_spawn(task1());
    co_spawn(task2());
    while (true) {
        auto r = co_await (co_await when_any_common(q1.pop(), q2.pop())).value;
        debug(), r;
    }
}

int main() {
    co_main(amain());
    return 0;
}

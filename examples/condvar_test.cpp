#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

TimedConditionVariable cv;

Task<Expected<>> func() {
    (void)co_await co_sleep(1000ms);
    cv.notify();
    co_return {};
}

Task<Expected<>> amain() {
    co_spawn(func());
    auto e = co_await cv.wait(800ms);
    debug(), e;
    e = co_await cv.wait(800ms);
    debug(), e;
    e = co_await cv.wait(800ms);
    debug(), e;
    e = co_await cv.wait(800ms);
    debug(), e;
    e = co_await cv.wait(800ms);
    debug(), e;
    co_return {};
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}

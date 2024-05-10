#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

ConditionVariable cv;

Task<Expected<>> func() {
    co_await sleep_for(1000ms);
    cv.notify();
    co_return {};
}

Task<Expected<>> amain() {
    co_spawn(func());
    auto e = co_await cv.wait_for(800ms);
    debug(), e;
    e = co_await cv.wait_for(800ms);
    debug(), e;
    e = co_await cv.wait_for(800ms);
    debug(), e;
    e = co_await cv.wait_for(800ms);
    debug(), e;
    e = co_await cv.wait_for(800ms);
    debug(), e;
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

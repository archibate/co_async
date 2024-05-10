#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

TimedQueue<int, 15> q;

Task<Expected<>> func() {
    co_await sleep_for(1000ms);
    co_await q.push(42);
    co_await sleep_for(1000ms);
    co_await q.push(64);
    co_return {};
}

Task<Expected<>> amain() {
    auto fut = co_future(func());
    for (int i = 0; i < 8; i++) {
        debug(), i;
        auto e = co_await q.pop_for(400ms);
        debug(), i, e;
        /* co_await sleep_for(400ms); */
    }
    co_await co_await fut;
    co_await sleep_for(1000ms);
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

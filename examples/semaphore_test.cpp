#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Semaphore sem(2, 0);

static Task<Expected<>> func() {
    (void)co_await co_sleep(1000ms);
    debug(), sem.count();
    co_await co_await sem.release();
    debug(), sem.count();
    (void)co_await co_sleep(1000ms);
    debug(), sem.count();
    co_await co_await sem.release();
    debug(), sem.count();
    co_await co_await sem.release();
    debug(), sem.count();
    co_await co_await sem.release();
    debug(), sem.count();
    co_return {};
}

static Task<Expected<>> amain() {
    co_spawn(func());
    auto success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    success = co_await co_timeout(sem.acquire(), 800ms);
    debug(), success, sem.count();
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

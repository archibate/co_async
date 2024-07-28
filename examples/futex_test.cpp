#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

std::atomic<uint32_t> flag{0};

static Task<Expected<>> func() {
    (void)co_await co_sleep(1000ms);
    debug(), 3;
    flag.fetch_add(1);
    futex_notify(&flag);
    debug(), 4;
    co_return {};
}

static Task<Expected<>> amain() {
    co_spawn(func());
    debug(), 1;
    co_await co_await futex_wait(&flag, 0);
    debug(), 2;
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

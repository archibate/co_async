#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

ThreadPool pool;

static Task<Expected<>> func() {
    co_await co_await pool.run([] { std::this_thread::sleep_for(1s); });
    co_return {};
}

static Task<Expected<>> amain() {
    co_await when_all(func(), func(), func());
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

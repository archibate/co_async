#define DEBUG_LEVEL 1
#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

[[maybe_unused]] static Task<> amain() {
    /* auto res = co_await co_any([&] (CancelToken cancel) -> Task<Expected<char>> { */
    /*     co_return co_await stdio().getchar(); */
    /* }, [&] (CancelToken cancel) -> Task<Expected<>> { */
    /*     co_return co_await co_sleep(1s, cancel); */
    /* }); */
    debug(), co_await when_any(raw_stdio().getchar(), co_sleep(1s));
}

int main() {
    IOContext().join(amain());
    return 0;
}

#define DEBUG_LEVEL 1
#include <co_async/std.hpp>
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std;

static Task<Expected<>> amain() {
    while (true) {
        auto r = co_await when_any(raw_stdio().getchar(), co_sleep(500ms));
        if (r.index() == 0) {
            char c = co_await get<0>(r);
            debug(), c;
        } else {
            debug(), "无输入";
        }
    }
}

int main() {
    co_main(amain());
    return 0;
}

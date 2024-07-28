#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

static Task<int> compute() {
    auto res = co_await co_sleep(200ms);
    debug(), "睡眠结果", res;
    CancelToken cancel = co_await co_cancel;
    if (cancel.is_canceled())
        co_return 0;
    co_return 42;
}

static Task<> amain() {
    auto ret = co_await co_timeout(compute(), 100ms);
    debug(), "计算结果", ret;
    co_return;
}

int main() {
    std::setlocale(LC_ALL, "");
    co_main(amain());
}

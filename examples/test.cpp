#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

Task<int> compute(CancelToken cancel) {
    debug(), "睡眠结果", co_await co_sleep(200ms, cancel);
    if (cancel.is_canceled())
        co_return 0;
    co_return 42;
}

Task<> amain() {
    auto ret = co_await co_timeout(compute, 100ms);
    debug(), "计算结果", ret;
    co_return;
}

int main() {
    std::setlocale(LC_ALL, "");
    IOContext().join(amain());
}

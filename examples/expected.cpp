#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/utils/debug.hpp>

using namespace co_async;
using namespace std;

[[maybe_unused]] static Expected<int> succ() {
    return 42;
}

[[maybe_unused]] static Expected<int> fail() {
    return std::errc::stream_timeout;
}

[[maybe_unused]] static Task<Expected<>> test() {
    co_await succ();
    co_await fail();
    co_await succ();
    co_return {};
}

[[maybe_unused]] static Task<> amain() {
    auto ret = co_await test();
    debug(), ret;
}

int main() {
    co_main(amain());
    return 0;
}

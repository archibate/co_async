#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto i = co_await fs_nop();
    std::cout << "fs_nop result: " << i << '\n';
}

int main() {
    IOContext().join(amain());
    return 0;
}

#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto i = co_await fs_nop();
    std::cout << i << '\n';
}

int main() {
    co_synchronize(amain());
    return 0;
}

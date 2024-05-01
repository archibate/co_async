#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    auto is = co_await co_await file_open(make_path("/tmp/a.txt.gz"), OpenMode::Read);
    auto zs = make_stream<ZlibDecompressStreamRaw>(&is);
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

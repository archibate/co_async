#include <co_async/co_async.hpp>
#include <co_async/iostream/gzip_stream.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<void, std::errc>> amain() {
    auto infile = co_await co_await file_open("/tmp/a.txt.gz", OpenMode::Read);
    co_await co_await zlib_gunzip(infile, stdio());
    co_return {};
}

int main(int argc, char **argv) {
    co_synchronize(amain()).value();
    return 0;
}

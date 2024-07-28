#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    auto file = co_await co_await file_open("../examples/read_file.cpp", OpenMode::Read);
    String buffer = co_await co_await file.getall();
    co_await co_await stdio().putline(buffer);
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}


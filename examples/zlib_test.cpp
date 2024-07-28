#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain(bool decomp) {
    co_await co_await (decomp ? zlib_inflate : zlib_deflate)(stdio(), stdio());
    co_return {};
}

int main(int argc, char **argv) {
    bool decomp = false;
    if (argc > 1) {
        if (argv[1] == "decompress"sv) {
            decomp = true;
        } else if (argv[1] != "compress"sv) {
            std::cerr << "Usage: " << argv[0] << " [compress|decompress]\n";
            return 1;
        }
    }
    co_main(amain(decomp));
    return 0;
}

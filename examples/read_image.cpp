#include <co_async/co_async.hpp>/*{import co_async;}*/
#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto path = make_path("/home/bate/Downloads/Screenshot_2024-04-04_19-24-13.png");
    auto file = co_await fs_open(path, OpenMode::Read);
    auto size = co_await fs_stat_size(path);
    std::vector<char> buffer(size);
    co_await fs_read(file, buffer);
    int nx, ny, comp;
    stbi_uc *p = stbi_load_from_memory((stbi_uc const *)buffer.data(), buffer.size(), &nx, &ny, &comp, 0);
    co_await stdio().putline("image size: " + to_string(nx) + "x" + to_string(ny) + "x" + to_string(comp));
    stbi_image_free(p);
}

int main() {
    co_spawn_and_wait(amain());
    return 0;
}

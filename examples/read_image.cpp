#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    auto file = co_await co_await file_open("/home/bate/下载/5b1b42af22a09746f2a88345717599dbf698ad5c.jpg", OpenMode::Read);
    std::string buffer = co_await file.getall();
    int nx, ny, comp;
    std::shared_ptr<stbi_uc[]> image(
        stbi_load_from_memory((stbi_uc const *)buffer.data(), buffer.size(),
                              &nx, &ny, &comp, 0),
        stbi_image_free);
    co_await co_await stdio().putline("image size: " + to_string(nx) + "x" +
                                      to_string(ny) + "x" + to_string(comp));
    co_return {};
}

int main() {
    co_synchronize(amain()).value();
    return 0;
}

#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_STDIO
#define STBIDEF [[maybe_unused]] static
#include "stb_image.h"

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain() {
    auto file = co_await co_await file_open("/home/bate/下载/5b1b42af22a09746f2a88345717599dbf698ad5c.jpg", OpenMode::Read);
    String buffer = co_await co_await file.getall();
    int nx, ny, comp;
    std::shared_ptr<stbi_uc[]> image(
        stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(buffer.data()), static_cast<int>(buffer.size()),
                              &nx, &ny, &comp, 0),
        stbi_image_free);
    co_await co_await stdio().putline("image size: " + to_string(nx) + "x" +
                                      to_string(ny) + "x" + to_string(comp));
    co_return {};
}

int main() {
    co_main(amain());
    return 0;
}

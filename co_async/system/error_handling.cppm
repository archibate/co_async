module;

#ifdef __linux__
#include <linux/errno.h>
extern "C" [[gnu::const]] int *__errno_location(void) noexcept;
# define errno (*__errno_location())
#endif

export module co_async:system.error_handling;

import std;

namespace co_async {

#if CO_ASYNC_DEBUG
auto checkError(auto res, std::source_location const &loc =
                              std::source_location::current()) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category(),
                                (std::string)loc.file_name() + ":" +
                                    std::to_string(loc.line()));
    }
    return res;
}

auto bypassSpecificError(auto res, int bypassval = -ETIME) {
    if (res == bypassval) {
        res = 0;
    }
    return res;
}

auto checkErrorReturn(auto res, std::source_location const &loc =
                                    std::source_location::current()) {
    if (res < 0) [[unlikely]] {
        throw std::system_error(-res, std::system_category(),
                                (std::string)loc.file_name() + ":" +
                                    std::to_string(loc.line()));
    }
    return res;
}

auto checkErrorNonBlock(
    auto res, int blockres = 0, int blockerr = EWOULDBLOCK,
    std::source_location const &loc = std::source_location::current()) {
    if (res == -1) {
        if (errno != blockerr) [[unlikely]] {
            throw std::system_error(errno, std::system_category(),
                                    (std::string)loc.file_name() + ":" +
                                        std::to_string(loc.line()));
        }
        res = blockres;
    }
    return res;
}
#else
auto checkError(auto res) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category());
    }
    return res;
}

auto checkErrorReturn(auto res) {
    if (res < 0) [[unlikely]] {
        throw std::system_error(-res, std::system_category());
    }
    return res;
}

auto checkErrorNonBlock(auto res, int blockres = 0,
                        int blockerr = EWOULDBLOCK) {
    if (res == -1) {
        if (errno != blockerr) [[unlikely]] {
            throw std::system_error(errno, std::system_category());
        }
        res = blockres;
    }
    return res;
}
#endif

} // namespace co_async

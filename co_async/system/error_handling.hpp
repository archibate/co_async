#pragma once

#include <errno.h>
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {

/* #if CO_ASYNC_DEBUG */
/* auto checkError(auto res, */
/*                 std::source_location loc = std::source_location::current()) { */
/*     if (res == -1) [[unlikely]] { */
/*         throw std::system_error(errno, std::system_category(), */
/*                                 (std::string)loc.file_name() + ":" + */
/*                                     std::to_string(loc.line())); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorReturn( */
/*     auto res, std::source_location loc = std::source_location::current()) { */
/*     if (res < 0) [[unlikely]] { */
/*         throw std::system_error(-res, std::system_category(), */
/*                                 (std::string)loc.file_name() + ":" + */
/*                                     std::to_string(loc.line())); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorReturnCanceled( */
/*     auto res, int cancelres = 0, int cancelerr = -ECANCELED, */
/*     std::source_location loc = std::source_location::current()) { */
/*     if (res == cancelerr) { */
/*         return cancelres; */
/*     } */
/*     if (res < 0) [[unlikely]] { */
/*         throw std::system_error(-res, std::system_category(), */
/*                                 (std::string)loc.file_name() + ":" + */
/*                                     std::to_string(loc.line())); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorNonBlock( */
/*     auto res, int blockres = 0, int blockerr = EWOULDBLOCK, */
/*     std::source_location loc = std::source_location::current()) { */
/*     if (res == -1) { */
/*         if (errno != blockerr) [[unlikely]] { */
/*             throw std::system_error(errno, std::system_category(), */
/*                                     (std::string)loc.file_name() + ":" + */
/*                                         std::to_string(loc.line())); */
/*         } */
/*         res = blockres; */
/*     } */
/*     return res; */
/* } */
/* #else */
/* auto checkError(auto res) { */
/*     if (res == -1) [[unlikely]] { */
/*         throw std::system_error(errno, std::system_category()); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorReturn(auto res) { */
/*     if (res < 0) [[unlikely]] { */
/*         throw std::system_error(-res, std::system_category()); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorReturnCanceled(auto res, int cancelres = 0, */
/*                               int cancelerr = -ECANCELED) { */
/*     if (res == cancelerr) { */
/*         return cancelres; */
/*     } */
/*     if (res < 0) [[unlikely]] { */
/*         throw std::system_error(-res, std::system_category()); */
/*     } */
/*     return res; */
/* } */
/*  */
/* auto checkErrorNonBlock(auto res, int blockres = 0, */
/*                         int blockerr = EWOULDBLOCK) { */
/*     if (res == -1) { */
/*         if (errno != blockerr) [[unlikely]] { */
/*             throw std::system_error(errno, std::system_category()); */
/*         } */
/*         res = blockres; */
/*     } */
/*     return res; */
/* } */
/* #endif */

inline Expected<int, std::errc> expectError(int res) {
    if (res < 0) [[unlikely]] {
        return Unexpected{std::errc(-res)};
    }
    return res;
}

inline int throwingError(int res) {
    if (res < 0) [[unlikely]] {
        throw std::system_error(-res, std::system_category());
    }
    return res;
}

inline int throwingErrorErrno(int res) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category());
    }
    return res;
}

} // namespace co_async

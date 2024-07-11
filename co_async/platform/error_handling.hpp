#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
#if CO_ASYNC_DEBUG
inline Expected<int>
expectError(int res,
            std::source_location loc = std::source_location::current()) {
    if (res < 0) [[unlikely]] {
        return Expected<int>(std::errc(-res), loc);
    }
    return res;
}
#else
inline Expected<int> expectError(int res) {
    if (res < 0) [[unlikely]] {
        return std::errc(-res);
    }
    return res;
}
#endif

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

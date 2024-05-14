#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
inline Expected<int> expectError(int res) {
    if (res < 0) [[unlikely]] {
        return Unexpected{std::make_error_code(std::errc(-res))};
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

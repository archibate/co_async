#pragma once
#include <co_async/std.hpp>

namespace co_async {

template <class T>
constexpr T bruteForceByteSwap(T value) {
    if constexpr (sizeof(T) > 1) {
        char* ptr = reinterpret_cast<char*>(&value);
        for (size_t i = 0; i < sizeof(T) / 2; ++i) {
            std::swap(ptr[i], ptr[sizeof(T) - 1 - i]);
        }
    }
    return value;
}

template <class T>
    requires (std::is_trivial_v<T> && !std::is_integral_v<T>)
constexpr T byteswap(T value) {
    return bruteForceByteSwap(value);
}

template <class T>
    requires std::is_integral_v<T>
constexpr T byteswap(T value) {
#if __cpp_lib_byteswap
    return std::byteswap(value);
#elif defined(__GNUC__) && defined(__has_builtin)
#if __has_builtin(__builtin_bswap)
    return __builtin_bswap(value);
#else
    return bruteForceByteSwap(value);
#endif
#else
    return brute_force_byteswap(value);
#endif
}

#if __cpp_lib_endian // C++20 支持的 <bit> 头文件中可以方便地判断本地硬件的大小端
inline constexpr bool is_little_endian = std::endian::native == std::endian::little;
#else
#if _MSC_VER
#include <endian.h>
#if defined(__BYTE_ORDER) && __BYTE_ORDER != 0 && __BYTE_ORDER == __BIG_ENDIAN
inline constexpr bool is_little_endian = false;
#else
inline constexpr bool is_little_endian = true;
#endif
#else
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ != 0
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline constexpr bool is_little_endian = false;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline constexpr bool is_little_endian = true;
#else
inline constexpr bool is_little_endian = true;
#endif
#else
inline constexpr bool is_little_endian = true;
#endif
#endif
#endif

template <class T>
    requires std::is_trivial_v<T>
constexpr T byteswap_if_little(T value) {
    if constexpr (is_little_endian) {
        return byteswap(value);
    } else {
        return value;
    }
}

}

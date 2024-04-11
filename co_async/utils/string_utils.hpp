#pragma once/*{export module co_async:utils.string_utils;}*/

#include <co_async/std.hpp>/*{import std;}*/

namespace co_async {

template <class T>
struct from_string_t;

template <>
struct from_string_t<std::string> {
    std::string operator()(std::string_view s) const {
        return std::string(s);
    }
};

template <>
struct from_string_t<std::string_view> {
    std::string_view operator()(std::string_view s) const {
        return s;
    }
};

template <std::integral T>
struct from_string_t<T> {
    std::optional<T> operator()(std::string_view s, int base = 10) const {
        T result;
        auto [p, ec] =
            std::from_chars(s.data(), s.data() + s.size(), result, base);
        if (ec != std::errc()) [[unlikely]] {
            return std::nullopt;
        }
        if (p != s.data() + s.size()) [[unlikely]] {
            return std::nullopt;
        }
        return result;
    }
};

template <std::floating_point T>
struct from_string_t<T> {
    std::optional<T>
    operator()(std::string_view s,
               std::chars_format fmt = std::chars_format::general) const {
        T result;
        auto [p, ec] =
            std::from_chars(s.data(), s.data() + s.size(), result, fmt);
        if (ec != std::errc()) [[unlikely]] {
            return std::nullopt;
        }
        if (p != s.data() + s.size()) [[unlikely]] {
            return std::nullopt;
        }
        return result;
    }
};

/*[export]*/ template <class T>
inline constexpr from_string_t<T> from_string;

template <class T = void>
struct to_string_t;

template <>
struct to_string_t<void> {
    template <class U>
    void operator()(std::string &result, U &&value) const {
        to_string_t<std::decay_t<U>>()(result, std::forward<U>(value));
    }

    template <class U>
    std::string operator()(U &&value) const {
        std::string result;
        operator()(result, std::forward<U>(value));
        return result;
    }
};

template <>
struct to_string_t<std::string> {
    void operator()(std::string &result, std::string const &value) const {
        result.assign(value);
    }
};

template <>
struct to_string_t<std::string_view> {
    void operator()(std::string &result, std::string_view value) const {
        result.assign(value);
    }
};

template <std::integral T>
struct to_string_t<T> {
    void operator()(std::string &result, T value) const {
        result.resize(std::numeric_limits<T>::digits10 + 1, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(p - result.data());
    }
};

template <std::floating_point T>
struct to_string_t<T> {
    void operator()(std::string &result, T value) const {
        result.resize(std::numeric_limits<T>::max_digits10 + 1, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(p - result.data());
    }
};

/*[export]*/ inline constexpr to_string_t<> to_string;

/*[export]*/ inline std::string lower_string(std::string s) {
    for (auto &c: s) {
        if (c >= 'A' && c <= 'Z') {
            c += 'a' - 'A';
        }
    }
    return s;
}

/*[export]*/ inline std::string upper_string(std::string s) {
    for (auto &c: s) {
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }
    }
    return s;
}

} // namespace co_async

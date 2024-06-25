#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/allocator.hpp>

namespace co_async {
template <class T>
struct from_string_t;

template <class Traits, class Alloc>
struct from_string_t<std::basic_string<char, Traits, Alloc>> {
    std::basic_string<char, Traits, Alloc>
    operator()(std::string_view s) const {
        return std::basic_string<char, Traits, Alloc>(s);
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
template <class T>
inline constexpr from_string_t<T> from_string;
template <class T = void>
struct to_string_t;

template <>
struct to_string_t<void> {
    template <class U>
    void operator()(String &result, U &&value) const {
        to_string_t<std::decay_t<U>>()(result, std::forward<U>(value));
    }

    template <class U>
    String operator()(U &&value) const {
        String result;
        operator()(result, std::forward<U>(value));
        return result;
    }
};

template <>
struct to_string_t<String> {
    template <class Traits, class Alloc>
    void operator()(String &result,
                    std::basic_string<char, Traits, Alloc> const &value) const {
        result.assign(value);
    }
};

template <>
struct to_string_t<std::string_view> {
    void operator()(String &result, std::string_view value) const {
        result.assign(value);
    }
};

template <std::integral T>
struct to_string_t<T> {
    void operator()(String &result, T value) const {
        result.resize(std::numeric_limits<T>::digits10 + 2, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(std::size_t(p - result.data()));
    }
};

template <std::floating_point T>
struct to_string_t<T> {
    void operator()(String &result, T value) const {
        result.resize(std::numeric_limits<T>::max_digits10 + 2, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(p - result.data());
    }
};

inline constexpr to_string_t<> to_string;

inline String lower_string(std::string_view s) {
    String ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'A' && c <= 'Z') {
            c += 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline String upper_string(std::string_view s) {
    String ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline String trim_string(std::string_view s,
                          std::string_view trims = {" \t\r\n", 4}) {
    auto pos = s.find_first_not_of(trims);
    if (pos == std::string_view::npos) {
        return {};
    }
    auto end = s.find_last_not_of(trims);
    return String(s.substr(pos, end - pos + 1));
}

template <class Delim>
struct SplitString {
    SplitString(std::string_view s, Delim delimiter)
        : s(s),
          delimiter(delimiter) {}

    struct sentinel {
        explicit sentinel() = default;
    };

    struct iterator {
        explicit iterator(std::string_view s, Delim delimiter) noexcept
            : s(s),
              delimiter(delimiter),
              ended(false),
              toBeEnded(false) {
            find_next();
        }

        std::string_view operator*() const noexcept {
            return current;
        }

        std::string_view rest() const noexcept {
            return std::string_view{current.data(), current.size() + s.size()};
        }

        iterator &operator++() {
            find_next();
            return *this;
        }

        bool operator!=(sentinel) const noexcept {
            return !ended;
        }

        bool operator==(sentinel) const noexcept {
            return ended;
        }

        friend bool operator==(sentinel const &lhs,
                               iterator const &rhs) noexcept {
            return rhs == lhs;
        }

        friend bool operator!=(sentinel const &lhs,
                               iterator const &rhs) noexcept {
            return rhs != lhs;
        }

    private:
        void find_next() {
            auto pos = s.find(delimiter);
            if (pos == std::string_view::npos) {
                current = s;
                s = {};
                ended = toBeEnded;
                toBeEnded = true;
            } else {
                current = s.substr(0, pos);
                if constexpr (std::is_same_v<Delim, std::string_view>) {
                    s = s.substr(pos + delimiter.size());
                } else if constexpr (std::is_same_v<Delim, char>) {
                    s = s.substr(pos + 1);
                } else {
                    static_assert(!std::is_void_v<std::void_t<Delim>>);
                }
            }
        }

        std::string_view s;
        Delim delimiter;
        std::string_view current;
        bool ended;
        bool toBeEnded;
    };

    iterator begin() const noexcept {
        return iterator(s, delimiter);
    }

    sentinel end() const noexcept {
        return sentinel();
    }

    std::vector<String> collect() const {
        std::vector<String> result;
        for (auto &&part: *this) {
            result.emplace_back(part);
        }
        return result;
    }

    template <std::size_t N>
        requires(N > 0)
    std::array<String, N> collect() const {
        std::array<String, N> result;
        std::size_t i = 0;
        for (auto it = begin(); it != end(); ++it, ++i) {
            if (i + 1 >= N) {
                result[i] = String(it.rest());
                break;
            }
            result[i] = String(*it);
        }
        return result;
    }

private:
    std::string_view s;
    Delim delimiter;
};

inline SplitString<std::string_view> split_string(std::string_view s,
                                                  std::string_view delimiter) {
    return {s, delimiter};
}

inline SplitString<char> split_string(std::string_view s, char delimiter) {
    return {s, delimiter};
}
} // namespace co_async

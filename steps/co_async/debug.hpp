#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <source_location>
#include <type_traits>
#include <typeinfo>
#include <sstream>
#include <memory>
#include <unordered_map>
#if defined(__unix__) && __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#if !defined(NDEBUG)
struct debug {
private:
    std::ostringstream oss;

    enum {
        silent = 0,
        print = 1,
        panic = 2,
        supress = 3,
    } state;

    char const *line;
    std::source_location const &loc;

    static void uni_quotes(std::ostream &oss, std::string_view sv, char quote) {
        oss << quote;
        for (char c: sv) {
            switch (c) {
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\\': oss << "\\\\"; break;
            case '\0': oss << "\\0"; break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F) {
                    auto f = oss.flags();
                    oss << "\\x" << std::hex << std::setfill('0')
                        << std::setw(2) << static_cast<int>(c);
                    oss.flags(f);
                } else {
                    if (c == quote) {
                        oss << '\\';
                    }
                    oss << c;
                }
                break;
            }
        }
        oss << quote;
    }

    static std::string uni_demangle(char const *name) {
#if defined(__unix__) && __has_include(<cxxabi.h>)
        int status;
        char *p = abi::__cxa_demangle(name, 0, 0, &status);
        std::string s = p ? p : name;
        std::free(p);
#else
        std::string s = name;
#endif
        return s;
    }

    template <class T0>
    static void uni_format(std::ostream &oss, T0 &&t) {
        using T = std::decay_t<T0>;
        if constexpr (std::is_convertible_v<T, std::string_view> &&
                      !std::is_same_v<T, char const *>) {
            uni_quotes(oss, t, '"');
        } else if constexpr (std::is_same_v<T, bool>) {
            auto f = oss.flags();
            oss << std::boolalpha << t;
            oss.flags(f);
        } else if constexpr (std::is_same_v<T, char> ||
                             std::is_same_v<T, signed char>) {
            uni_quotes(oss, {reinterpret_cast<char const *>(&t), 1}, '\'');
        } else if constexpr (std::is_same_v<T, char8_t> ||
                             std::is_same_v<T, char16_t> ||
                             std::is_same_v<T, char32_t>) {
            auto f = oss.flags();
            oss << "'\\"
                << " xu U"[sizeof(T)] << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2) << std::uppercase
                << static_cast<std::uint32_t>(t) << "'";
            oss.flags(f);
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            auto f = oss.flags();
            oss << "0x" << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2) << std::uppercase;
            if constexpr (sizeof(T) == 1) {
                oss << static_cast<unsigned int>(t);
            } else {
                oss << t;
            }
            oss.flags(f);
        } else if constexpr (std::is_floating_point_v<T>) {
            auto f = oss.flags();
            oss << std::fixed
                << std::setprecision(std::numeric_limits<T>::digits10) << t;
            oss.flags(f);
        } else if constexpr (requires(std::ostream &oss, T0 &&t) {
                                 oss << std::forward<T0>(t);
                             }) {
            oss << std::forward<T0>(t);
        } else if constexpr (requires(T0 &&t) {
                                 std::to_string(std::forward<T0>(t));
                             }) {
            oss << std::to_string(std::forward<T0>(t));
        } else if constexpr (requires(T0 &&t) {
                                 std::begin(std::forward<T0>(t)) !=
                                     std::end(std::forward<T0>(t));
                             }) {
            oss << '{';
            bool add_comma = false;
            for (auto &&i: t) {
                if (add_comma)
                    oss << ", ";
                add_comma = true;
                uni_format(oss, std::forward<decltype(i)>(i));
            }
            oss << '}';
        } else if constexpr (requires(T0 &&t) {
                                 /* []<std::size_t... Is>( */
                                 /*     T &&t, std::index_sequence<Is...>) { */
                                 /*     void (*f)(...) = nullptr; */
                                 /*     f(std::get<Is>(t)...); */
                                 /* }(std::forward<T0>(t), */
                                 /*   std::make_index_sequence< */
                                 std::tuple_size<T>::value
                                     /* >{}) */
                                     ;
                             }) {
            oss << '{';
            bool add_comma = false;
            std::apply(
                [&](auto &&...args) {
                    (([&] {
                         if (add_comma)
                             oss << ", ";
                         add_comma = true;
                         (uni_format)(oss, std::forward<decltype(args)>(args));
                     }()),
                     ...);
                },
                t);
            oss << '}';
        } else if constexpr (std::is_enum_v<T>) {
            uni_format(oss, static_cast<std::underlying_type_t<T>>(t));
        } else if constexpr (std::is_same_v<T, std::type_info>) {
            oss << uni_demangle(t.name());
        } else if constexpr (requires(T0 &&t) { std::forward<T0>(t).repr(); }) {
            uni_format(oss, std::forward<T0>(t).repr());
        } else if constexpr (requires(T0 &&t) {
                                 std::forward<T0>(t).repr(oss);
                             }) {
            std::forward<T0>(t).repr(oss);
        } else if constexpr (requires(T0 &&t) { repr(std::forward<T0>(t)); }) {
            uni_format(oss, repr(std::forward<T0>(t)));
        } else if constexpr (requires(T0 &&t) {
                                 repr(oss, std::forward<T0>(t));
                             }) {
            repr(oss, std::forward<T0>(t));
        } else if constexpr (requires(T0 const &t) {
                                 (*t);
                                 (bool)t;
                             }) {
            if ((bool)t) {
                uni_format(oss, *t);
            } else {
                oss << "nil";
            }
        } else if constexpr (requires(T0 const &t) {
                                 visit([](auto const &) {}, t);
                             }) {
            visit([&oss](auto const &t) { uni_format(oss, t); }, t);
        } else {
            oss << '[' << uni_demangle(typeid(t).name()) << " at "
                << reinterpret_cast<void const *>(std::addressof(t)) << ']';
        }
    }

    debug &add_location_marks() {
        char const *fn = loc.file_name();
        for (char const *fp = fn; *fp; ++fp) {
            if (*fp == '/') {
                fn = fp + 1;
            }
        }
        oss << fn << ':' << loc.line() << ':' << '\t';
        if (line) {
            oss << '[' << line << ']' << '\t';
        } else {
#if 0
            static thread_local std::unordered_map<std::string, std::string> fileCache;
            auto key = std::to_string(loc.line()) + loc.file_name();
            if (auto it = fileCache.find(key); it != fileCache.end() && !it->second.empty()) [[likely]] {
                oss << '[' << it->second << ']';
            } else if (auto file = std::ifstream(loc.file_name()); file.is_open()) [[likely]] {
                std::string line;
                for (int i = 0; i < loc.line(); ++i) {
                    if (!std::getline(file, line)) [[unlikely]] {
                        line.clear();
                        break;
                    }
                }
                if (auto pos = line.find_first_not_of(" \t\r\n"); pos != line.npos) [[likely]] {
                    line = line.substr(pos);
                }
                if (!line.empty()) [[likely]] {
                    if (line.back() == ';') [[likely]] {
                        line.pop_back();
                    }
                    oss << '[' << line << ']';
                }
                fileCache.try_emplace(key, std::move(line));
            } else {
                oss << '[' << '?' << ']';
                fileCache.try_emplace(key);
            }
#endif
        }
        oss << ' ';
        return *this;
    }

    template <class T>
    struct debug_condition {
    private:
        debug &d;
        T const &t;

        template <class U>
        debug &check(bool cond, U const &u, char const *sym) {
            if (!cond) [[unlikely]] {
                d.on_error("assertion failed:") << t << sym << u;
            }
            return d;
        }

    public:
        explicit debug_condition(debug &d, T const &t) noexcept : d(d), t(t) {}

        template <class U>
        debug &operator<(U const &u) {
            return check(t < u, u, "<");
        }

        template <class U>
        debug &operator>(U const &u) {
            return check(t > u, u, ">");
        }

        template <class U>
        debug &operator<=(U const &u) {
            return check(t <= u, u, "<=");
        }

        template <class U>
        debug &operator>=(U const &u) {
            return check(t >= u, u, ">=");
        }

        template <class U>
        debug &operator==(U const &u) {
            return check(t == u, u, "==");
        }

        template <class U>
        debug &operator!=(U const &u) {
            return check(t != u, u, "!=");
        }
    };

    debug &on_error(char const *msg) {
        if (state != supress) {
            state = panic;
            add_location_marks();
        } else {
            oss << ' ';
        }
        oss << msg;
        return *this;
    }

    template <class T>
    debug &on_print(T &&t) {
        if (state == supress)
            return *this;
        if (state == silent) {
            state = print;
            add_location_marks();
        } else {
            oss << ' ';
        }
        uni_format(oss, std::forward<T>(t));
        return *this;
    }

public:
    debug(bool enable = true, char const *line = nullptr,
          std::source_location const &loc =
              std::source_location::current()) noexcept
        : state(enable ? silent : supress),
          line(line),
          loc(loc) {}

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug_condition<T> check(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    template <class T>
    debug_condition<T> operator>>(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    debug &fail(bool fail = true) {
        if (fail) [[unlikely]] {
            on_error("error:");
        } else {
            state = supress;
        }
        return *this;
    }

    debug &on(bool enable) {
        if (!enable) [[likely]] {
            state = supress;
        }
        return *this;
    }

    template <class T>
    debug &operator<<(T &&t) {
        return on_print(std::forward<T>(t));
    }

    template <class T>
    debug &operator,(T &&t) {
        return on_print(std::forward<T>(t));
    }

    ~debug() noexcept(false) {
        if (state == panic) [[unlikely]] {
            throw std::runtime_error(oss.str());
        }
        if (state == print) {
            oss << '\n';
            std::cerr << oss.str();
        }
    }
};
#else
struct debug {
    debug(bool = true, char const * = nullptr) noexcept {}

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug &operator,(T &&) {
        return *this;
    }

    template <class T>
    debug &operator<<(T &&) {
        return *this;
    }

    debug &on(bool) {
        return *this;
    }

    debug &fail(bool = true) {
        return *this;
    }

    ~debug() noexcept(false) {}

private:
    struct debug_condition {
        debug &d;

        explicit debug_condition(debug &d) : d(d) {}

        template <class U>
        debug &operator<(U const &) {
            return d;
        }

        template <class U>
        debug &operator>(U const &) {
            return d;
        }

        template <class U>
        debug &operator<=(U const &) {
            return d;
        }

        template <class U>
        debug &operator>=(U const &) {
            return d;
        }

        template <class U>
        debug &operator==(U const &) {
            return d;
        }

        template <class U>
        debug &operator!=(U const &) {
            return d;
        }
    };

public:
    template <class T>
    debug_condition check(T const &) noexcept {
        return debug_condition{*this};
    }

    template <class T>
    debug_condition operator>>(T const &) noexcept {
        return debug_condition{*this};
    }
};
#endif

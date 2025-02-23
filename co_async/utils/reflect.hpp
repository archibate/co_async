#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

/* #include <array> */
/* #include <cstddef> */
/* #include <cstdint> */
/* #include <map> */
/* #include <memory> */
/* #include <optional> */
/* #include <string> */
/* #include <string_view> */
/* #include <type_traits> */
/* #include <unordered_map> */
/* #include <variant> */
/* #include <vector> */

namespace co_async {
#if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
# define REFLECT(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT_GLOBAL(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT_GLOBAL_TEMPLATED(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT__PP_VA_OPT_SUPPORT(...) 0
#else
# define REFLECT__PP_CONCAT_(a, b)          a##b
# define REFLECT__PP_CONCAT(a, b)           REFLECT__PP_CONCAT_(a, b)
# define REFLECT__PP_GET_1(a, ...)          a
# define REFLECT__PP_GET_2(a, b, ...)       b
# define REFLECT__PP_GET_3(a, b, c, ...)    c
# define REFLECT__PP_GET_4(a, b, c, d, ...) d
# define REFLECT__PP_VA_EMPTY_(...)         REFLECT__PP_GET_2(__VA_OPT__(, ) 0, 1, )
# define REFLECT__PP_VA_OPT_SUPPORT         !REFLECT__PP_VA_EMPTY_
# if REFLECT__PP_VA_OPT_SUPPORT(?)
#  define REFLECT__PP_VA_EMPTY(...) REFLECT__PP_VA_EMPTY_(__VA_ARGS__)
# else
#  define REFLECT__PP_VA_EMPTY(...) 0
# endif
# define REFLECT__PP_IF(a, t, f)  REFLECT__PP_IF_(a, t, f)
# define REFLECT__PP_IF_(a, t, f) REFLECT__PP_IF__(a, t, f)
# define REFLECT__PP_IF__(a, t, f) \
     REFLECT__PP_IF___(REFLECT__PP_VA_EMPTY a, t, f)
# define REFLECT__PP_IF___(a, t, f)  REFLECT__PP_IF____(a, t, f)
# define REFLECT__PP_IF____(a, t, f) REFLECT__PP_IF_##a(t, f)
# define REFLECT__PP_IF_0(t, f)      REFLECT__PP_UNWRAP_BRACE(f)
# define REFLECT__PP_IF_1(t, f)      REFLECT__PP_UNWRAP_BRACE(t)
# define REFLECT__PP_NARG(...) \
     REFLECT__PP_IF((__VA_ARGS__), (0), \
                    (REFLECT__PP_NARG_(__VA_ARGS__, 26, 25, 24, 23, 22, 21, \
                                       20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
                                       10, 9, 8, 7, 6, 5, 4, 3, 2, 1)))
# define REFLECT__PP_NARG_(...) REFLECT__PP_NARG__(__VA_ARGS__)
# define REFLECT__PP_NARG__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                            _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, \
                            _23, _24, _25, _26, N, ...) \
     N
# define REFLECT__PP_FOREACH(f, ...) \
     REFLECT__PP_FOREACH_(REFLECT__PP_NARG(__VA_ARGS__), f, __VA_ARGS__)
# define REFLECT__PP_FOREACH_(N, f, ...) \
     REFLECT__PP_FOREACH__(N, f, __VA_ARGS__)
# define REFLECT__PP_FOREACH__(N, f, ...) \
     REFLECT__PP_FOREACH_##N(f, __VA_ARGS__)
# define REFLECT__PP_FOREACH_0(f, ...)
# define REFLECT__PP_FOREACH_1(f, a)             f(a)
# define REFLECT__PP_FOREACH_2(f, a, b)          f(a) f(b)
# define REFLECT__PP_FOREACH_3(f, a, b, c)       f(a) f(b) f(c)
# define REFLECT__PP_FOREACH_4(f, a, b, c, d)    f(a) f(b) f(c) f(d)
# define REFLECT__PP_FOREACH_5(f, a, b, c, d, e) f(a) f(b) f(c) f(d) f(e)
# define REFLECT__PP_FOREACH_6(f, a, b, c, d, e, g) \
     f(a) f(b) f(c) f(d) f(e) f(g)
# define REFLECT__PP_FOREACH_7(f, a, b, c, d, e, g, h) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h)
# define REFLECT__PP_FOREACH_8(f, a, b, c, d, e, g, h, i) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i)
# define REFLECT__PP_FOREACH_9(f, a, b, c, d, e, g, h, i, j) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j)
# define REFLECT__PP_FOREACH_10(f, a, b, c, d, e, g, h, i, j, k) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k)
# define REFLECT__PP_FOREACH_11(f, a, b, c, d, e, g, h, i, j, k, l) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l)
# define REFLECT__PP_FOREACH_12(f, a, b, c, d, e, g, h, i, j, k, l, m) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m)
# define REFLECT__PP_FOREACH_13(f, a, b, c, d, e, g, h, i, j, k, l, m, n) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n)
# define REFLECT__PP_FOREACH_14(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o)
# define REFLECT__PP_FOREACH_15(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) f(p)
# define REFLECT__PP_FOREACH_16(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q)
# define REFLECT__PP_FOREACH_17(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r)
# define REFLECT__PP_FOREACH_18(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s)
# define REFLECT__PP_FOREACH_19(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t)
# define REFLECT__PP_FOREACH_20(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u)
# define REFLECT__PP_FOREACH_21(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v)
# define REFLECT__PP_FOREACH_22(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w)
# define REFLECT__PP_FOREACH_23(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x)
# define REFLECT__PP_FOREACH_24(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x, y) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y)
# define REFLECT__PP_FOREACH_25(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x, y, z) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y) f(z)
# define REFLECT__PP_STRINGIFY(...)     REFLECT__PP_STRINGIFY(__VA_ARGS__)
# define REFLECT__PP_STRINGIFY_(...)    #__VA_ARGS__
# define REFLECT__PP_EXPAND(...)        REFLECT__PP_EXPAND_(__VA_ARGS__)
# define REFLECT__PP_EXPAND_(...)       __VA_ARGS__
# define REFLECT__PP_UNWRAP_BRACE(...)  REFLECT__PP_UNWRAP_BRACE_ __VA_ARGS__
# define REFLECT__PP_UNWRAP_BRACE_(...) __VA_ARGS__
# ifdef DEBUG_REPR
#  define REFLECT__EXTRA(...)        DEBUG_REPR(__VA_ARGS__)
#  define REFLECT_GLOBAL__EXTRA(...) DEBUG_REPR_GLOBAL(__VA_ARGS__)
#  define REFLECT_GLOBAL_TEMPLATED__EXTRA(...) \
      DEBUG_REPR_GLOBAL_TEMPLATED(__VA_ARGS__)
# else
#  define REFLECT__EXTRA(...)
#  define REFLECT_GLOBAL__EXTRA(...)
#  define REFLECT_GLOBAL_TEMPLATED__EXTRA(...)
# endif
# define REFLECT__ON_EACH(x) reflector(#x, x);
# define REFLECT(...) \
     template <class ReflectorT> \
     constexpr void REFLECT__MEMBERS(ReflectorT &reflector){ \
         REFLECT__PP_FOREACH(REFLECT__ON_EACH, \
                             __VA_ARGS__)} REFLECT__EXTRA(__VA_ARGS__)
# define REFLECT__GLOBAL_ON_EACH(x) \
     reflector(#x##_REFLECT__static_string, object.x);
# define REFLECT_GLOBAL(T, ...) \
     template <class ReflectorT> \
     constexpr void REFLECT__MEMBERS(ReflectorT &reflector, T &object){ \
         REFLECT__PP_FOREACH(REFLECT__GLOBAL_ON_EACH, \
                             __VA_ARGS__)} REFLECT_GLOBAL__EXTRA(__VA_ARGS__)
# define REFLECT_GLOBAL_TEMPLATED(T, Tmpls, TmplsClassed, ...) \
     template <class ReflectorT, REFLECT__PP_UNWRAP_BRACE(TmplsClassed)> \
     constexpr void REFLECT__MEMBERS( \
         ReflectorT &reflector, \
         T<REFLECT__PP_UNWRAP_BRACE(Tmpls)> &object){REFLECT__PP_FOREACH( \
         REFLECT__GLOBAL_ON_EACH, \
         __VA_ARGS__)} REFLECT_GLOBAL_TEMPLATED__EXTRA(__VA_ARGS__)
#endif
struct JsonValue {
    using Ptr = std::unique_ptr<JsonValue>;
    using Null = std::monostate;
    using String = std::string;
    using Dict = std::map<std::string, JsonValue::Ptr>;
    using Array = std::vector<JsonValue::Ptr>;
    using Integer = std::int64_t;
    using Real = double;
    using Boolean = bool;
    using Union =
        std::variant<Null, String, Dict, Array, Integer, Real, Boolean>;
    Union inner;

    template <class T>
    explicit JsonValue(std::in_place_type_t<T>, T &&value)
        : inner(std::in_place_type<T>, std::move(value)) {}

    template <class T>
    static Ptr make(T value) {
        return std::make_unique<JsonValue>(std::in_place_type<T>,
                                           std::move(value));
    }
};

/* template <class T> */
/* struct NoDefault { */
/* private: */
/*     T value; */
/*  */
/* public: */
/*     NoDefault(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>) */
/*         : value(std::move(value)) {} */
/*  */
/*     NoDefault(T const &value)
 * noexcept(std::is_nothrow_copy_constructible_v<T>) */
/*         : value(value) {} */
/*  */
/*     template <class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
 */
/*     NoDefault(U &&value) noexcept(std::is_nothrow_convertible_v<U, T>) */
/*         : value(std::forward<U>(value)) {} */
/*  */
/*     template <class = std::enable_if_t<std::is_convertible_v< */
/*                   std::initializer_list<typename T::value_type>, T>>> */
/*     NoDefault(std::initializer_list<typename T::value_type> value) noexcept(
 */
/*         std::is_nothrow_convertible_v< */
/*             std::initializer_list<typename T::value_type>, T>) */
/*         : value(std::move(value)) {} */
/*  */
/*     NoDefault(NoDefault const &) = default; */
/*     NoDefault &operator=(NoDefault const &) = default; */
/*     NoDefault(NoDefault &&) = default; */
/*     NoDefault &operator=(NoDefault &&) = default; */
/*  */
/*     T &operator*() noexcept { */
/*         return value; */
/*     } */
/*  */
/*     T const &operator*() const noexcept { */
/*         return value; */
/*     } */
/*  */
/*     T *operator->() noexcept { */
/*         return std::addressof(value); */
/*     } */
/*  */
/*     T const *operator->() const noexcept { */
/*         return std::addressof(value); */
/*     } */
/*  */
/*     using value_type = T; */
/* }; */

struct JsonEncoder;

template <class T, class = void>
struct JsonTrait {
    static void putValue(JsonEncoder *encoder, T const &value) {
        static_assert(!std::is_same_v<T, T>,
                      "the given type contains members that are not reflected, "
                      "please add REFLECT macro to it");
    }

    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        static_assert(!std::is_same_v<T, T>,
                      "the given type contains members that are not reflected, "
                      "please add REFLECT macro to it");
        return false;
    }
};

struct JsonEncoder {
    std::string json;

    void put(char c) {
        json.push_back(c);
    }

    void put(char const *s, std::size_t len) {
        json.append(s, len);
    }

    void putLiterialString(char const *name) {
        put('"');
        json.append(name);
        put('"');
    }

    void putString(char const *name, std::size_t len) {
        put('"');
        for (char const *it = name, *ep = name + len; it != ep; ++it) {
            char c = *it;
            switch (c) {
            case '\n': put("\\n", 2); break;
            case '\r': put("\\r", 2); break;
            case '\t': put("\\t", 2); break;
            case '\\': put("\\\\", 2); break;
            case '\0': put("\\0", 2); break;
            case '"':  put("\\\"", 2); break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F) {
                    put("\\u00", 4);
                    auto u = static_cast<unsigned char>(c);
                    put("0123456789abcdef"[u >> 4]);
                    put("0123456789abcdef"[u & 0x0F]);
                } else {
                    put(c);
                }
                break;
            }
        }
        put('"');
    }

    template <class T>
    void putArithmetic(T const &value) {
        json.append(std::to_string(value));
    }

    template <class T>
    void putValue(T const &value) {
        JsonTrait<T>::putValue(this, value);
    }
};
enum class JsonError : int {
    Success = 0,
    NullEntry,
    TypeMismatch,
    UnexpectedEnd,
    UnexpectedToken,
    NonTerminatedString,
    InvalidUTF16String,
    DictKeyNotString,
    InvalidNumberFormat,
    InvalidVariantType,
    NotImplemented,
};

inline std::error_category const &jsonCategory() {
    static struct : std::error_category {
        char const *name() const noexcept override {
            return "json";
        }

        std::string message(int e) const override {
            using namespace std::string_literals;
            switch (static_cast<JsonError>(e)) {
            case JsonError::Success:         return "success"s;
            case JsonError::NullEntry:       return "no such entry"s;
            case JsonError::TypeMismatch:    return "type mismatch"s;
            case JsonError::UnexpectedEnd:   return "unexpected end"s;
            case JsonError::UnexpectedToken: return "unexpected token"s;
            case JsonError::NonTerminatedString:
                return "non-terminated string"s;
            case JsonError::InvalidUTF16String: return "invalid utf-16 string"s;
            case JsonError::DictKeyNotString:   return "dict key must be string"s;
            case JsonError::InvalidNumberFormat:
                return "invalid number format"s;
            case JsonError::InvalidVariantType:
                return "invalid variant type"s;
            case JsonError::NotImplemented: return "not implemented"s;
            default:                        return "unknown error"s;
            }
        }
    } instance;

    return instance;
}

inline std::error_code make_error_code(JsonError e) {
    return std::error_code(static_cast<int>(e), jsonCategory());
}

inline JsonValue::Ptr jsonParse(std::string_view &json, std::error_code &ec) {
    using namespace std::string_view_literals;
    JsonValue::Ptr current;
    auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
    if (nonempty == json.npos) {
        ec = make_error_code(JsonError::UnexpectedEnd);
        return nullptr;
    }
    json.remove_prefix(nonempty);
    char c = json.front();
    if (c == '"') {
        json.remove_prefix(1);
        std::string str;
        unsigned int phase = 0;
        unsigned int lasthex = 0;
        unsigned int hex = 0;
        std::size_t i;
        auto unsignedExtent = [](unsigned int x) {
            return static_cast<char>(static_cast<unsigned char>(x));
        };
        for (i = 0;; ++i) {
            if (i == json.size()) {
                ec = make_error_code(JsonError::NonTerminatedString);
                return nullptr;
            }
            char c = json[i];
            if (phase == 0) {
                if (c == '"') {
                    break;
                } else if (c == '\\') {
                    phase = 1;
                    continue;
                }
            } else if (phase == 1) {
                if (c == 'u') {
                    phase = 2;
                    hex = 0;
                    lasthex = false;
                    continue;
                } else if (c == 'n') {
                    c = '\n';
                } else if (c == 't') {
                    c = '\t';
                } else if (c == '\\') {
                    c = '\\';
                } else if (c == '0') {
                    c = '\0';
                } else if (c == 'r') {
                    c = '\r';
                } else if (c == 'v') {
                    c = '\v';
                } else if (c == 'f') {
                    c = '\f';
                } else if (c == 'b') {
                    c = '\b';
                } else if (c == 'a') {
                    c = '\a';
                }
                phase = 0;
            } else {
                hex <<= 4;
                if ('0' <= c && c <= '9') {
                    hex |= static_cast<unsigned int>(c - '0');
                } else if ('a' <= c && c <= 'f') {
                    hex |= static_cast<unsigned int>(c - 'a' + 10);
                } else if ('A' <= c && c <= 'F') {
                    hex |= static_cast<unsigned int>(c - 'A' + 10);
                }
                if (phase == 5) {
                    if (0xD800 <= hex && hex < 0xDC00) {
                        if (!lasthex) {
                            phase = 2;
                            lasthex = hex;
                            hex = 0;
                            continue;
                        } else {
                            ec = make_error_code(JsonError::InvalidUTF16String);
                            return nullptr;
                        }
                    } else if (0xDC00 <= hex && hex < 0xE000) {
                        if (lasthex) {
                            hex = 0x10000 + (lasthex - 0xD800) * 0x400 +
                                  (hex - 0xDC00);
                            lasthex = false;
                            phase = 0;
                        } else {
                            ec = make_error_code(JsonError::InvalidUTF16String);
                            return nullptr;
                        }
                    }
                    if (hex <= 0x7F) {
                        str.push_back(unsignedExtent(hex));
                    } else if (hex <= 0x7FF) {
                        str.push_back(unsignedExtent(0xC0 | (hex >> 6)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0xFFFF) {
                        str.push_back(unsignedExtent(0xE0 | (hex >> 12)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0x10FFFF) {
                        str.push_back(unsignedExtent(0xF0 | (hex >> 18)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 12) & 0x3F)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else {
                        ec = make_error_code(JsonError::InvalidUTF16String);
                        return nullptr;
                    }
                    phase = 0;
                } else {
                    ++phase;
                }
                continue;
            }
            str.push_back(c);
        }
        json.remove_prefix(i + 1);
        current = JsonValue::make<JsonValue::String>(std::move(str));
    } else if (c == '{') {
        json.remove_prefix(1);
        std::map<std::string, JsonValue::Ptr> dict;
        for (;;) {
            nonempty = json.find_first_not_of(" \t\n\r\0"sv);
            if (nonempty == json.npos) {
                ec = make_error_code(JsonError::UnexpectedEnd);
                return nullptr;
            }
            json.remove_prefix(nonempty);
            if (json.front() == '}') {
                json.remove_prefix(1);
                break;
            } else if (json.front() == ',') {
                json.remove_prefix(1);
                continue;
            } else {
                auto key = jsonParse(json, ec);
                if (!key) {
                    return nullptr;
                }
                std::string keyString;
                if (auto p = std::get_if<JsonValue::String>(&key->inner)) {
                    keyString = std::move(*p);
                } else {
                    ec = make_error_code(JsonError::DictKeyNotString);
                    return nullptr;
                }
                auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
                if (nonempty == json.npos) {
                    ec = make_error_code(JsonError::UnexpectedEnd);
                    return nullptr;
                }
                json.remove_prefix(nonempty);
                if (json.front() != ':') {
                    ec = make_error_code(JsonError::UnexpectedToken);
                    return nullptr;
                }
                json.remove_prefix(1);
                auto value = jsonParse(json, ec);
                if (!value) {
                    return nullptr;
                }
                dict.emplace(std::move(keyString), std::move(value));
            }
        }
        current = JsonValue::make<JsonValue::Dict>(std::move(dict));
    } else if (c == '[') {
        json.remove_prefix(1);
        std::vector<JsonValue::Ptr> array;
        for (;;) {
            auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
            if (nonempty == json.npos) {
                ec = make_error_code(JsonError::UnexpectedEnd);
                return nullptr;
            }
            json.remove_prefix(nonempty);
            if (json.front() == ']') {
                json.remove_prefix(1);
                break;
            } else if (json.front() == ',') {
                json.remove_prefix(1);
                continue;
            } else {
                auto value = jsonParse(json, ec);
                if (!value) {
                    return nullptr;
                }
                array.emplace_back(std::move(value));
            }
        }
        current = JsonValue::make<JsonValue::Array>(std::move(array));
    } else if (('0' <= c && c <= '9') || c == '.' || c == '-' || c == '+') {
        auto end = json.find_first_of(",]}"sv);
        if (end == json.npos) {
            end = json.size();
        }
        auto str = std::string(json.data(), end);
        if (str.find('.') != str.npos) {
            double value;
            try {
                value = std::stod(str);
            } catch (std::exception const &) {
                ec = make_error_code(JsonError::InvalidNumberFormat);
                return nullptr;
            }
            current = JsonValue::make<JsonValue::Real>(value);
        } else {
            std::int64_t value;
            try {
                value = std::stoll(str);
            } catch (std::exception const &) {
                ec = make_error_code(JsonError::InvalidNumberFormat);
                return nullptr;
            }
            current = JsonValue::make<JsonValue::Integer>(value);
        }
        json.remove_prefix(end);
    } else if (c == 't') {
        if (!json.starts_with("true"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Boolean>(true);
        json.remove_prefix(4);
    } else if (c == 'f') {
        if (!json.starts_with("false"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Boolean>(false);
        json.remove_prefix(5);
    } else if (c == 'n') {
        if (!json.starts_with("null"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Null>(JsonValue::Null());
        json.remove_prefix(4);
    } else {
        ec = make_error_code(JsonError::UnexpectedToken);
        return nullptr;
    }
    return current;
}

struct ReflectorJsonEncode {
    JsonEncoder *encoder;
    bool comma = false;

    template <class T>
    void operator()(char const *name, T &value) {
        if (!comma) {
            comma = true;
        } else {
            encoder->put(',');
        }
        encoder->putLiterialString(name);
        encoder->put(':');
        encoder->putValue(value);
    }
};

struct ReflectorJsonDecode {
    JsonValue::Dict *currentDict;
    std::error_code ec{};
    bool failed = false;

    template <class T>
    void operator()(char const *name, T &value) {
        if (failed) {
            return;
        }
        auto it = currentDict->find(name);
        if (it == currentDict->end()) {
            JsonValue::Union nullData;
            failed = !JsonTrait<T>::getValue(nullData, value, ec);
        } else {
            failed = !JsonTrait<T>::getValue(it->second->inner, value, ec);
        }
    }

    static void typeMismatch(char const *expect, JsonValue::Union const &inner,
                             std::error_code &ec) {
        if (std::holds_alternative<JsonValue::Null>(inner)) {
#if DEBUG_LEVEL
            std::cerr << std::string("json_decode no such entry (expect ") +
                             expect + ", got null)\n";
#endif
            ec = make_error_code(JsonError::NullEntry);
        } else {
            char const *got = "???";
            std::visit(
                [&](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, JsonValue::Null>) {
                        got = "null";
                    } else if constexpr (std::is_same_v<T, JsonValue::String>) {
                        got = "string";
                    } else if constexpr (std::is_same_v<T, JsonValue::Dict>) {
                        got = "dict";
                    } else if constexpr (std::is_same_v<T, JsonValue::Array>) {
                        got = "array";
                    } else if constexpr (std::is_same_v<T,
                                                        JsonValue::Integer>) {
                        got = "integer";
                    } else if constexpr (std::is_same_v<T, JsonValue::Real>) {
                        got = "real";
                    } else if constexpr (std::is_same_v<T,
                                                        JsonValue::Boolean>) {
                        got = "boolean";
                    }
                },
                inner);
#if DEBUG_LEVEL
            std::cerr << std::string("json_decode type mismatch (expect ") +
                             expect + ", got " + got + ")\n";
#endif
            ec = make_error_code(JsonError::TypeMismatch);
        }
    }
};

struct JsonTraitPointerLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value == nullptr) {
            encoder->put("null", 4);
        } else {
            JsonTrait<typename std::pointer_traits<T>::element_type>::putValue(
                encoder, *value);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union const &inner, T &value,
                         std::error_code &ec) {
        JsonTrait<typename std::pointer_traits<T>::element_type>::getValue(
            inner, *value, ec);
        return !ec;
    }
};

struct JsonTraitArrayLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        auto bit = value.begin();
        auto eit = value.end();
        encoder->put('[');
        bool comma = false;
        for (auto it = bit; it != eit; ++it) {
            if (!comma) {
                comma = true;
            } else {
                encoder->put(',');
            }
            JsonTrait<typename T::value_type>::putValue(encoder, *it);
        }
        encoder->put(']');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Array>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.emplace_back();
                if (!JsonTrait<typename T::value_type>::getValue((*it)->inner,
                                                                 element, ec)) {
                    return false;
                }
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("array", data, ec);
            return false;
        }
    }
};

struct JsonTraitDictLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        auto bit = value.begin();
        auto eit = value.end();
        encoder->put('{');
        bool comma = false;
        for (auto it = bit; it != eit; ++it) {
            if (!comma) {
                comma = true;
            } else {
                encoder->put(',');
            }
            encoder->putString(it->first.data(), it->first.size());
            encoder->put(':');
            JsonTrait<typename T::mapped_type>::putValue(encoder, it->second);
        }
        encoder->put('}');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.try_emplace(it->first).first->second;
                if (!JsonTrait<typename T::mapped_type>::getValue(
                        it->second->inner, element, ec)) {
                    return false;
                }
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("dict", data, ec);
            return false;
        }
    }
};

struct JsonTraitStringLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putString(value.data(), value.size());
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::String>(&data)) {
            value = std::move(*p);
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("string", data, ec);
            return false;
        }
    }
};

struct JsonTraitNullLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->put("null", 4);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (std::get_if<JsonValue::Null>(&data)) {
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("null", data, ec);
            return false;
        }
    }
};

struct JsonTraitOptionalLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value) {
            encoder->putValue(*value);
        } else {
            encoder->put("null", 4);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (std::get_if<JsonValue::Null>(&data)) {
            value = std::nullopt;
            return true;
        } else {
            return JsonTrait<typename T::value_type>::getValue(
                data, value.emplace(), ec);
        }
    }
};

struct JsonTraitBooleanLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value) {
            encoder->put("true", 4);
        } else {
            encoder->put("false", 5);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Boolean>(&data)) {
            value = *p;
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("boolean", data, ec);
            return false;
        }
    }
};

struct JsonTraitArithmeticLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putArithmetic(value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Integer>(&data)) {
            value = static_cast<T>(*p);
            return true;
        } else if (auto p = std::get_if<JsonValue::Real>(&data)) {
            value = static_cast<T>(*p);
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("integer or real", data, ec);
            return false;
        }
    }
};

struct JsonTraitVariantLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        std::visit([&](auto const &arg) {
            using Arg = std::decay_t<decltype(arg)>;
            encoder->put('{');
            encoder->putLiterialString("type");
            encoder->put(':');
            encoder->putLiterialString(Arg::name);
            encoder->put(',');
            encoder->putLiterialString("object");
            encoder->put(':');
            encoder->putValue(arg);
            encoder->put('}');
        }, value);
    }

    template <class T, std::size_t ...Is>
    static bool getValueImpl(JsonValue::String const &name,
                             JsonValue::Union &object, T &value,
                             std::error_code &ec, std::index_sequence<Is...>) {
        int ret = 0;
        (void)((name == std::variant_alternative_t<Is, T>::name ?
            (ret = (JsonTrait<std::variant_alternative_t<Is, T>>
             ::getValue(object, value.template emplace<Is>(), ec) ? 2 : 1), true) :
            false) || ...);
        switch (ret) {
            case 2: return true;
            case 1: return false;
            default:
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            if (auto type = p->find("type"); type == p->end()) {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            } else if (auto object = p->find("object"); object == p->end()) {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            } else if (auto p = std::get_if<JsonValue::String>(&type->second->inner)) {
                if (!getValueImpl(*p, object->second->inner, value, ec,
                                  std::make_index_sequence<std::variant_size_v<T>>())) {
                    return false;
                }
                return true;
            } else {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            }
        } else {
            ReflectorJsonDecode::typeMismatch("object", data, ec);
            return false;
        }
    }
};

struct JsonTraitJsonValueLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putValue(value.inner);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, JsonValue &value,
                         std::error_code &ec) {
        value.inner = std::move(data);
        return true;
    }
};

struct JsonTraitObjectLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        ReflectorJsonEncode reflector(encoder);
        encoder->put('{');
        reflect_members(reflector, const_cast<T &>(value));
        encoder->put('}');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            ReflectorJsonDecode reflector(p);
            reflect_members(reflector, value);
            if (reflector.failed) {
                ec = reflector.ec;
                return false;
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("object", data, ec);
            return false;
        }
    }
};

struct JsonTraitWrapperLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        return JsonTrait<typename T::value_type>::putValue(encoder, *value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        return JsonTrait<typename T::value_type>::getValue(data, *value, ec);
    }
};

template <>
struct JsonTrait<JsonValue> {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        std::visit([&]<class U>(
                       U const &arg) { JsonTrait<U>::getValue(encoder, arg); },
                   value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, JsonValue &value,
                         std::error_code &ec) {
        value.inner = std::move(data);
        return true;
    }
};

template <class T>
struct JsonTrait<T *> : JsonTraitPointerLike {};

template <class T, class Deleter>
struct JsonTrait<std::unique_ptr<T, Deleter>> : JsonTraitPointerLike {};

template <class T>
struct JsonTrait<std::shared_ptr<T>> : JsonTraitPointerLike {};

template <class T, std::size_t N>
struct JsonTrait<std::array<T, N>> : JsonTraitArrayLike {};

template <class T, class Alloc>
struct JsonTrait<std::vector<T, Alloc>> : JsonTraitArrayLike {};

template <class K, class V, class Cmp, class Alloc>
struct JsonTrait<std::map<K, V, Cmp, Alloc>> : JsonTraitDictLike {};

template <class K, class V, class Hash, class Eq, class Alloc>
struct JsonTrait<std::unordered_map<K, V, Hash, Eq, Alloc>>
    : JsonTraitDictLike {};

template <class Traits, class Alloc>
struct JsonTrait<std::basic_string<char, Traits, Alloc>> : JsonTraitStringLike {
};

template <class Traits>
struct JsonTrait<std::basic_string_view<char, Traits>> : JsonTraitStringLike {};

template <class... Ts>
struct JsonTrait<std::variant<Ts...>> : JsonTraitVariantLike {};

template <class T>
struct JsonTrait<std::optional<T>> : JsonTraitOptionalLike {};

template <>
struct JsonTrait<std::nullptr_t> : JsonTraitNullLike {};

template <>
struct JsonTrait<std::nullopt_t> : JsonTraitNullLike {};

template <>
struct JsonTrait<std::monostate> : JsonTraitNullLike {};

template <>
struct JsonTrait<bool> : JsonTraitBooleanLike {};

/* template <class T> */
/* struct JsonTrait<NoDefault<T>> : JsonTraitWrapperLike {}; */

template <class T>
struct JsonTrait<T, std::enable_if_t<std::is_arithmetic_v<T>>>
    : JsonTraitArithmeticLike {};

template <class Reflector, class T>
inline std::void_t<
    decltype(std::declval<T &>().REFLECT__MEMBERS(std::declval<Reflector &>()))>
reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(reflector);
}

template <class Reflector, class T,
          class = std::void_t<decltype(REFLECT__MEMBERS(
              std::declval<T &>(), std::declval<Reflector &>()))>>
inline void reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(value, reflector);
}

template <class T>
struct JsonTrait<T, JsonValue> : JsonTraitJsonValueLike {};

template <class T>
struct JsonTrait<
    T, std::void_t<decltype(reflect_members(
           std::declval<ReflectorJsonEncode &>(), std::declval<T &>()))>>
    : JsonTraitObjectLike {};

template <class T>
inline std::string json_encode(T const &value) {
    JsonEncoder encoder;
    encoder.putValue(value);
    return encoder.json;
}

template <class T>
inline bool json_decode(JsonValue &root, T &value, std::error_code &ec) {
    return JsonTrait<T>::getValue(root.inner, value, ec);
}

template <class T>
inline bool json_decode(std::string_view json, T &value, std::error_code &ec) {
    auto root = jsonParse(json, ec);
    if (!root) {
        return false;
    }
    return json_decode(*root, value, ec);
}

template <class T>
inline Expected<T> json_decode(JsonValue &root) {
    T value{};
    std::error_code ec;
    if (!json_decode(root, value, ec)) [[unlikely]] {
        return ec;
    }
    return std::move(value);
}

template <class T>
inline Expected<T> json_decode(std::string_view json) {
    T value{};
    std::error_code ec;
    if (!json_decode(json, value, ec)) [[unlikely]] {
        return ec;
    }
    return std::move(value);
}

} // namespace co_async

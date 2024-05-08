#pragma once

#include "co_async/utils/debug.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <array>
#include <memory>
#include <variant>
#include <map>
#include <unordered_map>

namespace reflect {

#if __cpp_constexpr >= 201703L
#define REFLECT__CONSTEXPR17 constexpr
#else
#define REFLECT__CONSTEXPR17
#endif

#if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#define REFLECT(...) __pragma(message("Please turn on /Zc:preprocessor before using REFLECT!"))
#define REFLECT_GLOBAL(...) __pragma(message("Please turn on /Zc:preprocessor before using REFLECT!"))
#define REFLECT_GLOBAL_TEMPLATED(...) __pragma(message("Please turn on /Zc:preprocessor before using REFLECT!"))
#define REFLECT__PP_VA_OPT_SUPPORT(...) 0
#else

#define REFLECT__PP_CONCAT_(a, b) a##b
#define REFLECT__PP_CONCAT(a, b) REFLECT__PP_CONCAT_(a, b)

#define REFLECT__PP_GET_1(a, ...) a
#define REFLECT__PP_GET_2(a, b, ...) b
#define REFLECT__PP_GET_3(a, b, c, ...) c
#define REFLECT__PP_GET_4(a, b, c, d, ...) d
#define REFLECT__PP_GET_5(a, b, c, d, e, ...) e
#define REFLECT__PP_GET_6(a, b, c, d, e, f, ...) f
#define REFLECT__PP_GET_7(a, b, c, d, e, f, g, ...) g
#define REFLECT__PP_GET_8(a, b, c, d, e, f, g, h, ...) h
#define REFLECT__PP_GET_9(a, b, c, d, e, f, g, h, i, ...) i
#define REFLECT__PP_GET_10(a, b, c, d, e, f, g, h, i, j, ...) j

#define REFLECT__PP_VA_EMPTY_(...) REFLECT__PP_GET_2(__VA_OPT__(,)0,1,)
#define REFLECT__PP_VA_OPT_SUPPORT ! REFLECT__PP_VA_EMPTY_

#if REFLECT__PP_VA_OPT_SUPPORT(?)
#define REFLECT__PP_VA_EMPTY(...) REFLECT__PP_VA_EMPTY_(__VA_ARGS__)
#else
#define REFLECT__PP_VA_EMPTY(...) 0
#endif
#define REFLECT__PP_IF(a, t, f) REFLECT__PP_IF_(a, t, f)
#define REFLECT__PP_IF_(a, t, f) REFLECT__PP_IF__(a, t, f)
#define REFLECT__PP_IF__(a, t, f) REFLECT__PP_IF___(REFLECT__PP_VA_EMPTY a, t, f)
#define REFLECT__PP_IF___(a, t, f) REFLECT__PP_IF____(a, t, f)
#define REFLECT__PP_IF____(a, t, f) REFLECT__PP_IF_##a(t, f)
#define REFLECT__PP_IF_0(t, f) REFLECT__PP_UNWRAP_BRACE(f)
#define REFLECT__PP_IF_1(t, f) REFLECT__PP_UNWRAP_BRACE(t)

#define REFLECT__PP_NARG(...) REFLECT__PP_IF((__VA_ARGS__), (0), (REFLECT__PP_NARG_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)))
#define REFLECT__PP_NARG_(...) REFLECT__PP_NARG__(__VA_ARGS__)
#define REFLECT__PP_NARG__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

#define REFLECT__PP_FOREACH(f, ...) REFLECT__PP_FOREACH_(REFLECT__PP_NARG(__VA_ARGS__), f, __VA_ARGS__)
#define REFLECT__PP_FOREACH_(N, f, ...) REFLECT__PP_FOREACH__(N, f, __VA_ARGS__)
#define REFLECT__PP_FOREACH__(N, f, ...) REFLECT__PP_FOREACH_##N(f, __VA_ARGS__)
#define REFLECT__PP_FOREACH_0(f, ...)
#define REFLECT__PP_FOREACH_1(f, a) f(a)
#define REFLECT__PP_FOREACH_2(f, a, b) f(a) f(b)
#define REFLECT__PP_FOREACH_3(f, a, b, c) f(a) f(b) f(c)
#define REFLECT__PP_FOREACH_4(f, a, b, c, d) f(a) f(b) f(c) f(d)
#define REFLECT__PP_FOREACH_5(f, a, b, c, d, e) f(a) f(b) f(c) f(d) f(e)
#define REFLECT__PP_FOREACH_6(f, a, b, c, d, e, g) f(a) f(b) f(c) f(d) f(e) f(g)
#define REFLECT__PP_FOREACH_7(f, a, b, c, d, e, g, h) f(a) f(b) f(c) f(d) f(e) f(g) f(h)
#define REFLECT__PP_FOREACH_8(f, a, b, c, d, e, g, h, i) f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i)
#define REFLECT__PP_FOREACH_9(f, a, b, c, d, e, g, h, i, j) f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j)
#define REFLECT__PP_FOREACH_10(f, a, b, c, d, e, g, h, i, j, k) f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k)

#define REFLECT__PP_EXPAND(...) REFLECT__PP_EXPAND_(__VA_ARGS__)
#define REFLECT__PP_EXPAND_(...) __VA_ARGS__

#define REFLECT__PP_UNWRAP_BRACE(...) REFLECT__PP_UNWRAP_BRACE_ __VA_ARGS__
#define REFLECT__PP_UNWRAP_BRACE_(...) __VA_ARGS__

#define REFLECT__ON_EACH(x) reflector.member(#x, x);
#define REFLECT(...) \
template <class ReflectorT> \
REFLECT__CONSTEXPR17 void REFLECT__MEMBERS(ReflectorT &reflector) { \
    REFLECT__PP_FOREACH(REFLECT__ON_EACH, __VA_ARGS__) \
}

#define REFLECT__GLOBAL_ON_EACH(x) reflector.member(#x##_REFLECT__static_string, object.x);
#define REFLECT_GLOBAL(T, ...) \
template <class ReflectorT> \
REFLECT__CONSTEXPR17 void REFLECT__MEMBERS(ReflectorT &reflector, T &object) { \
    REFLECT__PP_FOREACH(REFLECT__GLOBAL_ON_EACH, __VA_ARGS__) \
}

#define REFLECT_GLOBAL_TEMPLATED(T, Tmpls, TmplsClassed, ...) \
template <class ReflectorT, REFLECT__PP_UNWRAP_BRACE(TmplsClassed)> \
REFLECT__CONSTEXPR17 void REFLECT__MEMBERS(ReflectorT &reflector, T<REFLECT__PP_UNWRAP_BRACE(Tmpls)> &object) { \
    REFLECT__PP_FOREACH(REFLECT__GLOBAL_ON_EACH, __VA_ARGS__) \
}
#endif

template <class T, class = void>
struct JsonTrait;

struct JsonEncoder {
    std::string json;

    void put(char c) {
        json.push_back(c);
    }

    void put(const char *s, std::size_t len) {
        json.append(s, len);
    }

    void putLiterialString(const char *name) {
        put('"');
        json.append(name);
        put('"');
    }

    void putString(const char *name, std::size_t len) {
        put('"');
        for (char const *it = name, *ep = name + len; it != ep; ++it) {
            char c = *it;
            switch (c) {
            case '\n': put("\\n", 2); break;
            case '\r': put("\\r", 2); break;
            case '\t': put("\\t", 2); break;
            case '\\': put("\\\\", 2); break;
            case '\0': put("\\0", 2); break;
            case '"': put("\\\"", 2); break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F) {
                    put("\\u00", 4);
                    auto u = static_cast<unsigned char>(c);
                    put("0123456789abcdef"[u & 0x0F]);
                    put("0123456789abcdef"[u >> 4]);
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

struct JsonDecoder {
    struct Value {
        using Ptr = std::unique_ptr<Value>;

        using Null = std::monostate;
        using String = std::string;
        using Dict = std::map<std::string, Value::Ptr>;
        using Array = std::vector<Value::Ptr>;
        using Integer = std::int64_t;
        using Real = double;
        using Boolean = bool;
        using Union = std::variant<Null, String, Dict, Array, Integer, Real, Boolean>;

        Union data;

        template <class T>
        explicit Value(std::in_place_type_t<T>, T &&value) : data(std::in_place_type<T>, std::move(value)) {}

        template <class T>
        static Ptr make(T value) {
            return std::make_unique<Value>(std::in_place_type<T>, std::move(value));
        }
    };

    Value::Ptr root;

    static Value::Ptr parse(std::string_view &json) {
        using namespace std::string_view_literals;
        if (json.empty()) {
            throw std::runtime_error("reflect::json_decode got unexpected end when parsing json");
        }
        Value::Ptr current;
        auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
        if (nonempty == json.npos) {
            return nullptr;
        }
        json.remove_prefix(nonempty);
        char c = json.front();
        if (c == '"') {
            json.remove_prefix(1);
            std::string str;
            unsigned int phase = 0;
            bool hisorr = false;
            unsigned int hex = 0;
            std::size_t i;
            for (i = 0; ; ++i) {
                if (i == json.size()) {
                    throw std::runtime_error("reflect::json_decode got non-terminated string");
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
                        hisorr = false;
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
                        hex |= c - '0';
                    } else if ('a' <= c && c <= 'f') {
                        hex |= c - 'a' + 10;
                    } else if ('A' <= c && c <= 'F') {
                        hex |= c - 'A' + 10;
                    }
                    if (phase == 5) {
                        if (0xD800 <= hex && hex < 0xDC00) {
                            if (!hisorr) {
                                phase = 2;
                                hisorr = true;
                                continue;
                            } else {
                                throw std::runtime_error("reflect::json_decode got invalid UTF-16");
                            }
                        } else if (0xDC00 <= hex && hex < 0xE000) {
                            if (hisorr) {
                                hex = 0x10000 + (hex - 0xD800) * 0x400 + (json[i] - 0xDC00);
                                hisorr = false;
                                phase = 0;
                            } else {
                                throw std::runtime_error("reflect::json_decode got invalid UTF-16");
                            }
                        }
                        if (hex <= 0x7F) {
                            str.push_back(hex);
                        } else if (hex <= 0x7FF) {
                            str.push_back(0xC0 | (hex >> 6));
                            str.push_back(0x80 | (hex & 0x3F));
                        } else if (hex <= 0xFFFF) {
                            str.push_back(0xE0 | (hex >> 12));
                            str.push_back(0x80 | ((hex >> 6) & 0x3F));
                            str.push_back(0x80 | (hex & 0x3F));
                        } else if (hex <= 0x10FFFF) {
                            str.push_back(0xF0 | (hex >> 18));
                            str.push_back(0x80 | ((hex >> 12) & 0x3F));
                            str.push_back(0x80 | ((hex >> 6) & 0x3F));
                            str.push_back(0x80 | (hex & 0x3F));
                        } else {
                            throw std::runtime_error("reflect::json_decode got invalid Unicode code point");
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
            current = Value::make<Value::String>(std::move(str));
        } else if (c == '{') {
            json.remove_prefix(1);
            std::map<std::string, Value::Ptr> dict;
            for (;;) {
                if (json.front() == '}') {
                    json.remove_prefix(1);
                    break;
                } else if (json.front() == ',') {
                    json.remove_prefix(1);
                    continue;
                } else {
                    auto key = parse(json);
                    if (!key) {
                        break;
                    }
                    std::string keyString;
                    if (auto p = std::get_if<Value::String>(&key->data)) {
                        keyString = std::move(*p);
                    } else {
                        throw std::runtime_error("reflect::json_decode got non-string key for dict");
                    }
                    debug(), keyString;
                    if (json.front() != ':') {
                        debug(), json;
                        throw std::runtime_error("reflect::json_decode expect ':' in dict");
                    }
                    json.remove_prefix(1);
                    debug(), json;
                    auto value = parse(json);
                    if (!value) {
                        throw std::runtime_error("reflect::json_decode got non-value for dict");
                    }
                    dict.emplace(std::move(keyString), std::move(value));
                    auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
                    if (nonempty == json.npos) {
                        throw std::runtime_error("reflect::json_decode got unexpected end when parsing dict");
                    }
                    json.remove_prefix(nonempty);
                }
            }
            current = Value::make<Value::Dict>(std::move(dict));
        } else if (c == '[') {
            json.remove_prefix(1);
            std::vector<Value::Ptr> array;
            current = Value::make<Value::Array>(std::move(array));
        } else if (('0' <= c && c <= '9') || c == '.' || c == '-' || c == '+') {
            debug(), json;
            auto end = json.find_first_of(",]}"sv);
            if (end == json.npos) end = json.size();
            auto str = std::string(json.data(), end);
            debug(), str;
            if (str.find('.') != str.npos) {
                double value = std::stod(str);
                current = Value::make<Value::Real>(value);
            } else {
                std::int64_t value = std::stoll(str);
                current = Value::make<Value::Integer>(value);
            }
            json.remove_prefix(end);
        } else {
            throw std::runtime_error("reflect::json_decode got unexpected token");
        }
        return current;
    }
};

struct ReflectorJsonEncode {
    JsonEncoder *encoder;
    bool comma = false;

    template <class T>
    void member(const char *name, T &value) {
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
    JsonDecoder::Value::Dict *currentDict;

    template <class T>
    void member(const char *name, T &value) {
        auto it = currentDict->find(name);
        if (it == currentDict->end()) {
            JsonDecoder::Value::Union nullData;
            JsonTrait<T>::getValue(nullData, value);
        } else {
            JsonTrait<T>::getValue(it->second->data, value);
        }
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
            encoder->putValue(*it);
        }
        encoder->put(']');
    }

    template <class T>
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::Array>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.emplace_back();
                JsonTrait<typename T::value_type>::getValue((*it)->data, element);
            }
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
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
            encoder->putString(it->first);
            encoder->put(':');
            encoder->putValue(it->second);
        }
        encoder->put('}');
    }

    template <class T>
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::Dict>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.try_emplace(it->first).first->second;
                JsonTrait<typename T::mapped_type>::getValue(it->second->data, element);
            }
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
        }
    }
};

struct JsonTraitStringLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putString(value.data(), value.size());
    }

    template <class T>
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::String>(&data)) {
            value = *p;
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
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
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (std::get_if<JsonDecoder::Value::Null>(&data)) {
            value = std::nullopt;
        } else {
            JsonTrait<typename T::value_type>::getValue(data, value.emplace());
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
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::Boolean>(&data)) {
            value = *p;
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
        }
    }
};

struct JsonTraitArithmeticLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putArithmetic(value);
    }

    template <class T>
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::Integer>(&data)) {
            value = *p;
        } else if (auto p = std::get_if<JsonDecoder::Value::Real>(&data)) {
            value = *p;
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
        }
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
    static void getValue(JsonDecoder::Value::Union &data, T &value) {
        if (auto p = std::get_if<JsonDecoder::Value::Dict>(&data)) {
            ReflectorJsonDecode reflector(p);
            reflect_members(reflector, value);
        } else {
            throw std::runtime_error("reflect::json_decode got type mismatch");
        }
    }
};

template <class T, std::size_t N>
struct JsonTrait<std::array<T, N>> : JsonTraitArrayLike {
};

template <class T, class Alloc>
struct JsonTrait<std::vector<T, Alloc>> : JsonTraitArrayLike {
};

template <class K, class V, class Cmp, class Alloc>
struct JsonTrait<std::map<K, V, Cmp, Alloc>> : JsonTraitDictLike {
};

template <class K, class V, class Hash, class Eq, class Alloc>
struct JsonTrait<std::unordered_map<K, V, Hash, Eq, Alloc>> : JsonTraitDictLike {
};

template <class Traits, class Alloc>
struct JsonTrait<std::basic_string<char, Traits, Alloc>> : JsonTraitStringLike {
};

template <class Traits>
struct JsonTrait<std::basic_string_view<char, Traits>> : JsonTraitStringLike {
};

template <class T>
struct JsonTrait<std::optional<T>> : JsonTraitOptionalLike {
};

template <>
struct JsonTrait<bool> : JsonTraitBooleanLike {
};

template <class T>
struct JsonTrait<T, std::enable_if_t<std::is_arithmetic_v<T>>> : JsonTraitArithmeticLike {
};

template <class Reflector, class T>
inline std::void_t<decltype(std::declval<T &>().REFLECT__MEMBERS(std::declval<Reflector &>()))> reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(reflector);
}

template <class Reflector, class T, class = std::void_t<decltype(REFLECT__MEMBERS(std::declval<T &>(), std::declval<Reflector &>()))>>
inline void reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(value, reflector);
}

template <class T>
struct JsonTrait<T, std::void_t<decltype(reflect_members(std::declval<ReflectorJsonEncode &>(), std::declval<T &>()))>> : JsonTraitObjectLike {
};

template <class T>
inline std::string json_encode(T const &value) {
    JsonEncoder encoder;
    encoder.putValue(value);
    return encoder.json;
}

template <class T>
inline T json_decode(std::string_view json) {
    JsonDecoder decoder;
    decoder.root = JsonDecoder::parse(json);
    T value;
    JsonTrait<T>::getValue(decoder.root->data, value);
    return value;
}

}

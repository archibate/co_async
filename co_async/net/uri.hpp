#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/utils/simple_map.hpp>

namespace co_async {
struct URIParams : SimpleMap<String, String> {
    using SimpleMap<String, String>::SimpleMap;
};

struct URI {
    String path;
    URIParams params;

public:
    static void url_decode(String &r, std::string_view s);
    static String url_decode(std::string_view s);
    static void url_encode(String &r, std::string_view s);
    static String url_encode(std::string_view s);
    static void url_encode_path(String &r, std::string_view s);
    static String url_encode_path(std::string_view s);
    static URI parse(std::string_view uri);
    void dump(String &r) const;
    String dump() const;

    String repr() const {
        return dump();
    }
};
} // namespace co_async

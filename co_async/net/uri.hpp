#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/simple_map.hpp>
namespace co_async {
struct URIParams : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};
struct URI {
    std::string path;
    URIParams   params;

private:
    static std::uint8_t fromHex(char c);
    static bool         isCharUrlSafe(char c);

public:
    static void        url_decode(std::string &r, std::string_view s);
    static std::string url_decode(std::string_view s);
    static void        url_encode(std::string &r, std::string_view s);
    static std::string url_encode(std::string_view s);
    static void        url_encode_path(std::string &r, std::string_view s);
    static std::string url_encode_path(std::string_view s);
    static URI         parse(std::string_view uri);
    void               dump(std::string &r) const;
    std::string        dump() const;
    std::string        repr() const {
        return dump();
    }
};
} // namespace co_async

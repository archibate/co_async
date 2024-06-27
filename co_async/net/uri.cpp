#include <co_async/net/uri.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
namespace {
std::uint8_t fromHex(char c) {
    if ('0' <= c && c <= '9') {
        return static_cast<std::uint8_t>(c - '0');
    } else if ('A' <= c && c <= 'F') {
        return static_cast<std::uint8_t>(c - 'A' + 10);
    } else [[unlikely]] {
        return 0;
    }
}

bool isCharUrlSafe(char c) {
    if ('0' <= c && c <= '9') {
        return true;
    }
    if ('a' <= c && c <= 'z') {
        return true;
    }
    if ('A' <= c && c <= 'Z') {
        return true;
    }
    if (c == '-' || c == '_' || c == '.') {
        return true;
    }
    return false;
}
} // namespace

void URI::url_decode(String &r, std::string_view s) {
    std::size_t b = 0;
    while (true) {
        auto i = s.find('%', b);
        if (i == std::string_view::npos || i + 3 > s.size()) {
            r.append(s.data() + b, s.data() + s.size());
            break;
        }
        r.append(s.data() + b, s.data() + i);
        char c1 = s[i + 1];
        char c2 = s[i + 2];
        r.push_back(static_cast<char>((fromHex(c1) << 4) | fromHex(c2)));
        b = i + 3;
    }
}

String URI::url_decode(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_decode(r, s);
    return r;
}

void URI::url_encode(String &r, std::string_view s) {
    static constexpr char lut[] = "0123456789ABCDEF";
    for (char c: s) {
        if (isCharUrlSafe(c)) {
            r.push_back(c);
        } else {
            r.push_back('%');
            r.push_back(lut[static_cast<std::uint8_t>(c) >> 4]);
            r.push_back(lut[static_cast<std::uint8_t>(c) & 0xF]);
        }
    }
}

String URI::url_encode(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_encode(r, s);
    return r;
}

void URI::url_encode_path(String &r, std::string_view s) {
    static constexpr char lut[] = "0123456789ABCDEF";
    for (char c: s) {
        if (isCharUrlSafe(c) || c == '/') {
            r.push_back(c);
        } else {
            r.push_back('%');
            r.push_back(lut[static_cast<std::uint8_t>(c) >> 4]);
            r.push_back(lut[static_cast<std::uint8_t>(c) & 0xF]);
        }
    }
}

String URI::url_encode_path(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_encode_path(r, s);
    return r;
}

URI URI::parse(std::string_view uri) {
    auto path = uri;
    URIParams params;
    if (auto i = uri.find('?'); i != std::string_view::npos) {
        path = uri.substr(0, i);
        do {
            uri.remove_prefix(i + 1);
            i = uri.find('&');
            auto pair = uri.substr(0, i);
            auto m = pair.find('=');
            if (m != std::string_view::npos) {
                auto k = pair.substr(0, m);
                auto v = pair.substr(m + 1);
                params.insert_or_assign(String(k), url_decode(v));
            }
        } while (i != std::string_view::npos);
    }
    String spath(path);
    if (spath.empty() || spath.front() != '/') [[unlikely]] {
        spath.insert(spath.begin(), '/');
    }
    return URI{spath, std::move(params)};
}

void URI::dump(String &r) const {
    r.append(path);
    char queryChar = '?';
    for (auto &[k, v]: params) {
        r.push_back(queryChar);
        url_encode(r, k);
        r.push_back('=');
        url_encode(r, v);
        queryChar = '&';
    }
}

String URI::dump() const {
    String r;
    dump(r);
    return r;
}
} // namespace co_async

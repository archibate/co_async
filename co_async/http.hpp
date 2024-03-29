#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <co_async/task.hpp>
#include <co_async/simple_map.hpp>
#include <co_async/http_status_code.hpp>
#include <co_async/string_utils.hpp>
#include <co_async/uri.hpp>

namespace co_async {

struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};

struct HTTPRequest {
    std::string method;
    URI uri;
    HTTPHeaders headers;
    std::string body;

    Task<> write_into(auto &sock) const {
        using namespace std::string_view_literals;
        co_await sock.puts(method);
        co_await sock.putchar(' ');
        co_await sock.puts(uri.dump());
        co_await sock.puts(" HTTP/1.1\r\n"sv);
        for (auto const &[k, v]: headers) {
            co_await sock.puts(k);
            co_await sock.puts(": "sv);
            co_await sock.puts(v);
            co_await sock.puts("\r\n"sv);
        }
        if (body.empty()) {
            co_await sock.puts("\r\n"sv);
        } else {
            co_await sock.puts("content-length: "sv);
            co_await sock.puts(to_string(body.size()));
            co_await sock.puts("\r\n\r\n"sv);
            co_await sock.puts(body);
        }
        co_await sock.flush();
    }

    Task<> read_from(auto &sock) {
        using namespace std::string_view_literals;
        auto line = co_await sock.getline("\r\n"sv);
        auto pos = line.find(' ');
        if (pos == line.npos || pos == line.size() - 1)
            [[unlikely]] {
            throw std::invalid_argument("invalid http request");
        }
        method = line.substr(0, pos);
        auto pos2 = line.find(' ', pos + 1);
        if (pos2 == line.npos || pos2 == line.size() - 1)
            [[unlikely]] {
            throw std::invalid_argument("invalid http request");
        }
        uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));
        while (true) {
            auto line = co_await sock.getline("\r\n"sv);
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 || line[pos + 1] != ' ')
                [[unlikely]] {
                throw std::invalid_argument("invalid http request");
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }
        if (auto p = headers.get("content-length"sv, from_string<std::size_t>)) {
            body = co_await sock.getn(*p);
        }
    }

    auto repr() const {
        return std::make_tuple(method, uri, headers, body);
    }
};

struct HTTPResponse {
    int status;
    HTTPHeaders headers;
    std::string body;

    Task<> write_into(auto &sock) const {
        using namespace std::string_view_literals;
        co_await sock.puts("HTTP/1.1 "sv);
        co_await sock.puts(to_string(status));
        co_await sock.putchar(' ');
        co_await sock.puts(getHTTPStatusName(status));
        co_await sock.puts("\r\n"sv);
        for (auto const &[k, v]: headers) {
            co_await sock.puts(k);
            co_await sock.puts(": "sv);
            co_await sock.puts(v);
            co_await sock.puts("\r\n"sv);
        }
        if (body.empty()) {
            co_await sock.puts("\r\n"sv);
        } else {
            co_await sock.puts("content-length: "sv);
            co_await sock.puts(to_string(body.size()));
            co_await sock.puts("\r\n\r\n"sv);
            co_await sock.puts(body);
        }
        co_await sock.flush();
    }

    Task<> read_from(auto &sock) {
        using namespace std::string_view_literals;
        auto line = co_await sock.getline("\r\n"sv);
        if (line.size() <= 9 || line.substr(0, 9) != "HTTP/1.1 "sv)
            [[unlikely]] {
            throw std::invalid_argument("invalid http response");
        }
        status = from_string<int>(line.substr(9));
        while (true) {
            auto line = co_await sock.getline("\r\n"sv);
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 || line[pos + 1] != ' ')
                [[unlikely]] {
                throw std::invalid_argument("invalid http response");
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }
        if (auto p = headers.get("content-length"sv, from_string<std::size_t>)) {
            body = co_await sock.getn(*p);
        }
    }

    auto repr() const {
        return std::make_tuple(status, headers, body);
    }
};

}

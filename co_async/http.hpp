#pragma once

#include <string>
#include <exception>
#include <string>
#include <string_view>
#include <tuple>
#include <co_async/task.hpp>
#include <co_async/simple_map.hpp>

namespace co_async {

struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};

struct HTTPRequest {
    std::string method;
    std::string uri;
    HTTPHeaders headers;
    std::string body;

    Task<> write_into(auto &sock) {
        using namespace std::string_view_literals;
        co_await sock.puts(method);
        co_await sock.putchar(' ');
        co_await sock.puts(uri);
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
            co_await sock.puts(std::to_string(body.size()));
            co_await sock.puts("\r\n"sv);
            co_await sock.puts(body);
        }
        co_await sock.flush();
    }

    auto repr() const {
        return std::make_tuple(method, uri, headers, body);
    }
};

struct HTTPResponse {
    int status;
    HTTPHeaders headers;
    std::string body;

    Task<> read_from(auto &sock) {
        using namespace std::string_view_literals;
        auto line = co_await sock.getline("\r\n"sv);
        if (line.size() <= 9 || line.substr(0, 9) != "HTTP/1.1 "sv)
            [[unlikely]] {
            throw std::invalid_argument("invalid http response");
        }
        status = std::stoi(line.substr(9));
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
        if (auto p = headers.at("content-length"sv)) [[likely]] {
            auto len = std::stoi(*p);
            body = co_await sock.getn(len);
        }
    }

    auto repr() const {
        return std::make_tuple(status, headers, body);
    }
};

}

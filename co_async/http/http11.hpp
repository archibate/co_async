#pragma once/*{export module co_async:http.http11;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/utils/simple_map.hpp>/*{import :utils.simple_map;}*/
#include <co_async/http/http_status_code.hpp>/*{import :http.http_status_code;}*/
#include <co_async/utils/string_utils.hpp>/*{import :utils.string_utils;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/system/pipe.hpp>/*{import :system.pipe;}*/
#include <co_async/iostream/socket_stream.hpp>/*{import :iostream.socket_stream;}*/
#include <co_async/http/uri.hpp>/*{import :http.uri;}*/

namespace co_async {

/*[export]*/ struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};

struct HTTPPolymorphicBody {
    HTTPPolymorphicBody() = default;
    HTTPPolymorphicBody(std::string content) : mBody(std::move(content)) {}
    HTTPPolymorphicBody(std::string_view content) : mBody(std::string(content)) {}
    HTTPPolymorphicBody(const char *content) : mBody(content) {}
    HTTPPolymorphicBody(DirFilePath path) : mBody(std::move(path)) {}
    HTTPPolymorphicBody(std::filesystem::path path) : mBody(DirFilePath(path)) {}

    bool isNone() const noexcept {
        return std::holds_alternative<std::monostate>(mBody);
    }

    bool isString() const noexcept {
        return std::holds_alternative<std::string>(mBody);
    }

    bool isFile() const noexcept {
        return std::holds_alternative<DirFilePath>(mBody);
    }

    std::string_view getString() const {
        return std::get<std::string>(mBody);
    }

    DirFilePath getFile() const {
        return std::get<DirFilePath>(mBody);
    }

    Task<> write_into(SocketStream &sock) const {
        using namespace std::string_view_literals;
        if (std::get_if<std::monostate>(&mBody)) {
            co_await sock.puts("\r\n"sv);
        } else if (auto bodyStr = std::get_if<std::string>(&mBody)) {
            co_await sock.puts("content-length: "sv);
            co_await sock.puts(to_string(bodyStr->size()));
            co_await sock.puts("\r\n\r\n"sv);
            co_await sock.puts(*bodyStr);
        } else if (auto bodyPath = std::get_if<DirFilePath>(&mBody)) {
            auto size = co_await fs_stat_size(*bodyPath);
            co_await sock.puts("content-length: "sv);
            co_await sock.puts(to_string(size));
            co_await sock.puts("\r\n\r\n"sv);
            co_await sock.flush();
            std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(size);
            std::span<char> bufSpan(buffer.get(), size);
            auto file = co_await fs_open(*bodyPath, OpenMode::Read);
            auto [readPipe, writePipe] = co_await make_pipe();
            while (size > 0) {
                auto n = co_await fs_splice(file, writePipe, size);
                co_await fs_splice(readPipe, sock.get(), n);
                size -= n;
            }
            co_await fs_close(std::move(file));
            co_await fs_close(std::move(readPipe));
            co_await fs_close(std::move(writePipe));
        }
    }

    auto repr() const {
        return mBody;
    }

private:
    std::variant<std::monostate, std::string, DirFilePath> mBody;
};

/*[export]*/ struct HTTPRequest {
    std::string method;
    URI uri;
    HTTPHeaders headers;
    HTTPPolymorphicBody body;
    bool keepAlive = true;

    Task<> write_into(SocketStream &sock) const {
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
        if (keepAlive) {
            co_await sock.puts("connection: keep-alive\r\n"sv);
        } else {
            co_await sock.puts("connection: close\r\n"sv);
        }
        co_await body.write_into(sock);
        co_await sock.flush();
    }

    Task<bool> read_from(SocketStream &sock) {
        using namespace std::string_view_literals;
        auto line = co_await sock.getline("\r\n"sv);
        if (line.empty()) co_return false;
        auto pos = line.find(' ');
        if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP request:\n\t[" + line + "]\n";
#endif
            throw std::invalid_argument("invalid http request: version");
        }
        method = line.substr(0, pos);
        auto pos2 = line.find(' ', pos + 1);
        if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP request:\n\t[" + line + "]\n";
#endif
            throw std::invalid_argument("invalid http request: method");
        }
        uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));
        while (true) {
            auto line = co_await sock.getline("\r\n"sv);
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 ||
                line[pos + 1] != ' ') [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: invalid HTTP request:\n\t[" + line + "]\n";
#endif
                throw std::invalid_argument("invalid http request: header");
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }
        if (auto p =
                headers.get("content-length"sv, from_string<std::size_t>)) {
            body = co_await sock.getn(*p);
        }

        if (auto connection = headers.get("connection"sv)) {
            keepAlive = lower_string(*connection) != "close";
        }
        co_return true;
    }

    auto repr() const {
        return std::make_tuple(method, uri, headers, body);
    }
};

/*[export]*/ struct HTTPResponse {
    int status;
    HTTPHeaders headers;
    HTTPPolymorphicBody body;
    bool keepAlive = true;

    Task<> write_into(SocketStream &sock) const {
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
        if (keepAlive) {
            co_await sock.puts("connection: keep-alive\r\n"sv);
        } else {
            co_await sock.puts("connection: close\r\n"sv);
        }
        co_await body.write_into(sock);
        co_await sock.flush();
    }

    Task<bool> read_from(SocketStream &sock) {
        using namespace std::string_view_literals;
        auto line = co_await sock.getline("\r\n"sv);
        if (line.empty()) co_return false;
        if (line.size() <= 9 || line.substr(0, 9) != "HTTP/1.1 "sv)
            [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP response:\n\t[" + line + "]\n";
#endif
            throw std::invalid_argument("invalid http response: version");
        }
        if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
            status = *statusOpt;
        } else [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP response:\n\t[" + line + "]\n";
#endif
            throw std::invalid_argument("invalid http response: status");
        }
        while (true) {
            auto line = co_await sock.getline("\r\n"sv);
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 ||
                line[pos + 1] != ' ') [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: invalid HTTP response:\n\t[" + line + "]\n";
#endif
                throw std::invalid_argument("invalid http response: header");
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }
        if (auto p =
                headers.get("content-length"sv, from_string<std::size_t>)) {
            body = co_await sock.getn(*p);
        }

        if (auto connection = headers.get("connection"sv)) {
            keepAlive = lower_string(*connection) != "close";
        }
        co_return true;
    }

    auto repr() const {
        return std::make_tuple(status, headers, body);
    }
};

} // namespace co_async

#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/iostream/string_stream.hpp>
#include <co_async/iostream/zlib_stream.hpp>
#include <co_async/http/http_status_code.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/http/uri.hpp>

namespace co_async {

struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};

struct HTTPTransferEncoding {
    enum Type {
        Identity = 0,
        Chunked,
        Gzip,
        Compress,
        Deflate,
        Br,
        Zstd,
    };

private:
    Type mType;
    std::size_t mContentLength;

public:
    HTTPTransferEncoding(Type type = Identity, std::size_t len = 0) noexcept
        : mType(type),
          mContentLength(len) {}

    operator Type() const noexcept {
        return mType;
    }

    auto &contentLength() noexcept {
        return mContentLength;
    }

    auto contentLength() const noexcept {
        return mContentLength;
    }

    auto repr() const {
        return std::make_tuple(mType, mContentLength);
    }
};

struct HTTPRequest {
    std::string method;
    URI uri;
    HTTPHeaders headers;

    auto repr() const {
        return std::make_tuple(method, uri, headers);
    }
};

struct HTTPResponse {
    int status;
    HTTPHeaders headers;

    auto repr() const {
        return std::make_tuple(status, headers);
    }
};

struct HTTPProtocol {
    std::unique_ptr<IOStream> sock;

    explicit HTTPProtocol(std::unique_ptr<IOStream> sock)
        : sock(std::move(sock)) {}

    virtual ~HTTPProtocol() = default;

    virtual Task<> writeBodyStream(IStream &body) = 0;
    virtual Task<bool> readBodyStream(OStream &body) = 0;
    virtual Task<> writeBody(std::string_view body) = 0;
    virtual Task<bool> readBody(std::string &body) = 0;
    virtual Task<> writeRequest(HTTPRequest const &req) = 0;
    virtual Task<bool> readRequest(HTTPRequest &req) = 0;
    virtual Task<> writeResponse(HTTPResponse const &res) = 0;
    virtual Task<bool> readResponse(HTTPResponse &res) = 0;
};

struct HTTPProtocolVersion11 : HTTPProtocol {
    using HTTPProtocol::HTTPProtocol;

    HTTPTransferEncoding encoding;

#if CO_ASYNC_DEBUG
private:
    int mPhase = 0;

    void checkPhase(int from, int to) {
        if (mPhase != from) [[unlikely]] {
            throw std::logic_error(
                "HTTPProtocol member function calling order wrong (phase = " +
                to_string(mPhase) + ", from = " + to_string(from) +
                ", to = " + to_string(to) + ")");
        }
        mPhase = to;
    }

public:
#endif

    Task<> writeBodyStream(IStream &body) override {
#if CO_ASYNC_DEBUG
        checkPhase(1, 0);
#endif
        using namespace std::string_view_literals;
        switch (encoding) {
        case HTTPTransferEncoding::Chunked: {
            bool hadHeader = false;
            do {
                auto bufSpan = body.rdbuf();
                auto n = bufSpan.size();
                if (n > 0) {
                    char buf[sizeof(n) * 2 + 4] = {}, *ep = buf;
                    *ep++ = '\r';
                    *ep++ = '\n';
                    do {
                        *ep++ = "0123456789ABCDEF"[n & 15];
                    } while (n >>= 4);
                    std::reverse(buf + 2, ep);
                    *ep++ = '\r';
                    *ep++ = '\n';
                    if (!hadHeader) {
                        co_await sock->puts("transfer-encoding: chunked\r\n"sv);
                        hadHeader = true;
                    }
                    co_await sock->puts(std::string_view{
                        buf, static_cast<std::size_t>(ep - buf)});
                    co_await sock->putspan(bufSpan);
                }
            } while (co_await body.fillbuf());
            if (!hadHeader) {
                co_await sock->puts("\r\n"sv);
            } else {
                co_await sock->puts("\r\n0\r\n\r\n"sv);
            }
        } break;
        case HTTPTransferEncoding::Gzip:
            co_await sock->puts("content-encoding: gzip\r\n\r\n"sv);
            throw std::runtime_error("compress encoding not supported yet");
            break;
        case HTTPTransferEncoding::Compress:
            co_await sock->puts("content-encoding: compress\r\n\r\n"sv);
            throw std::runtime_error("compress encoding not supported yet");
            break;
        case HTTPTransferEncoding::Deflate:
            co_await sock->puts("content-encoding: deflate\r\n\r\n"sv);
            throw std::runtime_error("compress encoding not supported yet");
            break;
        case HTTPTransferEncoding::Br:
            co_await sock->puts("content-encoding: br\r\n\r\n"sv);
            throw std::runtime_error("compress encoding not supported yet");
            break;
        case HTTPTransferEncoding::Zstd:
            co_await sock->puts("content-encoding: zstd\r\n\r\n"sv);
            throw std::runtime_error("compress encoding not supported yet");
            break;
        case HTTPTransferEncoding::Identity: {
            auto content = co_await body.getall();
            co_await sock->puts("content-length: "sv);
            co_await sock->puts(to_string(content.size()));
            co_await sock->puts("\r\n\r\n"sv);
            co_await sock->puts(content);
        } break;
        }
        co_await sock->flush();
    }

    Task<> writeBody(std::string_view body) override {
        using namespace std::string_view_literals;
        if (body.empty()) {
#if CO_ASYNC_DEBUG
            checkPhase(1, 0);
#endif
            using namespace std::string_view_literals;
            co_await sock->puts("\r\n"sv);
        } else {
            switch (encoding) {
            case HTTPTransferEncoding::Identity: {
#if CO_ASYNC_DEBUG
                checkPhase(1, 0);
#endif
                co_await sock->puts("content-length: "sv);
                co_await sock->puts(to_string(body.size()));
                co_await sock->puts("\r\n\r\n"sv);
                co_await sock->puts(body);
            } break;
            case HTTPTransferEncoding::Chunked: {
#if CO_ASYNC_DEBUG
                checkPhase(1, 0);
#endif
                co_await sock->puts("transfer-encoding: chunked\r\n"sv);
                auto n = body.size();
                char buf[sizeof(n) * 2 + 4] = {}, *ep = buf;
                *ep++ = '\r';
                *ep++ = '\n';
                do {
                    *ep++ = "0123456789ABCDEF"[n & 15];
                } while (n >>= 4);
                std::reverse(buf + 2, ep);
                *ep++ = '\r';
                *ep++ = '\n';
                co_await sock->puts(
                    std::string_view{buf, static_cast<std::size_t>(ep - buf)});
                co_await sock->puts(body);
                co_await sock->puts("\r\n0\r\n\r\n"sv);
            } break;
            default: {
                StringIStream is(body);
                co_await writeBodyStream(is);
            } break;
            }
        }
        co_await sock->flush();
    }

    Task<bool> readBodyStream(OStream &body) override {
#if CO_ASYNC_DEBUG
        checkPhase(-1, 0);
#endif
        using namespace std::string_view_literals;
        switch (encoding) {
        case HTTPTransferEncoding::Identity: {
            if (auto n = encoding.contentLength(); n > 0) {
                std::string line;
                co_await sock->getn(line, n);
                co_await body.puts(line);
            }
        } break;
        case HTTPTransferEncoding::Chunked: {
            std::string line;
            while (true) {
                line.clear();
                if (!co_await sock->getline(line, "\r\n"sv)) [[unlikely]] {
                    co_return false;
                }
                std::size_t n = std::strtoull(line.c_str(), nullptr, 16);
                if (n <= 0) {
                    if (!co_await sock->dropn(2)) [[unlikely]] {
                        co_return false;
                    }
                    break;
                }
                line.clear();
                co_await sock->getn(line, n);
                co_await body.puts(line);
                if (!co_await sock->dropn(2)) [[unlikely]] {
                    co_return false;
                }
            }
        } break;
        case HTTPTransferEncoding::Gzip: [[fallthrough]];
        case HTTPTransferEncoding::Compress: [[fallthrough]];
        case HTTPTransferEncoding::Deflate: [[fallthrough]];
        case HTTPTransferEncoding::Br: [[fallthrough]];
        case HTTPTransferEncoding::Zstd: {
            throw std::runtime_error("compress encoding not supported yet");
            /* std::string content = co_await sock->getall(); */
            /* co_await body.puts(content); */
        } break;
        }
        co_await body.flush();
        co_return true;
    }

    Task<bool> readBody(std::string &body) override {
        using namespace std::string_view_literals;
        switch (encoding) {
        case HTTPTransferEncoding::Identity: {
#if CO_ASYNC_DEBUG
            checkPhase(-1, 0);
#endif
            if (auto n = encoding.contentLength(); n > 0) {
                co_await sock->getn(body, n);
            }
        } break;
        case HTTPTransferEncoding::Chunked: {
#if CO_ASYNC_DEBUG
            checkPhase(-1, 0);
#endif
            std::string line;
            while (true) {
                line.clear();
                if (!co_await sock->getline(line, "\r\n"sv)) [[unlikely]] {
                    co_return false;
                }
                auto n = std::strtoul(line.c_str(), nullptr, 16);
                if (n == 0) {
                    break;
                }
                co_await sock->getn(body, n);
                if (!co_await sock->dropn(2)) [[unlikely]] {
                    co_return false;
                }
            }
        } break;
        default: {
            StringOStream os;
            co_await readBodyStream(os);
            co_return false;
        } break;
        };
        co_return true;
    }

    Task<> writeRequest(HTTPRequest const &req) override {
#if CO_ASYNC_DEBUG
        checkPhase(0, 1);
#endif
        using namespace std::string_view_literals;
        co_await sock->puts(req.method);
        co_await sock->putchar(' ');
        co_await sock->puts(req.uri.dump());
        co_await sock->puts(" HTTP/1.1\r\n"sv);
        for (auto const &[k, v]: req.headers) {
            co_await sock->puts(k);
            co_await sock->puts(": "sv);
            co_await sock->puts(v);
            co_await sock->puts("\r\n"sv);
        }
        co_await sock->puts("connection: keep-alive\r\n"sv);
        encoding = HTTPTransferEncoding::Chunked;
    }

    Task<bool> readRequest(HTTPRequest &req) override {
#if CO_ASYNC_DEBUG
        checkPhase(0, -1);
#endif
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        std::string line;
        if (!co_await sock->getline(line, "\r\n"sv))
            co_return false;
        auto pos = line.find(' ');
        if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP request:\n\t[" + line + "]\n";
#endif
            /* throw std::invalid_argument("invalid http request: version"); */
            co_return false;
        }
        req.method = line.substr(0, pos);
        auto pos2 = line.find(' ', pos + 1);
        if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP request:\n\t[" + line + "]\n";
#endif
            /* throw std::invalid_argument("invalid http request: method"); */
            co_return false;
        }
        req.uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));
        while (true) {
            line.clear();
            if (!co_await sock->getline(line, "\r\n"sv)) [[unlikely]] {
                co_return false;
            }
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 ||
                line[pos + 1] != ' ') [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: invalid HTTP request:\n\t[" + line +
                                 "]\n";
#endif
                /* throw std::invalid_argument("invalid http request: header"); */
                co_return false;
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            req.headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }

        if (auto transEnc = req.headers.get("transfer-encoding"sv)) {
            encoding = encodingByName(*transEnc);
            req.headers.erase("transfer-encoding"sv);
        } else if (auto contEnc = req.headers.get("content-encoding"sv)) {
            encoding = encodingByName(*contEnc);
            req.headers.erase("content-encoding"sv);
        }
        if (encoding == HTTPTransferEncoding::Identity) {
            std::size_t len =
                req.headers.get("content-length"sv, from_string<std::size_t>)
                    .value_or(0);
            req.headers.erase("content-length"sv);
            encoding.contentLength() = len;
        }
        /* if (auto acceptEnc = req.headers.get("accept-encoding"sv)) { */
        /*     for (std::string_view encName: split_string(*acceptEnc, ", "sv)) { */
        /*         if (auto i = encName.find(';'); i != encName.npos) { */
        /*             encName = encName.substr(0, i); */
        /*         } */
        /*         auto enc = encodingByName(encName); */
        /*         if (enc != HTTPTransferEncoding::Identity) [[likely]] { */
        /*             encoding = enc; */
        /*             break; */
        /*         } */
        /*     } */
        /*     req.headers.erase("accept-encoding"sv); */
        /* } */
        req.headers.erase("accept-encoding"sv);
        req.headers.erase("connection"sv);
        co_return true;
    }

    Task<> writeResponse(HTTPResponse const &res) override {
#if CO_ASYNC_DEBUG
        checkPhase(0, 1);
#endif
        using namespace std::string_view_literals;
        co_await sock->puts("HTTP/1.1 "sv);
        co_await sock->puts(to_string(res.status));
        co_await sock->putchar(' ');
        co_await sock->puts(getHTTPStatusName(res.status));
        co_await sock->puts("\r\n"sv);
        for (auto const &[k, v]: res.headers) {
            co_await sock->puts(k);
            co_await sock->puts(": "sv);
            co_await sock->puts(v);
            co_await sock->puts("\r\n"sv);
        }
        co_await sock->puts("connection: keep-alive\r\n"sv);
        encoding = HTTPTransferEncoding::Chunked;
    }

    Task<bool> readResponse(HTTPResponse &res) override {
#if CO_ASYNC_DEBUG
        checkPhase(0, -1);
#endif
        using namespace std::string_view_literals;
        auto line = co_await sock->getline("\r\n"sv);
        if (line.empty())
            co_return false;
        if (line.size() <= 9 || line.substr(0, 7) != "HTTP/1."sv ||
            line[8] != ' ') [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP response:\n\t[" + line + "]\n";
#endif
            /* throw std::invalid_argument("invalid http response: version"); */
            co_return false;
        }
        if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
            res.status = *statusOpt;
        } else [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: invalid HTTP response:\n\t[" + line + "]\n";
#endif
            /* throw std::invalid_argument("invalid http response: status"); */
            co_return false;
        }
        while (true) {
            line.clear();
            if (!co_await sock->getline(line, "\r\n"sv)) [[unlikely]] {
                co_return false;
            }
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 ||
                line[pos + 1] != ' ') [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: invalid HTTP response:\n\t[" + line +
                                 "]\n";
#endif
                /* throw std::invalid_argument("invalid http response: header"); */
                co_return false;
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            res.headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }

        encoding = HTTPTransferEncoding::Identity;
        if (auto transEnc = res.headers.get("transfer-encoding"sv)) {
            encoding = encodingByName(*transEnc);
            res.headers.erase("transfer-encoding"sv);
        } else if (auto contEnc = res.headers.get("content-encoding"sv)) {
            encoding = encodingByName(*contEnc);
            res.headers.erase("content-encoding"sv);
        }
        if (encoding == HTTPTransferEncoding::Identity) {
            std::size_t len =
                res.headers.get("content-length"sv, from_string<std::size_t>)
                    .value_or(0);
            res.headers.erase("content-length"sv);
            encoding.contentLength() = len;
        }
        res.headers.erase("connection");
        co_return true;
    }

    static HTTPTransferEncoding encodingByName(std::string_view name) {
        using namespace std::string_view_literals;
        static constexpr std::pair<std::string_view, HTTPTransferEncoding::Type>
            encodings[] = {
                {"identity"sv, HTTPTransferEncoding::Identity},
                {"chunked"sv, HTTPTransferEncoding::Chunked},
                {"gzip"sv, HTTPTransferEncoding::Gzip},
                {"compress"sv, HTTPTransferEncoding::Compress},
                {"deflate"sv, HTTPTransferEncoding::Deflate},
                {"br"sv, HTTPTransferEncoding::Br},
                {"zstd"sv, HTTPTransferEncoding::Zstd},
            };
        for (auto const &[k, v]: encodings) {
            if (name == k) {
                return v;
            }
        }
        return HTTPTransferEncoding::Identity;
    }
};

} // namespace co_async

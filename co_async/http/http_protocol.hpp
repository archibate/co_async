#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/iostream/string_stream.hpp>
#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/iostream/zlib_stream.hpp>
#include <co_async/http/http_status_code.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/system/process.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/http/uri.hpp>

namespace co_async {

struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
    // NOTE: user-specified http headers must not contain the following keys:
    // - connection
    // - acecpt-encoding
    // - transfer-encoding
    // - content-encoding
    // - content-length
    // they should only be used internally in our http protocol implementation
};

enum class HTTPContentEncoding {
    Identity = 0,
    Gzip,
    Deflate,
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
public:
    OwningStream sock;

protected:
    HTTPContentEncoding mContentEncoding;
    std::optional<std::size_t> mContentLength;

public:
    explicit HTTPProtocol(OwningStream sock) : sock(std::move(sock)) {}

    virtual ~HTTPProtocol() = default;

    virtual void initServerState() = 0;
    virtual void initClientState() = 0;
    virtual Task<Expected<void, std::errc>>
    writeBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<void, std::errc>>
    readBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<void, std::errc>>
    writeBody(std::string_view body) = 0;
    virtual Task<Expected<void, std::errc>> readBody(std::string &body) = 0;
    virtual Task<Expected<void, std::errc>>
    writeRequest(HTTPRequest const &req) = 0;
    virtual Task<Expected<void, std::errc>> readRequest(HTTPRequest &req) = 0;
    virtual Task<Expected<void, std::errc>>
    writeResponse(HTTPResponse const &res) = 0;
    virtual Task<Expected<void, std::errc>> readResponse(HTTPResponse &res) = 0;
};

struct HTTPProtocolVersion11 : HTTPProtocol {
    using HTTPProtocol::HTTPProtocol;

protected:
    HTTPContentEncoding httpContentEncodingByName(std::string_view name) {
        using namespace std::string_view_literals;
        static constexpr std::pair<std::string_view, HTTPContentEncoding>
            encodings[] = {
                {"gzip"sv, HTTPContentEncoding::Gzip},
                {"deflate"sv, HTTPContentEncoding::Deflate},
                {"identity"sv, HTTPContentEncoding::Identity},
            };
        for (auto const &[k, v]: encodings) {
            if (name == k) {
                return v;
            }
        }
        return HTTPContentEncoding::Identity;
    }

    Task<Expected<void, std::errc>> parseHeaders(HTTPHeaders &headers) {
        using namespace std::string_view_literals;
        std::string line;
        while (true) {
            line.clear();
            co_await co_await sock.getline(line, "\r\n"sv);
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == line.npos || pos == line.size() - 1 ||
                line[pos + 1] != ' ') [[unlikely]] {
                co_return Unexpected{std::errc::protocol_error};
            }
            auto key = line.substr(0, pos);
            for (auto &c: key) {
                if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            headers.insert_or_assign(std::move(key), line.substr(pos + 2));
        }
        headers.erase("connection"sv);
        co_return {};
    }

    Task<Expected<void, std::errc>> dumpHeaders(HTTPHeaders const &headers) {
        using namespace std::string_view_literals;
        for (auto const &[k, v]: headers) {
            co_await co_await sock.puts(k);
            co_await co_await sock.puts(": "sv);
            co_await co_await sock.puts(v);
            co_await co_await sock.puts("\r\n"sv);
        }
        co_await co_await sock.puts("connection: keep-alive\r\n"sv);
        co_return {};
    }

    void handleContentEncoding(HTTPHeaders &headers) {
        using namespace std::string_view_literals;
        mContentEncoding = HTTPContentEncoding::Identity;
        mContentLength = std::nullopt;
        bool needLength = true;
        if (auto transEnc = headers.get("transfer-encoding"sv)) {
            if (*transEnc == "chunked") [[likely]] {
                needLength = false;
            }
            headers.erase("transfer-encoding"sv);
        }
        if (auto contEnc = headers.get("content-encoding"sv)) {
            mContentEncoding = httpContentEncodingByName(*contEnc);
            headers.erase("content-encoding"sv);
        }
        if (needLength) {
            mContentLength =
                headers.get("content-length"sv, from_string<std::size_t>)
                    .value_or(0);
            headers.erase("content-length"sv);
        }
    }

    void handleAcceptEncoding(HTTPHeaders &headers) {
        using namespace std::string_view_literals;
        mContentEncoding = HTTPContentEncoding::Identity;
#if CO_ASYNC_ZLIB
        if (auto acceptEnc = headers.get("accept-encoding"sv)) {
            for (std::string_view encName: split_string(*acceptEnc, ", "sv)) {
                if (auto i = encName.find(';'); i != encName.npos) {
                    encName = encName.substr(0, i);
                }
                auto enc = httpContentEncodingByName(encName);
                if (enc != HTTPContentEncoding::Identity) [[likely]] {
                    mContentEncoding = enc;
                    break;
                }
            }
            headers.erase("accept-encoding"sv);
        }
#else
        headers.erase("accept-encoding"sv);
#endif
    }

    Task<Expected<void, std::errc>> writeChunked(BorrowedStream &body) {
        using namespace std::string_view_literals;
        bool hadHeader = false;
        do {
            auto bufSpan = body.peekbuf();
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
                    co_await co_await sock.puts(
                        "transfer-encoding: chunked\r\n"sv);
                    hadHeader = true;
                }
                co_await co_await sock.puts(
                    std::string_view{buf, static_cast<std::size_t>(ep - buf)});
                co_await co_await sock.putspan(bufSpan);
            }
        } while (co_await body.fillbuf());
        if (!hadHeader) {
            co_await co_await sock.puts("\r\n"sv);
        } else {
            co_await co_await sock.puts("\r\n0\r\n\r\n"sv);
        }
        co_return {};
    }

    Task<Expected<void, std::errc>> writeChunkedString(std::string_view body) {
        using namespace std::string_view_literals;
        if (body.empty()) {
            co_await co_await sock.puts("\r\n"sv);
        } else {
            co_await co_await sock.puts("content-length: "sv);
            co_await co_await sock.puts(to_string(body.size()));
            co_await co_await sock.puts("\r\n\r\n"sv);
            co_await co_await sock.puts(body);
        }
        co_return {};
    }

    Task<Expected<void, std::errc>> readChunked(BorrowedStream &body) {
        using namespace std::string_view_literals;
        if (mContentLength) {
            if (auto n = *mContentLength; n > 0) {
                std::string line;
                co_await co_await sock.getn(line, n);
                co_await co_await body.puts(line);
            }
        } else {
            std::string line;
            while (true) {
                line.clear();
                co_await co_await sock.getline(line, "\r\n"sv);
                std::size_t n = std::strtoull(line.c_str(), nullptr, 16);
                if (n <= 0) {
                    co_await co_await sock.dropn(2);
                    break;
                }
                line.clear();
                co_await co_await sock.getn(line, n);
                co_await co_await body.puts(line);
                co_await co_await sock.dropn(2);
            }
        }
        co_return {};
    }

    Task<Expected<void, std::errc>> readChunkedString(std::string &body) {
        using namespace std::string_view_literals;
        if (mContentLength) {
            if (auto n = *mContentLength; n > 0) {
                co_await co_await sock.getn(body, n);
            }
        } else {
            std::string line;
            while (true) {
                line.clear();
                co_await co_await sock.getline(line, "\r\n"sv);
                auto n = std::strtoul(line.c_str(), nullptr, 16);
                if (n == 0) {
                    break;
                }
                co_await co_await sock.getn(body, n);
                co_await co_await sock.dropn(2);
            }
        }
        co_return {};
    }

    Task<Expected<void, std::errc>> writeEncoded(BorrowedStream &body) {
        using namespace std::string_view_literals;
        switch (mContentEncoding) {
        case HTTPContentEncoding::Identity: {
            co_await co_await writeChunked(body);
        } break;
        case HTTPContentEncoding::Deflate: {
            co_await co_await sock.puts("content-encoding: deflate\r\n"sv);
            auto [r, w] = co_await co_await pipe_stream();
            FutureGroup group;
            group.add([&body, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await zlib_deflate(body, w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            });
            group.add(writeChunked(r));
            co_await co_await group.wait();
        } break;
        case HTTPContentEncoding::Gzip: {
            co_await co_await sock.puts("content-encoding: gzip\r\n"sv);
            OwningStream pin, pout;
            auto pid = co_await co_await ProcessBuilder()
                           .path("gzip"sv)
                           .arg("-"sv)
                           .pipe_in(0, pin)
                           .pipe_out(1, pout)
                           .spawn();
            FutureGroup group;
            group.add(pipe_forward(body, pin));
            group.add(writeChunked(pout));
            co_await co_await group.wait();
            co_await co_await wait_process(pid);
        } break;
        };
        co_return {};
    }

    Task<Expected<void, std::errc>> writeEncoded(std::string_view body) {
        using namespace std::string_view_literals;
        switch (mContentEncoding) {
        case HTTPContentEncoding::Identity: {
            co_await co_await writeChunkedString(body);
        } break;
        default: {
            auto is = make_stream<IStringStream>(body);
            co_await co_await writeEncoded(is);
        } break;
        };
        co_return {};
    }

    Task<Expected<void, std::errc>> readEncoded(BorrowedStream &body) {
        using namespace std::string_view_literals;
        switch (mContentEncoding) {
        case HTTPContentEncoding::Identity: {
            co_await co_await readChunked(body);
        } break;
        case HTTPContentEncoding::Deflate: {
            auto [r, w] = co_await co_await pipe_stream();
            FutureGroup group;
            group.add(pipe_bind(std::move(w),
                                &std::decay_t<decltype(*this)>::readChunked,
                                this));
            group.add([this, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await readChunked(w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            });
            group.add(zlib_deflate(r, body));
            co_await co_await group.wait();
        } break;
        case HTTPContentEncoding::Gzip: {
            OwningStream pin, pout;
            auto pid = co_await co_await ProcessBuilder()
                           .path("gzip"sv)
                           .arg("-d"sv)
                           .arg("-"sv)
                           .pipe_in(0, pin)
                           .pipe_out(1, pout)
                           .spawn();
            FutureGroup group;
            group.add(
                [this, pin = std::move(pin)]() mutable -> Task<Expected<>> {
                    co_await co_await readChunked(pin);
                    co_await co_await pin.flush();
                    co_await pin.close();
                    co_return {};
                });
            group.add(pipe_forward(pout, body));
            co_await co_await group.wait();
            co_await co_await wait_process(pid);
        } break;
        };
        co_return {};
    }

    Task<Expected<void, std::errc>> readEncoded(std::string &body) {
        using namespace std::string_view_literals;
        switch (mContentEncoding) {
        case HTTPContentEncoding::Identity: {
            co_await co_await readChunkedString(body);
        } break;
        default: {
            auto os = make_stream<OStringStream>(body);
            co_await co_await readEncoded(os);
        } break;
        };
        co_return {};
    }

#if CO_ASYNC_DEBUG
    void checkPhase(int from, int to) {
        // debug(), from, to, this;
        if (mPhase != from) [[unlikely]] {
            throw std::logic_error(
                "HTTPProtocol member function calling order wrong (phase = " +
                to_string(mPhase) + ", from = " + to_string(from) +
                ", to = " + to_string(to) + ")");
        }
        mPhase = to;
    }

private:
    int mPhase = 0;
#else
    void checkPhase(int from, int to) {}
#endif

public:
    Task<Expected<void, std::errc>>
    writeBodyStream(BorrowedStream &body) override {
        checkPhase(1, 0);
        co_await co_await writeEncoded(body);
        co_await co_await sock.flush();
        co_return {};
    }

    Task<Expected<void, std::errc>> writeBody(std::string_view body) override {
        checkPhase(1, 0);
        co_await co_await writeEncoded(body);
        co_await co_await sock.flush();
        co_return {};
    }

    Task<Expected<void, std::errc>>
    readBodyStream(BorrowedStream &body) override {
        checkPhase(-1, 0);
        co_await co_await readEncoded(body);
        co_return {};
    }

    Task<Expected<void, std::errc>> readBody(std::string &body) override {
        checkPhase(-1, 0);
        co_await co_await readEncoded(body);
        co_return {};
    }

    Task<Expected<void, std::errc>>
    writeRequest(HTTPRequest const &req) override {
        checkPhase(0, 1);
        using namespace std::string_view_literals;
        co_await co_await sock.puts(req.method);
        co_await co_await sock.putchar(' ');
        co_await co_await sock.puts(req.uri.dump());
        co_await co_await sock.puts(" HTTP/1.1\r\n"sv);
        co_await co_await dumpHeaders(req.headers);
        mContentEncoding = HTTPContentEncoding::Identity;
        co_return {};
    }

    void initServerState() override {
#if CO_ASYNC_DEBUG
        mPhase = 0;
#endif
    }

    void initClientState() override {
#if CO_ASYNC_DEBUG
        mPhase = 0;
#endif
    }

    Task<Expected<void, std::errc>> readRequest(HTTPRequest &req) override {
        checkPhase(0, -1);
        using namespace std::string_view_literals;
        std::string line;
        if (!co_await sock.getline(line, "\r\n"sv) || line.empty()) {
            co_return Unexpected{std::errc::broken_pipe};
        }
        auto pos = line.find(' ');
        if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
            co_return Unexpected{std::errc::protocol_error};
        }
        req.method = line.substr(0, pos);
        auto pos2 = line.find(' ', pos + 1);
        if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
            co_return Unexpected{std::errc::protocol_error};
        }
        req.uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));

        co_await co_await parseHeaders(req.headers);
        handleContentEncoding(req.headers);
        handleAcceptEncoding(req.headers);
        co_return {};
    }

    Task<Expected<void, std::errc>>
    writeResponse(HTTPResponse const &res) override {
        checkPhase(0, 1);
        using namespace std::string_view_literals;
        co_await co_await sock.puts("HTTP/1.1 "sv);
        co_await co_await sock.puts(to_string(res.status));
        co_await co_await sock.putchar(' ');
        co_await co_await sock.puts(getHTTPStatusName(res.status));
        co_await co_await sock.puts("\r\n"sv);
        co_await co_await dumpHeaders(res.headers);
        mContentEncoding = HTTPContentEncoding::Identity;
        co_return {};
    }

    Task<Expected<void, std::errc>> readResponse(HTTPResponse &res) override {
        checkPhase(0, -1);
        using namespace std::string_view_literals;
        std::string line;
        if (!co_await sock.getline(line, "\r\n"sv) || line.empty())
            [[unlikely]] {
            co_return Unexpected{std::errc::broken_pipe};
        }
        if (line.size() <= 9 || line.substr(0, 7) != "HTTP/1."sv ||
            line[8] != ' ') [[unlikely]] {
            co_return Unexpected{std::errc::protocol_error};
        }
        if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
            res.status = *statusOpt;
        } else [[unlikely]] {
            co_return Unexpected{std::errc::protocol_error};
        }
        co_await co_await parseHeaders(res.headers);
        handleContentEncoding(res.headers);
        co_return {};
    }
};

struct HTTPProtocolVersion2 : HTTPProtocolVersion11 {
    using HTTPProtocolVersion11::HTTPProtocolVersion11;

    void initServerState() override {
        HTTPProtocolVersion11::initServerState();
    }

    void initClientState() override {
        HTTPProtocolVersion11::initClientState();
    }

    Task<Expected<void, std::errc>>
    writeBodyStream(BorrowedStream &body) override {
        checkPhase(1, 0);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>>
    readBodyStream(BorrowedStream &body) override {
        checkPhase(-1, 0);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>> writeBody(std::string_view body) override {
        checkPhase(1, 0);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>> readBody(std::string &body) override {
        checkPhase(-1, 0);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>>
    writeRequest(HTTPRequest const &req) override {
        checkPhase(0, 1);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>> readRequest(HTTPRequest &req) override {
        checkPhase(0, -1);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>>
    writeResponse(HTTPResponse const &res) override {
        checkPhase(0, 1);
        co_return Unexpected{std::errc::function_not_supported};
    }

    Task<Expected<void, std::errc>> readResponse(HTTPResponse &res) override {
        checkPhase(0, -1);
        co_return Unexpected{std::errc::function_not_supported};
    }
};

} // namespace co_async

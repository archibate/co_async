#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/string_stream.hpp>
#include <co_async/iostream/zlib_stream.hpp>
#include <co_async/net/http_protocol.hpp>
#include <co_async/net/http_string_utils.hpp>
#include <co_async/net/uri.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/process.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
HTTPContentEncoding
HTTPProtocolVersion11::httpContentEncodingByName(std::string_view name) {
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

Task<Expected<>> HTTPProtocolVersion11::parseHeaders(HTTPHeaders &headers) {
    using namespace std::string_view_literals;
    String line;
    while (true) {
        line.clear();
        co_await co_await sock.getline(line, "\r\n"sv);
        if (line.empty()) {
            break;
        }
        auto pos = line.find(':');
        if (pos == line.npos || pos == line.size() - 1 || line[pos + 1] != ' ')
            [[unlikely]] {
            co_return std::errc::protocol_error;
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

Task<Expected<>>
HTTPProtocolVersion11::dumpHeaders(HTTPHeaders const &headers) {
    using namespace std::string_view_literals;
    for (auto const &[k, v]: headers) {
        co_await co_await sock.puts(k);
        co_await co_await sock.puts(": "sv);
        co_await co_await sock.puts(v);
        co_await co_await sock.puts("\r\n"sv);
    }
    // co_await co_await sock.puts("connection: keep-alive\r\n"sv);
    co_return {};
}

void HTTPProtocolVersion11::handleContentEncoding(HTTPHeaders &headers) {
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

void HTTPProtocolVersion11::handleAcceptEncoding(HTTPHeaders &headers) {
    using namespace std::string_view_literals;
    if (auto acceptEnc = headers.get("accept-encoding"sv)) {
        mAcceptEncoding = std::move(*acceptEnc);
        headers.erase("accept-encoding"sv);
    } else {
        mAcceptEncoding.clear();
    }
}

void HTTPProtocolVersion11::negotiateAcceptEncoding(
    HTTPHeaders &headers, std::span<HTTPContentEncoding const> encodings) {
    using namespace std::string_view_literals;
    mContentEncoding = HTTPContentEncoding::Identity;
    if (!mAcceptEncoding.empty()) {
        for (std::string_view encName: split_string(mAcceptEncoding, ", "sv)) {
            if (auto i = encName.find(';'); i != encName.npos) {
                encName = encName.substr(0, i);
            }
            auto enc = httpContentEncodingByName(encName);
            if (enc != HTTPContentEncoding::Identity) [[likely]] {
                if (std::find(encodings.begin(), encodings.end(), enc) !=
                    encodings.end()) {
                    mContentEncoding = enc;
                    break;
                }
            }
        }
    }
}

Task<Expected<>> HTTPProtocolVersion11::writeChunked(BorrowedStream &body) {
    using namespace std::string_view_literals;
    co_await co_await sock.puts("transfer-encoding: chunked\r\n\r\n"sv);
    do {
        auto bufSpan = body.peekbuf();
        auto n = bufSpan.size();
        /* debug(), std::string_view(bufSpan.data(), n); */
        if (n > 0) {
            char buf[sizeof(n) * 2 + 2] = {}, *ep = buf;
            do {
                *ep++ = "0123456789ABCDEF"[n & 15];
            } while (n >>= 4);
            std::reverse(buf, ep);
            *ep++ = '\r';
            *ep++ = '\n';
            co_await co_await sock.puts(
                std::string_view{buf, static_cast<std::size_t>(ep - buf)});
            co_await co_await sock.putspan(bufSpan);
            co_await co_await sock.puts("\r\n"sv);
            co_await co_await sock.flush();
        }
    } while (co_await body.fillbuf());
    co_await co_await sock.puts("0\r\n\r\n"sv);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>>
HTTPProtocolVersion11::writeChunkedString(std::string_view body) {
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

Task<Expected<>> HTTPProtocolVersion11::readChunked(BorrowedStream &body) {
    using namespace std::string_view_literals;
    if (mContentLength) {
        if (auto n = *mContentLength; n > 0) {
            String line;
            co_await co_await sock.getn(line, n);
            co_await co_await body.puts(line);
            co_await co_await body.flush();
        }
    } else {
        String line;
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
            co_await co_await body.flush();
            co_await co_await sock.dropn(2);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readChunkedString(String &body) {
    using namespace std::string_view_literals;
    if (mContentLength) {
        if (auto n = *mContentLength; n > 0) {
            co_await co_await sock.getn(body, n);
        }
    } else {
        String line;
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

Task<Expected<>> HTTPProtocolVersion11::writeEncoded(BorrowedStream &body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await writeChunked(body);
    } break;
    case HTTPContentEncoding::Deflate: {
        co_await co_await sock.puts("content-encoding: deflate\r\n"sv);
        auto [r, w] = pipe_stream();
        co_await co_await when_all(
            co_bind([&body, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await zlib_deflate(body, w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            }),
            writeChunked(r));
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
        co_await co_await when_all(pipe_forward(body, pin), writeChunked(pout));
        co_await co_await wait_process(pid);
    } break;
    };
    co_return {};
}

Task<Expected<>>
HTTPProtocolVersion11::writeEncodedString(std::string_view body) {
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

Task<Expected<>> HTTPProtocolVersion11::readEncoded(BorrowedStream &body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await readChunked(body);
    } break;
    case HTTPContentEncoding::Deflate: {
        auto [r, w] = pipe_stream();
        co_await co_await when_all(
            pipe_bind(std::move(w), &std::decay_t<decltype(*this)>::readChunked,
                      this),
            co_bind([this, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await readChunked(w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            }),
            zlib_deflate(r, body));
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
        co_await co_await when_all(
            co_bind([this, pin = std::move(pin)]() mutable -> Task<Expected<>> {
                co_await co_await readChunked(pin);
                co_await co_await pin.flush();
                co_await pin.close();
                co_return {};
            }),
            pipe_forward(pout, body));
        co_await co_await wait_process(pid);
    } break;
    };
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readEncodedString(String &body) {
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
void HTTPProtocolVersion11::checkPhase(int from, int to) {
    // debug(), from, to, this;
    if (mPhase != from) [[unlikely]] {
        throw std::logic_error(
            "HTTPProtocol member function calling order wrong (phase = " +
            std::to_string(mPhase) + ", from = " + std::to_string(from) +
            ", to = " + std::to_string(to) + ")");
    }
    mPhase = to;
}
#else
void HTTPProtocolVersion11::checkPhase(int from, int to) {}
#endif
Task<Expected<>> HTTPProtocolVersion11::writeBodyStream(BorrowedStream &body) {
    checkPhase(1, 0);
    co_await co_await writeEncoded(body);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeBody(std::string_view body) {
    checkPhase(1, 0);
    co_await co_await writeEncodedString(body);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readBodyStream(BorrowedStream &body) {
    checkPhase(-1, 0);
    co_await co_await readEncoded(body);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readBody(String &body) {
    checkPhase(-1, 0);
    co_await co_await readEncodedString(body);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeRequest(HTTPRequest const &req) {
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

void HTTPProtocolVersion11::initServerState() {
#if CO_ASYNC_DEBUG
    mPhase = 0;
#endif
}

void HTTPProtocolVersion11::initClientState() {
#if CO_ASYNC_DEBUG
    mPhase = 0;
#endif
}

Task<Expected<>> HTTPProtocolVersion11::readRequest(HTTPRequest &req) {
    checkPhase(0, -1);
    using namespace std::string_view_literals;
    String line;
    co_await co_await sock.getline(line, "\r\n"sv);
    auto pos = line.find(' ');
    if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    req.method = line.substr(0, pos);
    auto pos2 = line.find(' ', pos + 1);
    if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    req.uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));
    co_await co_await parseHeaders(req.headers);
    handleContentEncoding(req.headers);
    handleAcceptEncoding(req.headers);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeResponse(HTTPResponse const &res) {
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

Task<Expected<>> HTTPProtocolVersion11::readResponse(HTTPResponse &res) {
    checkPhase(0, -1);
    using namespace std::string_view_literals;
    String line;
    co_await co_await sock.getline(line, "\r\n"sv);
    if (line.size() <= 9 || line.substr(0, 7) != "HTTP/1."sv || line[8] != ' ')
        [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
        res.status = *statusOpt;
    } else [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    co_await co_await parseHeaders(res.headers);
    handleContentEncoding(res.headers);
    co_return {};
}

HTTPProtocolVersion11::HTTPProtocolVersion11(OwningStream sock)
    : HTTPProtocol(std::move(sock)) {}

HTTPProtocolVersion11::~HTTPProtocolVersion11() = default;
} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/net/uri.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/simple_map.hpp>

namespace co_async {
struct HTTPHeaders : SimpleMap<String, String> {
    using SimpleMap<String, String>::SimpleMap;
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
    String method{"GET", 3};
    URI uri{String{"/", 1}, {}};
    HTTPHeaders headers{};

    auto repr() const {
        return std::make_tuple(method, uri, headers);
    }
};

struct HTTPResponse {
    int status{0};
    HTTPHeaders headers{};

    auto repr() const {
        return std::make_tuple(status, headers);
    }
};

struct HTTPProtocol {
public:
    OwningStream sock;

    explicit HTTPProtocol(OwningStream sock) : sock(std::move(sock)) {}

    HTTPProtocol(HTTPProtocol &&) = delete;
    virtual ~HTTPProtocol() = default;
    virtual void initServerState() = 0;
    virtual void initClientState() = 0;
    virtual Task<Expected<>> writeBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<>> readBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<>> writeBody(std::string_view body) = 0;
    virtual Task<Expected<>> readBody(String &body) = 0;
    virtual Task<Expected<>> writeRequest(HTTPRequest const &req) = 0;
    virtual Task<Expected<>> readRequest(HTTPRequest &req) = 0;
    virtual Task<Expected<>> writeResponse(HTTPResponse const &res) = 0;
    virtual Task<Expected<>> readResponse(HTTPResponse &res) = 0;
};

struct HTTPProtocolVersion11 : HTTPProtocol {
    using HTTPProtocol::HTTPProtocol;

protected:
    HTTPContentEncoding mContentEncoding;
    String mAcceptEncoding;
    std::optional<std::size_t> mContentLength;
    HTTPContentEncoding httpContentEncodingByName(std::string_view name);
    Task<Expected<>> parseHeaders(HTTPHeaders &headers);
    Task<Expected<>> dumpHeaders(HTTPHeaders const &headers);
    void handleContentEncoding(HTTPHeaders &headers);
    void handleAcceptEncoding(HTTPHeaders &headers);
    void
    negotiateAcceptEncoding(HTTPHeaders &headers,
                            std::span<HTTPContentEncoding const> encodings);
    Task<Expected<>> writeChunked(BorrowedStream &body);
    Task<Expected<>> writeChunkedString(std::string_view body);
    Task<Expected<>> readChunked(BorrowedStream &body);
    Task<Expected<>> readChunkedString(String &body);
    Task<Expected<>> writeEncoded(BorrowedStream &body);
    Task<Expected<>> writeEncodedString(std::string_view body);
    Task<Expected<>> readEncoded(BorrowedStream &body);
    Task<Expected<>> readEncodedString(String &body);
#if CO_ASYNC_DEBUG
    void checkPhase(int from, int to);

private:
    int mPhase = 0;
#else
    void checkPhase(int from, int to);
#endif
public:
    Task<Expected<>> writeBodyStream(BorrowedStream &body) override;
    Task<Expected<>> writeBody(std::string_view body) override;
    Task<Expected<>> readBodyStream(BorrowedStream &body) override;
    Task<Expected<>> readBody(String &body) override;
    Task<Expected<>> writeRequest(HTTPRequest const &req) override;
    void initServerState() override;
    void initClientState() override;
    Task<Expected<>> readRequest(HTTPRequest &req) override;
    Task<Expected<>> writeResponse(HTTPResponse const &res) override;
    Task<Expected<>> readResponse(HTTPResponse &res) override;
    explicit HTTPProtocolVersion11(OwningStream sock);
    ~HTTPProtocolVersion11() override;
};

struct HTTPProtocolVersion2 : HTTPProtocolVersion11 {
    using HTTPProtocolVersion11::HTTPProtocolVersion11;
};
} // namespace co_async

#pragma once /*{export module co_async:http.http_client;}*/

#include <co_async/std.hpp>                  /*{import std;}*/
#include <co_async/awaiter/task.hpp>         /*{import :awaiter.task;}*/
#include <co_async/http/http11.hpp>          /*{import :http.http11;}*/
#include <co_async/http/http_status_code.hpp>/*{import :http.http_status_code;}*/
#include <co_async/utils/string_utils.hpp>   /*{import :utils.string_utils;}*/
#include <co_async/system/socket.hpp>        /*{import :system.socket;}*/
#include <co_async/system/fs.hpp>            /*{import :system.fs;}*/
#include <co_async/http/uri.hpp>             /*{import :http.uri;}*/
#include <co_async/http/http11.hpp>          /*{import :http.http11;}*/
#include <co_async/iostream/socket_stream.hpp>/*{import :iostream.socket_stream;}*/
#include <co_async/iostream/ssl_socket_stream.hpp>/*{import :iostream.ssl_socket_stream;}*/

namespace co_async {

/*[export]*/ struct HTTPConnection {
    HTTPConnection() = default;
    HTTPConnection(HTTPConnection &&) = delete;

    virtual ~HTTPConnection() = default;

protected:
    virtual Task<> doConnect(std::string_view host, int port, std::string_view proxy, std::chrono::nanoseconds timeout) = 0;
    virtual int getProtocolVersion() = 0;
    virtual Task<> doWriteHeader(HTTPRequest const &req) = 0;
    virtual Task<> doWriteBody(std::string_view body) = 0;
    virtual Task<> doWriteBodyStream(FileStream &body) = 0;
    virtual Task<> doWriteNoBody() = 0;
    virtual Task<bool> doReadHeader(HTTPResponse &res) = 0;
    virtual Task<std::string> doReadBody() = 0;
    virtual Task<> doReadBodyStream(FileStream &body) = 0;

    std::string mHostName;

public:
    std::atomic_bool mOwned{false};

    Task<> connectTo(std::string_view hostName, int defaultPort, std::string_view proxy, std::chrono::nanoseconds timeout) {
        int port = defaultPort;
        auto host = hostName;
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1))) [[likely]] {
                port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        co_await doConnect(host, port, proxy, timeout);
        mHostName = hostName;
    }

    Task<> write_header(HTTPRequest req) {
        // TODO: auto-reconnect?
        using namespace std::string_literals;
        req.headers.insert("user-agent"s, "co_async/0.0.1"s);
        req.headers.insert("host"s, mHostName);
        /* req.headers.insert("accept-encoding"s, "gzip, deflate, br, compress"s); */
        /* req.headers.insert("accept-encoding"s, "deflate"s); */
        req.headers.insert("accept"s, "*/*"s);
        co_await doWriteHeader(req);
    }

    Task<> write_body(std::string_view body) {
        co_await doWriteBody(body);
    }

    Task<> write_body_stream(FileStream &body) {
        co_await doWriteBodyStream(body);
    }

    Task<> write_nobody() {
        co_await doWriteNoBody();
    }

    Task<> read_header(HTTPResponse &res) {
        if (!co_await doReadHeader(res)) [[unlikely]] {
            throw std::runtime_error("failed to read header");
        }
    }

    Task<std::string> read_body() {
        co_return co_await doReadBody();
    }

    Task<> read_body_stream(FileStream &body) {
        co_await doReadBodyStream(body);
    }
};

template <class HTTP>
struct HTTPConnectionImpl : HTTPConnection {
protected:
    std::optional<HTTP> mHttp;

public:
    Task<> doWriteHeader(HTTPRequest const &req) override {
        co_await mHttp->write_header(req);
    }

    Task<> doWriteBody(std::string_view body) override {
        co_await mHttp->write_body(body);
    }

    Task<> doWriteBodyStream(FileStream &body) override {
        co_await mHttp->write_body_stream(body);
    }

    Task<> doWriteNoBody() override {
        co_await mHttp->write_nobody();
    }

    Task<bool> doReadHeader(HTTPResponse &res) override {
        co_return co_await mHttp->read_header(res);
    }

    Task<std::string> doReadBody() override {
        co_return co_await mHttp->read_body();
    }

    Task<> doReadBodyStream(FileStream &body) override {
        co_await mHttp->read_body_stream(body);
    }
};

/*[export]*/ struct HTTPConnectionHTTP : HTTPConnectionImpl<HTTPProtocol<SocketStream>> {
    Task<> doConnect(std::string_view host, int port, std::string_view proxy, std::chrono::nanoseconds timeout) override {
        mHttp.emplace(
            co_await SocketStream::connect(std::string(host).c_str(), port, proxy, timeout));
    }

    int getProtocolVersion() override {
        return 1;
    }
};

/*[export]*/ struct HTTPConnectionHTTPS : HTTPConnectionImpl<HTTPProtocol<SSLClientSocketStream>> {
protected:
    static inline SSLClientTrustAnchor mTrustAnchors;
    static inline char const *const mProtocols[] = {
        // "h2", // TODO: support HTTP/2
        "http/1.1",
    };

public:
    static void load_custom_certificate(std::string_view cert) {
        mTrustAnchors.add(cert);
    }

    static Task<> load_ca_certificates() {
        auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
        if (auto s = co_await fs_stat(path)) [[likely]] {
            if (s->is_readable()) [[likely]] {
                mTrustAnchors.add(co_await file_read(path));
            }
        }
    }

    Task<> doConnect(std::string_view host, int port, std::string_view proxy, std::chrono::nanoseconds timeout) override {
        mHttp.emplace(co_await SSLClientSocketStream::connect(
            std::string(host).c_str(), port, mTrustAnchors, mProtocols, proxy, timeout));
    }

    int getProtocolVersion() override {
        auto p = mHttp->sock.ssl_get_selected_protocol();
        if (p == "h2")
            return 2;
        return 1;
    }
};

/*[export]*/ inline Task<std::unique_ptr<HTTPConnection>> http_connect(std::string_view host, std::chrono::nanoseconds timeout = std::chrono::seconds(5), bool followProxy = true) {
    std::unique_ptr<HTTPConnection> conn;
    if (host.starts_with("https://")) {
        host.remove_prefix(8);
        conn = std::make_unique<HTTPConnectionHTTPS>();
        std::string proxy;
        if (followProxy) {
            if (auto p = std::getenv("https_proxy")) {
                proxy = p;
            }
        }
        co_await conn->connectTo(host, 443, proxy, timeout);
    } else if (host.starts_with("http://")) {
        host.remove_prefix(7);
        conn = std::make_unique<HTTPConnectionHTTP>();
        std::string proxy;
        if (followProxy) {
            if (auto p = std::getenv("http_proxy")) {
                proxy = p;
            }
        }
        co_await conn->connectTo(host, 80, proxy, timeout);
    } else {
        throw std::runtime_error(
            "invalid protocol, must be http:// or https://");
    }
    co_return conn;
}

} // namespace co_async

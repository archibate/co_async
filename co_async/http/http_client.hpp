#pragma once

#include "co_async/utils/debug.hpp"
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/http/http11.hpp>
#include <co_async/http/http_status_code.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/http/uri.hpp>
#include <co_async/http/http11.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>
#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/threading/future.hpp>

namespace co_async {

inline SSLClientTrustAnchor gTrustAnchors;

struct HTTPConnection {
private:
    std::unique_ptr<HTTPProtocol> mHttp;
    HTTPHeaders mDefaultHeaders;

    void updateHeaders(HTTPRequest &req) {
        for (auto const &[k, v]: mDefaultHeaders) {
            req.headers.insert(k, v);
        }
    }

public:
    HTTPConnection(std::unique_ptr<HTTPProtocol> http,
                   std::string_view hostName)
        : mHttp(std::move(http)) {
        using namespace std::string_literals;
        mDefaultHeaders.insert("host", std::string(hostName));
        mDefaultHeaders.insert("user-agent", "co_async/0.0.1"s);
    }

    Task<> request(HTTPRequest req, std::string_view in, HTTPResponse &res,
                   std::string &out) {
        updateHeaders(req);
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBody(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBody(out);
    }

    Task<> request(HTTPRequest req, std::string_view in, HTTPResponse &res,
                   OStream &out) {
        updateHeaders(req);
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBody(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBodyStream(out);
    }

    Task<> request(HTTPRequest req, IStream &in, HTTPResponse &res,
                   std::string &out) {
        updateHeaders(req);
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBody(out);
    }

    Task<> request(HTTPRequest req, IStream &in, HTTPResponse &res,
                   OStream &out) {
        updateHeaders(req);
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBodyStream(out);
    }

    Task<> request(HTTPRequest req, IStream &in,
                   FutureReference<HTTPResponse> const &res, OStream &out) {
        updateHeaders(req);
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBodyStream(out);
    }
};

inline std::tuple<std::string, int> parseHostAndPort(std::string_view hostName,
                                                     int defaultPort) {
    int port = defaultPort;
    auto host = hostName;
    if (auto i = host.rfind(':'); i != host.npos) {
        if (auto portOpt = from_string<int>(host.substr(i + 1))) [[likely]] {
            port = *portOpt;
            host.remove_suffix(host.size() - i);
        }
    }
    return {std::string(host), port};
}

inline Task<HTTPConnection>
http_connect(std::string_view host,
             std::chrono::nanoseconds timeout = std::chrono::seconds(5),
             bool followProxy = true) {
    if (host.starts_with("https://")) {
        host.remove_prefix(8);
        std::string proxy;
        if (followProxy) {
            if (auto p = std::getenv("https_proxy")) {
                proxy = p;
            }
        }
        auto [h, p] = parseHostAndPort(host, 443);
        static char const *const protocols[] = {
            // "h2",
            "http/1.1",
        };
        auto sock = std::make_unique<SSLClientSocketStream>(
            co_await SSLClientSocketStream::connect(h.c_str(), p, gTrustAnchors,
                                                    protocols, proxy, timeout));
        std::unique_ptr<HTTPProtocol> http;
        if (sock->ssl_get_selected_protocol() == "h2") {
            throw std::runtime_error("http/2 not implemented yet");
        } else {
            http = std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        }
        co_return HTTPConnection(std::move(http), host);
    } else if (host.starts_with("http://")) {
        host.remove_prefix(7);
        std::string proxy;
        if (followProxy) {
            if (auto p = std::getenv("http_proxy")) {
                proxy = p;
            }
        }
        auto [h, p] = parseHostAndPort(host, 80);
        auto sock = std::make_unique<SocketStream>(
            co_await SocketStream::connect(h.c_str(), p, proxy, timeout));
        auto http = std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        co_return HTTPConnection(std::move(http), host);
    } else {
        throw std::runtime_error(
            "invalid protocol, must be http:// or https://");
    }
}

inline Task<> https_load_ca_certificates() {
    auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
    if (auto s = co_await fs_stat(path)) [[likely]] {
        if (s->is_readable()) [[likely]] {
            gTrustAnchors.add(co_await file_read(path));
        }
    }
}

} // namespace co_async

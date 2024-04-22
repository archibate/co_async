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
#include <co_async/iostream/pipe_stream.hpp>/*{import :iostream.pipe_stream;}*/
#include <co_async/threading/condition_variable.hpp>/*{import :threading.condition_variable;}*/

namespace co_async {

/*[export]*/ struct HTTPConnection {
protected:
    std::unique_ptr<HTTPProtocol> mHttp;
    std::string mHostName;

public:
    HTTPConnection(std::unique_ptr<HTTPProtocol> http, std::string_view hostName) : mHttp(std::move(http)), mHostName(hostName) {
    }

    std::string const &hostName() const noexcept {
        return mHostName;
    }

    Task<> request(HTTPRequest const &req, std::string_view in, HTTPResponse &res, std::string &out) {
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBody(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBody(out);
    }

    Task<> request(HTTPRequest const &req, std::string_view in, HTTPResponse &res, OStream &out) {
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBody(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBodyStream(out);
    }

    Task<> request(HTTPRequest const &req, IStream &in, HTTPResponse &res, std::string &out) {
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBody(out);
    }

    Task<> request(HTTPRequest const &req, IStream &in, HTTPResponse &res, OStream &out) {
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await mHttp->readBodyStream(out);
    }

    Task<> request(HTTPRequest const &req, IStream &in, HTTPResponse &res, OStream &out, ConditionVariable &resReady) {
        co_await mHttp->writeRequest(req);
        co_await mHttp->writeBodyStream(in);
        co_await mHttp->readResponse(res);
        co_await resReady.notify_one();
        co_await mHttp->readBodyStream(out);
    }
};

inline std::tuple<std::string, int> parseHostAndPort(std::string_view hostName, int defaultPort) {
    int port = defaultPort;
    auto host = hostName;
    if (auto i = host.rfind(':'); i != host.npos) {
        if (auto portOpt = from_string<int>(host.substr(i + 1))) [[likely]] {
            port = *portOpt;
            host.remove_suffix(host.size() - i);
        }
    }
    return {host, port};
}

/*[export]*/ inline Task<HTTPConnection> http_connect(std::string_view host, std::chrono::nanoseconds timeout = std::chrono::seconds(5), bool followProxy = true) {
    if (host.starts_with("https://")) {
        host.remove_prefix(8);
        conn = std::make_unique<HTTPConnectionHTTPS>();
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
        auto sock = co_await SSLClientSocketStream::connect(h.c_str(), p, ta, protocols, proxy, timeout);
        std::unique_ptr<HTTPProtocol> http;
        if (sock.ssl_get_selected_protocol() == "h2") {
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
        auto sock = co_await SSLClientSocketStream::connect(h.c_str(), p, proxy, timeout);
        auto http = std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        co_return HTTPConnection(std::move(http), host);
    } else {
        throw std::runtime_error(
            "invalid protocol, must be http:// or https://");
    }
    co_return conn;
}

inline SSLClientTrustAnchor gTrustAnchors;

/*[export]*/ inline Task<> https_load_ca_certificates() {
    auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
    if (auto s = co_await fs_stat(path)) [[likely]] {
        if (s->is_readable()) [[likely]] {
            mTrustAnchors.add(co_await file_read(path));
        }
    }
}

} // namespace co_async

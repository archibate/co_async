#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/http/http11.hpp>
#include <co_async/http/http_status_code.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/pipe.hpp>
#include <co_async/http/uri.hpp>
#include <co_async/http/http11.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>

namespace co_async {

enum class HTTPRouteMode {
    SuffixAny = 0, // "/a-9\\*g./.."
    SuffixName,    // "/a"
    SuffixPath,    // "/a/b/c"
};

struct HTTPServer {
    struct IO {
        explicit IO(HTTPProtocol *http) noexcept : mHttp(http) {}

        HTTPRequest request;

        Task<Expected<>> readHTTPRequest() {
            return mHttp->readRequest(request);
        }

        Task<Expected<std::string>> request_body() {
#if CO_ASYNC_DEBUG
            if (mBodyRead) [[unlikely]] {
                throw std::runtime_error(
                    "request_body() may only be called once");
            }
#endif
            mBodyRead = true;
            std::string body;
            co_await co_await mHttp->readBody(body);
            co_return body;
        }

        Task<Expected<>> request_body_stream(OStream &out) {
#if CO_ASYNC_DEBUG
            if (mBodyRead) [[unlikely]] {
                throw std::runtime_error(
                    "request_body() may only be called once");
            }
#endif
            mBodyRead = true;
            co_await co_await mHttp->readBodyStream(out);
            co_return {};
        }

        Task<Expected<>> response(HTTPResponse const &resp, std::string_view content) {
            if (!mBodyRead) {
                co_await co_await request_body();
            }
            co_await co_await mHttp->writeResponse(resp);
            co_await co_await mHttp->writeBody(content);
            mBodyRead = false;
            co_return {};
        }

        Task<Expected<>> response(HTTPResponse const &resp, IStream &body) {
            if (!mBodyRead) {
                co_await co_await request_body();
            }
            co_await co_await mHttp->writeResponse(resp);
            co_await co_await mHttp->writeBodyStream(body);
            mBodyRead = false;
            co_return {};
        }

    private:
        HTTPProtocol *mHttp;
        bool mBodyRead = false;
    };

    using HTTPHandler = std::function<Task<Expected<>>(IO &)>;
    using HTTPPrefixHandler = std::function<Task<Expected<>>(IO &, std::string_view)>;

    HTTPServer() = default;
    HTTPServer(HTTPServer &&) = delete;

    void route(std::string_view methods, std::string_view path,
               HTTPHandler handler) {
        mRoutes.insert_or_assign(
            std::string(path),
            {handler, split_string(upper_string(methods), ' ').collect()});
    }

    void route(std::string_view methods, std::string_view prefix,
               HTTPRouteMode mode, HTTPPrefixHandler handler) {
        auto it =
            std::lower_bound(mPrefixRoutes.begin(), mPrefixRoutes.end(), prefix,
                             [](auto const &item, auto const &prefix) {
                                 return item.first.size() > prefix.size();
                             });
        mPrefixRoutes.insert(
            it, {std::string(prefix),
                 {handler, mode,
                  split_string(upper_string(methods), ' ').collect()}});
    }

    void route(HTTPHandler handler) {
        mDefaultRoute = handler;
    }

    Task<std::unique_ptr<HTTPProtocol>>
    prepareHTTPS(SocketHandle handle,
                 SSLServerCertificate const &cert, SSLPrivateKey const &skey,
                 SSLSessionCache *cache = nullptr) const {
        SSLServerSocketStream sock(std::move(handle), cert, skey, cache);
        if (sock.ssl_is_protocol_offered("h2")) {
            /* co_return std::make_unique<HTTPProtocolVersion11>(std::make_unique<SSLServerSocketStream>(std::move(sock))); todo: support http/2! */
        }
        co_return std::make_unique<HTTPProtocolVersion11>(
            std::make_unique<SSLServerSocketStream>(std::move(sock)));
    }

    Task<std::unique_ptr<HTTPProtocol>> prepareHTTP(SocketHandle handle) const {
        SocketStream sock(std::move(handle));
        co_return std::make_unique<HTTPProtocolVersion11>(
            std::make_unique<SocketStream>(std::move(sock)));
    }

    Task<Expected<>> handle_http(SocketHandle handle) const {
        co_await co_await doHandleConnection(co_await prepareHTTP(std::move(handle)));
        co_return {};
    }

    Task<Expected<>> handle_http_redirect_to_https(SocketHandle handle) const {
        using namespace std::string_literals;
        auto http = co_await prepareHTTP(std::move(handle));
        IO io(http.get());
        while (co_await io.readHTTPRequest()) {
            if (auto host = io.request.headers.get("host")) {
                auto location = "https://"s + *host + io.request.uri.dump();
                HTTPResponse res = {
                    .status = 302,
                    .headers = {
                        {"location", location},
                        {"content-type", "text/plain"},
                    },
                };
                co_await co_await io.response(res, location);
            } else {
                co_await co_await make_error_response(io, 403);
            }
        }
    }

    Task<Expected<>> handle_https(SocketHandle handle,
                 SSLServerCertificate const &cert, SSLPrivateKey const &skey,
                 SSLSessionCache *cache = nullptr) const {
        co_await co_await doHandleConnection(co_await prepareHTTPS(std::move(handle), cert, skey, cache));
        co_return {};
    }

    Task<Expected<>> doHandleConnection(std::unique_ptr<HTTPProtocol> http) const {
        IO io(http.get());
        while (co_await io.readHTTPRequest()) {
            co_await co_await doHandleRequest(io);
        }
        co_return {};
    }

    static Task<Expected<>> make_error_response(IO &io, int status) {
        auto error =
            to_string(status) + " " + std::string(getHTTPStatusName(status));
        HTTPResponse res{
            .status = status,
            .headers =
                {
                    {"content-type", "text/html;charset=utf-8"},
                },
        };
        co_await co_await io.response(
            res,
            "<html><head><title>" + error +
                "</title></head><body><center><h1>" + error +
                "</h1></center><hr><center>co_async</center></body></html>");
        co_return {};
    }

private:
    struct Route {
        HTTPHandler mHandler;
        std::vector<std::string> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }
    };

    struct PrefixRoute {
        HTTPPrefixHandler mHandler;
        HTTPRouteMode mRouteMode;
        std::vector<std::string> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }

        bool checkSuffix(std::string_view &suffix) const {
            switch (mRouteMode) {
            case HTTPRouteMode::SuffixName: {
                if (suffix.starts_with('/')) {
                    suffix.remove_prefix(1);
                }
                if (suffix.empty()) [[unlikely]] {
                    return false;
                }
                // make sure no '/' in suffix
                if (suffix.find('/') != std::string_view::npos) [[unlikely]] {
                    return false;
                }
                return true;
            }
            case HTTPRouteMode::SuffixPath: {
                if (suffix.starts_with('/')) {
                    suffix.remove_prefix(1);
                }
                if (suffix.empty()) {
                    return true;
                }
                // make sure no ".." or "." after spliting by '/'
                for (auto const &part: split_string(suffix, '/')) {
                    switch (part.size()) {
                    case 2:
                        if (part[0] == '.' && part[1] == '.') [[unlikely]] {
                            return false;
                        }
                        break;
                    case 1:
                        if (part[0] == '.') [[unlikely]] {
                            return false;
                        }
                        break;
                    case 0: return false;
                    }
                }
                return true;
            }
            default: return true;
            }
        }
    };

    SimpleMap<std::string, Route> mRoutes;
    std::vector<std::pair<std::string, PrefixRoute>> mPrefixRoutes;
    HTTPHandler mDefaultRoute = [](IO &io) -> Task<Expected<>> {
        co_return co_await make_error_response(io, 404);
    };

    Task<Expected<>> doHandleRequest(IO &io) const {
        if (auto route = mRoutes.at(io.request.uri.path)) {
            if (!route->checkMethod(io.request.method)) [[unlikely]] {
                co_await co_await make_error_response(io, 405);
            }
            co_await co_await route->mHandler(io);
        }
        for (auto const &[prefix, route]: mPrefixRoutes) {
            if (io.request.uri.path.starts_with(prefix)) {
                if (!route.checkMethod(io.request.method)) [[unlikely]] {
                    co_await co_await make_error_response(io, 405);
                    co_return {};
                }
                auto suffix = std::string_view(io.request.uri.path);
                suffix.remove_prefix(prefix.size());
                if (!route.checkSuffix(suffix)) [[unlikely]] {
                    co_await co_await make_error_response(io, 405);
                    co_return {};
                }
                co_await co_await route.mHandler(io, suffix);
            }
        }
        co_await co_await mDefaultRoute(io);
        co_return {};
    }
};

} // namespace co_async

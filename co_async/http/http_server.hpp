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

        Task<std::string> request_body() {
#if CO_ASYNC_DEBUG
            if (mBodyRead) [[unlikely]] {
                throw std::runtime_error(
                    "request_body() may only be called once");
            }
#endif
            mBodyRead = true;
            std::string body;
            if (!co_await mHttp->readBody(body)) {
                throw std::runtime_error("failed to read request body");
            }
            co_return body;
        }

        Task<> request_body_stream(OStream &out) {
#if CO_ASYNC_DEBUG
            if (mBodyRead) [[unlikely]] {
                throw std::runtime_error(
                    "request_body() may only be called once");
            }
#endif
            mBodyRead = true;
            co_await mHttp->readBodyStream(out);
        }

        Task<> response(HTTPResponse const &resp, std::string_view content) {
            if (!mBodyRead) {
                co_await request_body();
            }
            co_await mHttp->writeResponse(resp);
            co_await mHttp->writeBody(content);
            mBodyRead = false;
        }

        Task<> response(HTTPResponse const &resp, IStream &body) {
            if (!mBodyRead) {
                co_await request_body();
            }
            co_await mHttp->writeResponse(resp);
            co_await mHttp->writeBodyStream(body);
            mBodyRead = false;
        }

    private:
        HTTPProtocol *mHttp;
        bool mBodyRead = false;
    };

    using HTTPHandler = std::function<Task<>(IO &)>;
    using HTTPPrefixHandler = std::function<Task<>(IO &, std::string_view)>;

    explicit HTTPServer(SocketListener &listener) : mListener(listener) {}

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
    accept_https(SSLServerCertificate const &cert, SSLPrivateKey const &pkey,
                 SSLSessionCache *cache = nullptr) const {
        auto sock = co_await SSLServerSocketStream::accept(mListener, cert,
                                                           pkey, cache);
        if (sock.ssl_is_protocol_offered("h2")) {
            /* co_return std::make_unique<HTTPProtocolVersion11>(std::make_unique<SSLServerSocketStream>(std::move(sock))); todo: support http/2! */
        }
        co_return std::make_unique<HTTPProtocolVersion11>(
            std::make_unique<SSLServerSocketStream>(std::move(sock)));
    }

    Task<std::unique_ptr<HTTPProtocol>> accept_http() const {
        auto sock = co_await SocketStream::accept(mListener);
        co_return std::make_unique<HTTPProtocolVersion11>(
            std::make_unique<SocketStream>(std::move(sock)));
    }

    Task<> process_connection(std::unique_ptr<HTTPProtocol> http) const {
        IO io(http.get());
        while (co_await http->readRequest(io.request)) {
            co_await doHandleRequest(io);
        }
    }

    static Task<> make_error_response(IO &io, int status) {
        auto error =
            to_string(status) + " " + std::string(getHTTPStatusName(status));
        HTTPResponse res{
            .status = status,
            .headers =
                {
                    {"content-type", "text/html;charset=utf-8"},
                },
        };
        co_await io.response(
            res,
            "<html><head><title>" + error +
                "</title></head><body><center><h1>" + error +
                "</h1></center><hr><center>co_async</center></body></html>");
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

    SocketListener &mListener;
    SimpleMap<std::string, Route> mRoutes;
    std::vector<std::pair<std::string, PrefixRoute>> mPrefixRoutes;
    HTTPHandler mDefaultRoute = [](IO &io) -> Task<> {
        co_await make_error_response(io, 404);
    };

    Task<> doHandleRequest(IO &io) const {
        if (auto route = mRoutes.at(io.request.uri.path)) {
            if (!route->checkMethod(io.request.method)) [[unlikely]] {
                co_await make_error_response(io, 405);
            }
            co_await route->mHandler(io);
        }
        for (auto const &[prefix, route]: mPrefixRoutes) {
            if (io.request.uri.path.starts_with(prefix)) {
                if (!route.checkMethod(io.request.method)) [[unlikely]] {
                    co_await make_error_response(io, 405);
                    co_return;
                }
                auto suffix = std::string_view(io.request.uri.path);
                suffix.remove_prefix(prefix.size());
                if (!route.checkSuffix(suffix)) [[unlikely]] {
                    co_await make_error_response(io, 405);
                    co_return;
                }
                co_await route.mHandler(io, suffix);
            }
        }
        co_await mDefaultRoute(io);
    }
};

} // namespace co_async

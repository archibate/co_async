#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>
#include <co_async/net/http_protocol.hpp>
#include <co_async/net/http_server.hpp>
#include <co_async/net/http_string_utils.hpp>
#include <co_async/net/uri.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/pipe.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/simple_map.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
struct HTTPServer::Impl {
    struct Route {
        HTTPHandler mHandler;
        std::vector<String> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }
    };

    struct PrefixRoute {
        HTTPPrefixHandler mHandler;
        HTTPRouteMode mRouteMode;
        std::vector<String> mMethods;

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

    SimpleMap<String, Route> mRoutes;
    std::vector<std::pair<String, PrefixRoute>> mPrefixRoutes;
    HTTPHandler mDefaultRoute = [](IO &io) -> Task<Expected<>> {
        co_return co_await make_error_response(io, 404);
    };
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(30);
#if CO_ASYNC_DEBUG
    bool mLogRequests = false;
#endif

    Task<Expected<>> doHandleRequest(IO &io) const {
        if (auto route = mRoutes.at(io.request.uri.path)) {
            if (!route->checkMethod(io.request.method)) [[unlikely]] {
                co_await co_await make_error_response(io, 405);
                co_return {};
            }
            co_await co_await route->mHandler(io);
            co_return {};
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
#if CO_ASYNC_DEBUG
                auto ret = co_await route.mHandler(io, suffix);
                if (ret.has_error() && mLogRequests) {
                    std::clog << "SERVER ERROR: " << ret.error() << '\n';
                }
                co_return ret;
#else
                co_await co_await route.mHandler(io, suffix);
                co_return {};
#endif
            }
        }
        co_await co_await mDefaultRoute(io);
        co_return {};
    }
};

Task<Expected<bool>> HTTPServer::IO::readRequestHeader() {
    mHttp->initServerState();
    co_return co_await (co_await mHttp->readRequest(request)).transform([] { return true; }).or_else(eofError(), [] { return false; });
}

Task<Expected<String>> HTTPServer::IO::request_body() {
#if CO_ASYNC_DEBUG
    if (mBodyRead) [[unlikely]] {
        throw std::runtime_error("request_body() may only be called once");
    }
#endif
    mBodyRead = true;
    String body;
    co_await co_await mHttp->readBody(body);
    co_return body;
}

Task<Expected<>> HTTPServer::IO::request_body_stream(OwningStream &out) {
#if CO_ASYNC_DEBUG
    if (mBodyRead) [[unlikely]] {
        throw std::runtime_error("request_body() may only be called once");
    }
#endif
    mBodyRead = true;
    co_await co_await mHttp->readBodyStream(out);
    co_return {};
}

Task<Expected<>> HTTPServer::IO::response(HTTPResponse resp,
                                          std::string_view content) {
#if CO_ASYNC_DEBUG
    mResponseSavedForDebug = resp;
#endif
    if (!mBodyRead) {
        co_await co_await request_body();
    }
    builtinHeaders(resp);
    co_await co_await mHttp->writeResponse(resp);
    co_await co_await mHttp->writeBody(content);
    mBodyRead = false;
    co_return {};
}

Task<Expected<>> HTTPServer::IO::response(HTTPResponse resp,
                                          OwningStream &body) {
#if CO_ASYNC_DEBUG
    mResponseSavedForDebug = resp;
#endif
    if (!mBodyRead) {
        co_await co_await request_body();
    }
    builtinHeaders(resp);
    co_await co_await mHttp->writeResponse(resp);
    co_await co_await mHttp->writeBodyStream(body);
    mBodyRead = false;
    co_return {};
}

void HTTPServer::IO::builtinHeaders(HTTPResponse &res) {
    res.headers.insert("server"_s, "co_async/0.0.1"_s);
    res.headers.insert("accept"_s, "*/*"_s);
    res.headers.insert("accept-ranges"_s, "bytes"_s);
    res.headers.insert("date"_s, httpDateNow());
}

HTTPServer::HTTPServer() : mImpl(std::make_unique<Impl>()) {}

HTTPServer::~HTTPServer() = default;

#if CO_ASYNC_DEBUG
void HTTPServer::enableLogRequests() {
    mImpl->mLogRequests = true;
}
#endif

void HTTPServer::timeout(std::chrono::steady_clock::duration timeout) {
    mImpl->mTimeout = timeout;
}

void HTTPServer::route(std::string_view methods, std::string_view path,
                       HTTPHandler handler) {
    mImpl->mRoutes.insert_or_assign(
        String(path),
        {handler, split_string(upper_string(methods), ' ').collect()});
}

void HTTPServer::route(std::string_view methods, std::string_view prefix,
                       HTTPRouteMode mode, HTTPPrefixHandler handler) {
    auto it = std::lower_bound(mImpl->mPrefixRoutes.begin(),
                               mImpl->mPrefixRoutes.end(), prefix,
                               [](auto const &item, auto const &prefix) {
                                   return item.first.size() > prefix.size();
                               });
    mImpl->mPrefixRoutes.insert(
        it,
        {String(prefix),
         {handler, mode, split_string(upper_string(methods), ' ').collect()}});
}

void HTTPServer::route(HTTPHandler handler) {
    mImpl->mDefaultRoute = handler;
}

Task<std::unique_ptr<HTTPProtocol>>
HTTPServer::prepareHTTPS(SocketHandle handle, SSLServerState &https) const {
    using namespace std::string_view_literals;
    static char const *const protocols[] = {
        /* "h2", */
        "http/1.1",
    };
    auto sock = ssl_accept(std::move(handle), https.cert, https.skey, protocols,
                           &https.cache);
    sock.timeout(mImpl->mTimeout);
    /* if (auto peek = co_await sock.peekn(2); peek && *peek == "h2"sv) { */
    /*     co_return std::make_unique<HTTPProtocolVersion2>(std::move(sock)); */
    /* } */
    co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
}

Task<std::unique_ptr<HTTPProtocol>>
HTTPServer::prepareHTTP(SocketHandle handle) const {
    auto sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(mImpl->mTimeout);
    co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
}

Task<Expected<>> HTTPServer::handle_http(SocketHandle handle) const {
    // #if CO_ASYNC_ALLOC
    //     std::pmr::unsynchronized_pool_resource pool{currentAllocator};
    //     ReplaceAllocator _ = &pool;
    // #endif
    /* int h = handle.fileNo(); */
#if CO_ASYNC_DEBUG
    auto err =
        co_await doHandleConnection(co_await prepareHTTP(std::move(handle)));
    if (err.has_error()) [[unlikely]] {
        std::cerr << err.mErrorLocation.file_name() << ":"
                  << err.mErrorLocation.line() << ": "
                  << err.mErrorLocation.function_name() << ": "
                  << err.error().message() << '\n';
        co_return err;
    }
#else
    co_await co_await doHandleConnection(
        co_await prepareHTTP(std::move(handle)));
#endif
    co_return {};
}

Task<Expected<>>
HTTPServer::handle_http_redirect_to_https(SocketHandle handle) const {
    using namespace std::string_literals;
    auto http = co_await prepareHTTP(std::move(handle));
    while (true) {
        IO io(http.get());
        if (!co_await co_await io.readRequestHeader()) {
            break;
        }
        if (auto host = io.request.headers.get("host")) {
            auto location = "https://"_s + *host + io.request.uri.dump();
            HTTPResponse res = {
                .status = 302,
                .headers =
                    {
                        {"location"_s, location},
                        {"content-type"_s, "text/plain"_s},
                    },
            };
            co_await co_await io.response(res, location);
        } else {
            co_await co_await make_error_response(io, 403);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPServer::handle_https(SocketHandle handle,
                                          SSLServerState &https) const {
    /* int h = handle.fileNo(); */
    co_await co_await doHandleConnection(
        co_await prepareHTTPS(std::move(handle), https));
    /* co_await UringOp().prep_shutdown(h, SHUT_RDWR); */
    co_return {};
}

Task<Expected<>>
HTTPServer::doHandleConnection(std::unique_ptr<HTTPProtocol> http) const {
    while (true) {
        IO io(http.get());
        if (!co_await co_await io.readRequestHeader()) {
            break;
        }
#if CO_ASYNC_DEBUG
        std::chrono::steady_clock::time_point t0;
        if (mImpl->mLogRequests) {
            std::clog << io.request.method + ' ' + io.request.uri.dump() + '\n';
            for (auto [k, v]: io.request.headers) {
                if (k == "cookie" || k == "set-cookie" ||
                    k == "authorization") {
                    v = "*****";
                }
                std::clog << "      " + capitalizeHTTPHeader(k) + ": " + v +
                                 '\n';
            }
            t0 = std::chrono::steady_clock::now();
        }
#endif
        co_await co_await mImpl->doHandleRequest(io);
#if CO_ASYNC_DEBUG
        if (mImpl->mLogRequests) {
            auto dt = std::chrono::steady_clock::now() - t0;
            std::clog << io.request.method + ' ' + io.request.uri.dump() + ' ' +
                             to_string(io.mResponseSavedForDebug.status) + ' ' +
                             String(getHTTPStatusName(
                                 io.mResponseSavedForDebug.status)) +
                             ' ' +
                             to_string(std::chrono::duration_cast<
                                           std::chrono::milliseconds>(dt)
                                           .count()) +
                             "ms\n";
            for (auto [k, v]: io.mResponseSavedForDebug.headers) {
                if (k == "cookie" || k == "set-cookie" ||
                    k == "authorization") {
                    v = "*****";
                }
                std::clog << "      " + capitalizeHTTPHeader(k) + ": " + v +
                                 '\n';
            }
        }
#endif
    }
    co_return {};
}

Task<Expected<>> HTTPServer::make_error_response(IO &io, int status) {
    auto error = to_string(status) + ' ' + String(getHTTPStatusName(status));
    HTTPResponse res{
        .status = status,
        .headers =
            {
                {"content-type", "text/html;charset=utf-8"},
            },
    };
    co_await co_await io.response(
        res, "<html><head><title>" + error +
                 "</title></head><body><center><h1>" + error +
                 "</h1></center><hr><center>co_async</center></body></html>");
    co_return {};
}
} // namespace co_async

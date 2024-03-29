#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <co_async/task.hpp>
#include <co_async/http.hpp>
#include <co_async/stream.hpp>
#include <co_async/http_status_code.hpp>
#include <co_async/string_utils.hpp>
#include <co_async/simple_map.hpp>
#include <co_async/socket.hpp>
#include <co_async/uri.hpp>

namespace co_async {

struct HTTPServer {
    using HTTPHandler = Task<HTTPResponse> (*)(HTTPRequest const &);

    void addRoute(std::string_view path, HTTPHandler handler) {
        mRoutes.insert_or_assign(std::string(path), handler);
    }

    void defaultRoute(HTTPHandler handler) {
        mDefaultRoute = handler;
    }

    static Task<HTTPResponse> makeDefaultErrorResponse(int status) {
        auto error =
            to_string(status) + " " + std::string(getHTTPStatusName(status));
        co_return {
            .status = status,
            .headers =
                {
                    {"content-type", "text/html;charset=utf-8"},
                },
            .body = "<html><head><title>" + error +
                    "</title></head><body><h1>" + error + "</h1></body></html>",
        };
    }

    Task<HTTPResponse> handleRequest(HTTPRequest const &req) {
        if (auto handler = mRoutes.at(req.uri.path)) {
            co_return co_await (*handler)(req);
        }
        co_return co_await mDefaultRoute(req);
    }

    Task<> processConnection(FileStream stream) {
        HTTPRequest req;
        co_await req.read_from(stream);
        HTTPResponse res = co_await handleRequest(req);
        co_await res.write_into(stream);
        co_await fs_close(stream.release());
    }

    SimpleMap<std::string, HTTPHandler> mRoutes;
    HTTPHandler mDefaultRoute = +[](HTTPRequest const &) {
        return makeDefaultErrorResponse(404);
    };
};

} // namespace co_async

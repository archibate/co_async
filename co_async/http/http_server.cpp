export module co_async:http.http_server;

import std;
import :awaiter.task;
import :http.http11;
import :iostream.file_stream;
import :http.http_status_code;
import :utils.string_utils;
import :utils.simple_map;
import :system.socket;
import :system.fs;
import :http.uri;

namespace co_async {

export struct HTTPServer {
    using HTTPHandler = Task<HTTPResponse> (*)(HTTPRequest const &);

    void addRoute(std::string_view path, HTTPHandler handler) {
        mRoutes.insert_or_assign(std::string(path), handler);
    }

    void defaultRoute(HTTPHandler handler) {
        mDefaultRoute = handler;
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

private:
    SimpleMap<std::string, HTTPHandler> mRoutes;
    HTTPHandler mDefaultRoute = +[](HTTPRequest const &) {
        return makeDefaultErrorResponse(404);
    };

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
                    "</title></head><body><center><h1>" + error +
                    "</h1></center><hr><center>co_async</center></body></html>",
        };
    }
};

} // namespace co_async

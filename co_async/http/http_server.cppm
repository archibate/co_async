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

    void route(std::string_view path, HTTPHandler handler) {
        mRoutes.insert_or_assign(std::string(path), handler);
    }

    void default_route(HTTPHandler handler) {
        mDefaultRoute = handler;
    }

    Task<> process_connection(FileStream stream) {
        HTTPRequest req;
        co_await req.read_from(stream);
        HTTPResponse res = co_await handleRequest(req);
        co_await res.write_into(stream);
        co_await fs_close(stream.release());
    }

    static HTTPResponse make_error_response(int status) {
        auto error =
            to_string(status) + " " + std::string(getHTTPStatusName(status));
        return {
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

    static Task<HTTPResponse> make_ok_response(std::string body, std::string contentType = "text/html;charset=utf-8") {
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", std::move(contentType)},
                },
            .body = std::move(body),
        };
    }

private:
    SimpleMap<std::string, HTTPHandler> mRoutes;
    HTTPHandler mDefaultRoute = +[](HTTPRequest const &) -> Task<HTTPResponse> {
        co_return make_error_response(404);
    };

    Task<HTTPResponse> handleRequest(HTTPRequest const &req) {
        if (auto handler = mRoutes.at(req.uri.path)) {
            co_return co_await (*handler)(req);
        }
        co_return co_await mDefaultRoute(req);
    }
};

} // namespace co_async

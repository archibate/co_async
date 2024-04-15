#pragma once /*{export module co_async:http.http_server;}*/

#include <co_async/std.hpp>                   /*{import std;}*/
#include <co_async/awaiter/task.hpp>          /*{import :awaiter.task;}*/
#include <co_async/http/http11.hpp>           /*{import :http.http11;}*/
#include <co_async/iostream/socket_stream.hpp>/*{import :iostream.socket_stream;}*/
#include <co_async/iostream/directory_stream.hpp>/*{import :iostream.directory_stream;}*/
#include <co_async/http/http_status_code.hpp>/*{import :http.http_status_code;}*/
#include <co_async/utils/string_utils.hpp>   /*{import :utils.string_utils;}*/
#include <co_async/utils/simple_map.hpp>     /*{import :utils.simple_map;}*/
#include <co_async/system/socket.hpp>        /*{import :system.socket;}*/
#include <co_async/system/fs.hpp>            /*{import :system.fs;}*/
#include <co_async/system/timer.hpp>         /*{import :system.timer;}*/
#include <co_async/http/uri.hpp>             /*{import :http.uri;}*/

namespace co_async {

/*[export]*/ struct HTTPServer {
    using HTTPHandler = Task<HTTPResponse> (*)(HTTPRequest const &);
    using HTTPPrefixHandler = Task<HTTPResponse> (*)(HTTPRequest const &,
                                                     std::string_view);

    enum SuffixMode {
        SuffixRaw = 0, // "/a-9\\*g./.."
        SuffixName,    // "/a"
        SuffixPath,    // "/a/b/c"
    };

    void route(std::string_view methods, std::string_view path,
               HTTPHandler handler) {
        mRoutes.insert_or_assign(
            std::string(path),
            {handler, split_string(upper_string(methods), ' ').collect()});
    }

    void prefix_route(std::string_view methods, std::string_view prefix,
                      SuffixMode mode, HTTPPrefixHandler handler) {
        mPrefixRoutes.push_back(
            {std::string(prefix),
             {handler, mode,
              split_string(upper_string(methods), ' ').collect()}});
    }

    Task<> process_connection(SocketStream stream) const {
        HTTPRequest req;
        while (co_await req.read_from(stream)) {
            HTTPResponse res = co_await handleRequest(req);
            res.keepAlive = req.keepAlive;
            co_await res.write_into(stream);
            if (!res.keepAlive) {
                break;
            }
        }
        co_await socket_shutdown(stream.get());
        co_await fs_close(stream.release());
    }

    static std::string html_encode(std::string_view str) {
        std::string res;
        res.reserve(str.size());
        for (auto c: str) {
            switch (c) {
            case '&': res.append("&amp;"); break;
            case '"': res.append("&quot;"); break;
            case '\'': res.append("&apos;"); break;
            case '<': res.append("&lt;"); break;
            case '>': res.append("&gt;"); break;
            default: res.push_back(c);
            }
        }
        return res;
    }

    static Task<HTTPResponse> make_response_from_directory(DirFilePath path) {
        auto dirPath = path.path().generic_string();
        std::string dirBase;
        if (dirPath == ".") {
            dirPath = "";
        } else {
            dirBase = URI::url_encode_path(dirPath) + "/";
        }
        std::string content = "<h1>Files in /" + dirPath + ":</h1>";
        auto parentPath = path.path().parent_path().generic_string();
        content += "<a href=\"/" + URI::url_encode_path(parentPath) + "\">..</a><br>";
        DirectoryStream dir(co_await fs_open(path, OpenMode::Directory));
        while (auto entry = co_await dir.getdirent()) {
            if (entry == ".." || entry == ".") continue;
            content += "<a href=\"/" + dirBase + URI::url_encode_path(*entry) + "\">" +
                       html_encode(*entry) + "</a><br>";
        }
        co_return make_ok_response(content);
    }

    static Task<HTTPResponse> make_response_from_file_or_directory(DirFilePath path) {
        auto stat = co_await fs_stat(path, STATX_MODE);
        if (!stat) [[unlikely]] {
            co_return HTTPServer::make_error_response(404);
        }
        if (!stat->is_readable()) [[unlikely]] {
            co_return HTTPServer::make_error_response(403);
        }
        if (stat->is_directory()) {
            co_return co_await make_response_from_directory(std::move(path));
        }
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", guessContentTypeByExtension(
                                         path.path().extension().string())},
                },
            .body = path,
        };
    }

    static Task<HTTPResponse> make_response_from_file(DirFilePath path) {
        auto stat = co_await fs_stat(path, STATX_MODE);
        if (!stat || stat->is_directory()) [[unlikely]] {
            co_return HTTPServer::make_error_response(404);
        }
        if (!stat->is_readable()) [[unlikely]] {
            co_return HTTPServer::make_error_response(403);
        }
        co_return {
            .status = 200,
            .headers =
                {
                    {"content-type", guessContentTypeByExtension(
                                         path.path().extension().string())},
                },
            .body = path,
        };
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

    static HTTPResponse
    make_ok_response(std::string body,
                     std::string contentType = "text/html;charset=utf-8") {
        return {
            .status = 200,
            .headers =
                {
                    {"content-type", std::move(contentType)},
                },
            .body = std::move(body),
        };
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
        SuffixMode mSuffixMode;
        std::vector<std::string> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }

        bool checkSuffix(std::string_view suffix) const {
            switch (mSuffixMode) {
            case SuffixName: {
                // make sure no '/' in suffix
                if (suffix.find('/') != std::string_view::npos) [[unlikely]] {
                    return false;
                }
                return true;
            }
            case SuffixPath: {
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

    Task<HTTPResponse> handleRequest(HTTPRequest const &req) const {
        if (auto route = mRoutes.at(req.uri.path)) {
            if (!route->checkMethod(req.method)) [[unlikely]] {
                co_return make_error_response(405);
            }
            co_return co_await route->mHandler(req);
        }
        for (auto const &[prefix, route]: mPrefixRoutes) {
            if (req.uri.path.starts_with(prefix)) {
                if (!route.checkMethod(req.method)) [[unlikely]] {
                    co_return make_error_response(405);
                }
                auto suffix =
                    std::string_view(req.uri.path).substr(prefix.size());
                if (!route.checkSuffix(suffix)) [[unlikely]] {
                    co_return make_error_response(400);
                }
                co_return co_await route.mHandler(req, suffix);
            }
        }
        co_return make_error_response(404);
    }
};

} // namespace co_async

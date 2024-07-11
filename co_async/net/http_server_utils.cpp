#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/directory_stream.hpp>
#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/net/http_protocol.hpp>
#include <co_async/net/http_server.hpp>
#include <co_async/net/http_server_utils.hpp>
#include <co_async/net/uri.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/pipe.hpp>
#include <co_async/platform/socket.hpp>

namespace co_async {
String HTTPServerUtils::html_encode(std::string_view str) {
    String res;
    res.reserve(str.size());
    for (auto c: str) {
        switch (c) {
        case '&':  res.append("&amp;"); break;
        case '"':  res.append("&quot;"); break;
        case '\'': res.append("&apos;"); break;
        case '<':  res.append("&lt;"); break;
        case '>':  res.append("&gt;"); break;
        default:   res.push_back(c);
        }
    }
    return res;
}

Task<Expected<>> HTTPServerUtils::make_ok_response(HTTPServer::IO &io,
                                                   std::string_view body,
                                                   String contentType) {
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type", std::move(contentType)},
            },
    };
    co_await co_await io.response(res, body);
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_directory(HTTPServer::IO &io,
                                              std::filesystem::path path) {
    String dirPath{path.generic_string()};
    String content = "<h1>Files in " + dirPath + ":</h1>";
    auto parentPath = path.parent_path().generic_string();
    content +=
        "<a href=\"/" + URI::url_encode_path(parentPath) + "\">..</a><br>";
    auto dir = co_await co_await dir_open(path);
    while (auto entry = co_await dir.next()) {
        if (*entry == ".." || *entry == ".") {
            continue;
        }
        content +=
            "<a href=\"/" +
            URI::url_encode_path(make_path(dirPath, *entry).generic_string()) +
            "\">" + html_encode(*entry) + "</a><br>";
    }
    co_await co_await make_ok_response(io, content);
    co_return {};
}

Task<Expected<>> HTTPServerUtils::make_error_response(HTTPServer::IO &io,
                                                      int status) {
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

Task<Expected<>> HTTPServerUtils::make_response_from_file_or_directory(
    HTTPServer::IO &io, std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    if (stat->is_directory()) {
        co_return co_await make_response_from_directory(io, std::move(path));
    }
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_path(HTTPServer::IO &io,
                                         std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    if (stat->is_directory()) {
        co_return co_await make_response_from_directory(io, path);
    }
    /* if (stat->is_executable()) { */
    /*     co_return co_await make_response_from_cgi_script(io, path); */
    /* } */
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_file(HTTPServer::IO &io,
                                         std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat || stat->is_directory()) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}
} // namespace co_async

#pragma once /*{export module co_async:http.http_server_utils;}*/

#include <co_async/std.hpp>                   /*{import std;}*/
#include <co_async/awaiter/task.hpp>          /*{import :awaiter.task;}*/
#include <co_async/http/http11.hpp>           /*{import :http.http11;}*/
#include <co_async/http/http_server.hpp>      /*{import :http.http_server;}*/
#include <co_async/iostream/socket_stream.hpp>/*{import :iostream.socket_stream;}*/
#include <co_async/iostream/directory_stream.hpp>/*{import :iostream.directory_stream;}*/
#include <co_async/system/socket.hpp>            /*{import :system.socket;}*/
#include <co_async/system/fs.hpp>                /*{import :system.fs;}*/
#include <co_async/http/uri.hpp>                 /*{import :http.uri;}*/
#include <co_async/system/process.hpp>           /*{import :system.process;}*/

namespace co_async {

/*[export]*/ struct HTTPServerUtils {
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
        content +=
            "<a href=\"/" + URI::url_encode_path(parentPath) + "\">..</a><br>";
        DirectoryStream dir(co_await fs_open(path, OpenMode::Directory));
        while (auto entry = co_await dir.getdirent()) {
            if (entry == ".." || entry == ".")
                continue;
            content += "<a href=\"/" + dirBase + URI::url_encode_path(*entry) +
                       "\">" + html_encode(*entry) + "</a><br>";
        }
        co_return make_ok_response(content);
    }

    static Task<HTTPResponse>
    make_response_from_file_or_directory(DirFilePath path) {
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

    static Task<HTTPResponse>
    make_response_from_cgi_script(HTTPRequest const &req,
                                  std::filesystem::path path) {
        auto stat = co_await fs_stat(path, STATX_MODE);
        if (!stat || stat->is_directory()) [[unlikely]] {
            co_return HTTPServer::make_error_response(404);
        }
        if (!stat->is_executable()) [[unlikely]] {
            co_return HTTPServer::make_error_response(403);
        }
        HTTPHeaders headers;
        std::string content;
        auto proc = ProcessBuilder();
        proc.path(path.string(), true);
        proc.inherit_env();
        proc.env("HTTP_PATH", req.uri.path);
        proc.env("HTTP_METHOD", req.method);
        for (auto const &[k, v]: req.uri.params) {
            for (char c: k) {
                if (!('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_'))
                    [[unlikely]] {
                    goto skip;
                }
            }
            proc.env("HTTP_GET_" + k, v);
        skip:;
        }
        auto pipe = co_await make_pipe();
        proc.close(0);
        proc.open(1, pipe.writer());
        proc.open(2, 2);
        Pid pid = co_await proc.spawn();
        FileIStream reader(pipe.reader());
        std::string line;
        while (true) {
            line.clear();
            if (!co_await reader.getline(line, '\n')) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "unexpected eof in cgi header\n";
#endif
                co_return HTTPServer::make_error_response(500);
            }
            if (line.empty()) {
                break;
            }
            auto pos = line.find(':');
            if (pos == std::string::npos) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "invalid k-v pair in cgi header\n";
#endif
                co_return HTTPServer::make_error_response(500);
            }
            headers.insert_or_assign(
                trim_string(lower_string(line.substr(0, pos))),
                trim_string(line.substr(pos + 1)));
        }
        co_await reader.getall(content);
        auto res = co_await wait_process(pid);
        if (res.status != 0) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "cgi script exit failure\n";
#endif
            co_return HTTPServer::make_error_response(500);
        }
        co_return {
            .status = 200,
            .headers = std::move(headers),
            .body = std::move(content),
        };
    }
};

} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/net/http_server.hpp>

namespace co_async {
struct HTTPServerUtils {
    static String html_encode(std::string_view str);
    static Task<Expected<>>
    make_ok_response(HTTPServer::IO &io, std::string_view body,
                     String contentType = "text/html;charset=utf-8");
    static Task<Expected<>>
    make_response_from_directory(HTTPServer::IO &io,
                                 std::filesystem::path path);
    static Task<Expected<>> make_error_response(HTTPServer::IO &io, int status);
    static Task<Expected<>>
    make_response_from_file_or_directory(HTTPServer::IO &io,
                                         std::filesystem::path path);
    static Task<Expected<>> make_response_from_path(HTTPServer::IO &io,
                                                    std::filesystem::path path);
    static Task<Expected<>> make_response_from_file(HTTPServer::IO &io,
                                                    std::filesystem::path path);
};
} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
std::string timePointToHTTPDate(std::chrono::system_clock::time_point tp);
Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(std::string const &date);
std::string httpDateNow();
std::string_view getHTTPStatusName(int status);
std::string guessContentTypeByExtension(
    std::string_view ext, char const *defaultType = "text/plain;charset=utf-8");
std::string capitalizeHTTPHeader(std::string_view key);
} // namespace co_async

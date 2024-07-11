#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/allocator.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
String timePointToHTTPDate(std::chrono::system_clock::time_point tp);
Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(String const &date);
String httpDateNow();
std::string_view getHTTPStatusName(int status);
String guessContentTypeByExtension(
    std::string_view ext, char const *defaultType = "text/plain;charset=utf-8");
String capitalizeHTTPHeader(std::string_view key);
} // namespace co_async

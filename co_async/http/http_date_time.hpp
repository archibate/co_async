#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {

// format chrono time point into HTTP date format, e.g.: Tue, 30 Apr 2024 07:31:38 GMT
inline std::string
timePointToHTTPDate(std::chrono::system_clock::time_point tp) {
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

inline Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(std::string const &date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss.imbue(std::locale::classic());
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) [[unlikely]] {
        return Unexpected{std::make_error_code(std::errc::invalid_argument)};
    }
    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

inline std::string httpDateNow() {
    std::time_t time = std::time(nullptr);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

} // namespace co_async

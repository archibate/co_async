#include <co_async/generic/allocator.hpp>
#include <co_async/net/http_string_utils.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
String timePointToHTTPDate(std::chrono::system_clock::time_point tp) {
    // format chrono time point into HTTP date format, e.g.:
    // Tue, 30 Apr 2024 07:31:38 GMT
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return String{ss.str()};
}

Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(String const &date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss.imbue(std::locale::classic());
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) [[unlikely]] {
        return std::errc::invalid_argument;
    }
    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

String httpDateNow() {
    std::time_t time = std::time(nullptr);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return String{ss.str()};
}

std::string_view getHTTPStatusName(int status) {
    using namespace std::string_view_literals;
    static constexpr std::pair<int, std::string_view> lut[] = {
        {100, "Continue"sv},
        {101, "Switching Protocols"sv},
        {102, "Processing"sv},
        {200, "OK"sv},
        {201, "Created"sv},
        {202, "Accepted"sv},
        {203, "Non-Authoritative Information"sv},
        {204, "No Content"sv},
        {205, "Reset Content"sv},
        {206, "Partial Content"sv},
        {207, "Multi-Status"sv},
        {208, "Already Reported"sv},
        {226, "IM Used"sv},
        {300, "Multiple Choices"sv},
        {301, "Moved Permanently"sv},
        {302, "Found"sv},
        {303, "See Other"sv},
        {304, "Not Modified"sv},
        {305, "Use Proxy"sv},
        {306, "Switch Proxy"sv},
        {307, "Temporary Redirect"sv},
        {308, "Permanent Redirect"sv},
        {400, "Bad Request"sv},
        {401, "Unauthorized"sv},
        {402, "Payment Required"sv},
        {403, "Forbidden"sv},
        {404, "Not Found"sv},
        {405, "Method Not Allowed"sv},
        {406, "Not Acceptable"sv},
        {407, "Proxy Authentication Required"sv},
        {408, "Request Timeout"sv},
        {409, "Conflict"sv},
        {410, "Gone"sv},
        {411, "Length Required"sv},
        {412, "Precondition Failed"sv},
        {413, "Payload Too Large"sv},
        {414, "URI Too Long"sv},
        {415, "Unsupported Media Type"sv},
        {416, "Range Not Satisfiable"sv},
        {417, "Expectation Failed"sv},
        {418, "I'm a teapot"sv},
        {421, "Misdirected Request"sv},
        {422, "Unprocessable Entity"sv},
        {423, "Locked"sv},
        {424, "Failed Dependency"sv},
        {426, "Upgrade Required"sv},
        {428, "Precondition Required"sv},
        {429, "Too Many Requests"sv},
        {431, "Request Header Fields Too Large"sv},
        {451, "Unavailable For Legal Reasons"sv},
        {500, "Internal Server Error"sv},
        {501, "Not Implemented"sv},
        {502, "Bad Gateway"sv},
        {503, "Service Unavailable"sv},
        {504, "Gateway Timeout"sv},
        {505, "HTTP Version Not Supported"sv},
        {506, "Variant Also Negotiates"sv},
        {507, "Insufficient Storage"sv},
        {508, "Loop Detected"sv},
        {510, "Not Extended"sv},
        {511, "Network Authentication Required"sv},
    };
    if (status == 200) {
        return "OK"sv;
    }
    auto it = std::lower_bound(
        std::begin(lut), std::end(lut), status,
        [](auto const &p, auto status) { return p.first < status; });
    if (it == std::end(lut) || it->first != status) [[unlikely]] {
        return "Unknown"sv;
    } else {
        return it->second;
    }
}

String guessContentTypeByExtension(std::string_view ext,
                                   char const *defaultType) {
    using namespace std::string_view_literals;
    if (ext == ".html"sv || ext == ".htm"sv) {
        return String{"text/html;charset=utf-8"sv};
    } else if (ext == ".css"sv) {
        return String{"text/css;charset=utf-8"sv};
    } else if (ext == ".js"sv) {
        return String{"application/javascript;charset=utf-8"sv};
    } else if (ext == ".txt"sv || ext == ".md"sv) {
        return String{"text/plain;charset=utf-8"sv};
    } else if (ext == ".json"sv) {
        return String{"application/json"sv};
    } else if (ext == ".png"sv) {
        return String{"image/png"sv};
    } else if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return String{"image/jpeg"sv};
    } else if (ext == ".gif"sv) {
        return String{"image/gif"sv};
    } else if (ext == ".xml"sv) {
        return String{"application/xml"sv};
    } else if (ext == ".pdf"sv) {
        return String{"application/pdf"sv};
    } else if (ext == ".mp4"sv) {
        return String{"video/mp4"sv};
    } else if (ext == ".mp3"sv) {
        return String{"audio/mp3"sv};
    } else if (ext == ".zip"sv) {
        return String{"application/zip"sv};
    } else if (ext == ".svg"sv) {
        return String{"image/svg+xml"sv};
    } else if (ext == ".wav"sv) {
        return String{"audio/wav"sv};
    } else if (ext == ".ogg"sv) {
        return String{"audio/ogg"sv};
    } else if (ext == ".mpg"sv || ext == ".mpeg"sv) {
        return String{"video/mpeg"sv};
    } else if (ext == ".webm"sv) {
        return String{"video/webm"sv};
    } else if (ext == ".ico"sv) {
        return String{"image/x-icon"sv};
    } else if (ext == ".rar"sv) {
        return String{"application/x-rar-compressed"sv};
    } else if (ext == ".7z"sv) {
        return String{"application/x-7z-compressed"sv};
    } else if (ext == ".tar"sv) {
        return String{"application/x-tar"sv};
    } else if (ext == ".gz"sv) {
        return String{"application/gzip"sv};
    } else if (ext == ".bz2"sv) {
        return String{"application/x-bzip2"sv};
    } else if (ext == ".xz"sv) {
        return String{"application/x-xz"sv};
    } else if (ext == ".zip"sv) {
        return String{"application/zip"sv};
    } else if (ext == ".tar.gz"sv || ext == ".tgz"sv) {
        return String{"application/tar+gzip"sv};
    } else if (ext == ".tar.bz2"sv || ext == ".tbz2"sv) {
        return String{"application/tar+bzip2"sv};
    } else if (ext == ".tar.xz"sv || ext == ".txz"sv) {
        return String{"application/tar+xz"sv};
    } else if (ext == ".doc"sv || ext == ".docx"sv) {
        return String{"application/msword"sv};
    } else if (ext == ".xls"sv || ext == ".xlsx"sv) {
        return String{"application/vnd.ms-excel"sv};
    } else if (ext == ".ppt"sv || ext == ".pptx"sv) {
        return String{"application/vnd.ms-powerpoint"sv};
    } else if (ext == ".csv"sv) {
        return String{"text/csv;charset=utf-8"sv};
    } else if (ext == ".rtf"sv) {
        return String{"application/rtf"sv};
    } else if (ext == ".exe"sv) {
        return String{"application/x-msdownload"sv};
    } else if (ext == ".msi"sv) {
        return String{"application/x-msi"sv};
    } else if (ext == ".bin"sv) {
        return String{"application/octet-stream"sv};
    } else {
        return String{defaultType};
    }
}

String capitalizeHTTPHeader(std::string_view key) {
    // e.g.: user-agent -> User-Agent
    String result(key);
    if (!result.empty()) [[likely]] {
        if ('a' <= result[0] && result[0] <= 'z') [[likely]] {
            result[0] -= 'a' - 'A';
        }
        for (std::size_t i = 1; i < result.size(); ++i) {
            if (result[i - 1] == '-' && 'a' <= result[i] && result[i] <= 'z')
                [[likely]] {
                result[i] -= 'a' - 'A';
            }
        }
    }
    return result;
}
} // namespace co_async

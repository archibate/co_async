export module co_async:http.http_status_code;

import std;
import :awaiter.task;
import :iostream.file_stream;

namespace co_async {

inline std::string_view getHTTPStatusName(int status) {
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

} // namespace co_async

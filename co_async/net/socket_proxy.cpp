#include <co_async/generic/io_context.hpp>
#include <co_async/net/socket_proxy.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
Task<Expected<SocketHandle>>
socket_proxy_connect(char const *host, int port, std::string_view proxy,
                     std::chrono::steady_clock::duration timeout) {
    using namespace std::string_view_literals;
    if (proxy.empty()) {
        co_return co_await socket_connect(
            co_await AddressResolver().host(host).port(port).resolve_one());
    } else {
#if CO_ASYNC_DEBUG
        if (!proxy.starts_with("http://")) {
            std::cerr << "WARNING: both http_proxy and https_proxy variable "
                         "should starts with http://\n";
        }
#endif
        auto sock = co_await co_await socket_connect(
            co_await AddressResolver().host(proxy).resolve_one(),
            co_await co_cancel);
        String hostName(host);
        hostName += ':';
        hostName += to_string(port);
        String header("CONNECT "sv);
        header += hostName;
        header += " HTTP/1.1\r\nHost: "sv;
        header += hostName;
        header += "\r\nProxy-Connection: Keep-Alive\r\n\r\n"sv;
        std::span<char const> buf = header;
        std::size_t n = 0;
        do {
            n = co_await co_await socket_write(sock, buf, timeout);
            if (!n) [[unlikely]] {
                co_return std::errc::connection_reset;
            }
            buf = buf.subspan(n);
        } while (buf.size() > 0);
        using namespace std::string_view_literals;
        auto desiredResponse = "HTTP/1.1 200 Connection established\r\n\r\n"sv;
        String response(39, '\0');
        std::span<char> outbuf = response;
        do {
            n = co_await co_await socket_read(sock, outbuf, timeout);
            if (!n) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: proxy server failed to establish "
                             "connection: [" +
                                 response.substr(0, response.size() -
                                                        outbuf.size()) +
                                 "]\n";
#endif
                co_return std::errc::connection_reset;
            }
            outbuf = outbuf.subspan(n);
        } while (outbuf.size() > 0);
        if (std::string_view(response).substr(8) != desiredResponse.substr(8))
            [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: proxy server seems failed to establish "
                         "connection: [" +
                             response + "]\n";
#endif
            co_return std::errc::connection_reset;
        }
        co_return sock;
    }
}
} // namespace co_async

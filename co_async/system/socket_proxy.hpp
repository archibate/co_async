#pragma once

#include <co_async/std.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {

inline Task<Expected<SocketHandle, std::errc>>
socket_proxy_connect(char const *host, int port, std::string_view proxy,
                     std::chrono::nanoseconds timeout) {
    if (proxy.starts_with("http://")) {
        proxy.remove_prefix(7);
        std::string proxyHost;
        int proxyPort = 80;
        if (auto i = proxy.rfind(':'); i != std::string_view::npos) {
            proxyHost = proxy.substr(0, i);
            if (auto portOpt = from_string<int>(proxy.substr(i + 1)))
                [[likely]] {
                proxyPort = *portOpt;
            }
        } else {
            proxyHost.assign(proxy);
        }
        auto sock = co_await co_await socket_connect({proxyHost.c_str(), proxyPort});
        auto hostName = std::string(host) + ":" + to_string(port);
        std::string header = "CONNECT " + hostName +
                             " HTTP/1.1\r\nHost: " + hostName +
                             "\r\nProxy-Connection: Keep-Alive\r\n\r\n";
        std::span<char const> buf = header;
        std::size_t n = 0;
        do {
            n = co_await co_await socket_write(sock, buf, timeout);
            if (!n) [[unlikely]] {
                co_return Unexpected{std::errc::connection_reset};
            }
            buf = buf.subspan(n);
        } while (buf.size() > 0);
        using namespace std::string_view_literals;
        auto desiredResponse = "HTTP/1.1 200 Connection established\r\n\r\n"sv;
        std::string response(39, '\0');
        std::span<char> outbuf = response;
        do {
            n = co_await co_await socket_read(sock, outbuf, timeout);
            if (!n) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: proxy server failed to establish connection: [" + response.substr(0, response.size() - outbuf.size()) + "]";
#endif
                co_return Unexpected{std::errc::connection_reset};
            }
            outbuf = outbuf.subspan(n);
        } while (outbuf.size() > 0);
        if (std::string_view(response).substr(8) != desiredResponse.substr(8))
            [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: proxy server seems failed to establish connection: [" + response + "]";
#endif
                co_return Unexpected{std::errc::connection_reset};
        }
        co_return sock;
    } else {
        co_return co_await socket_connect({host, port});
    }
}

} // namespace co_async

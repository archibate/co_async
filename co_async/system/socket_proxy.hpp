#pragma once /*{export module co_async:system.socket_proxy;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/fs.hpp>            /*{import :system.fs;}*/
#include <co_async/system/socket.hpp>   /*{import :system.system_loop;}*/
#include <co_async/utils/string_utils.hpp>   /*{import :utils.string_utils;}*/
#include <co_async/awaiter/task.hpp>         /*{import :awaiter.task;}*/

namespace co_async {

/*[export]*/ inline Task<SocketHandle> socket_proxy_connect(const char *host, int port, std::string_view proxy, std::chrono::nanoseconds timeout) {
    if (proxy.starts_with("http://")) {
        proxy.remove_prefix(7);
        std::string proxyHost;
        int proxyPort = 80;
        if (auto i = proxy.rfind(':'); i != std::string_view::npos) {
            proxyHost = proxy.substr(0, i);
            if (auto portOpt = from_string<int>(proxy.substr(i + 1))) [[likely]] {
                proxyPort = *portOpt;
            }
        } else {
            proxyHost.assign(proxy);
        }
        auto sock = co_await socket_connect({proxyHost.c_str(), proxyPort});
        auto hostName = std::string(host) + ":" + to_string(port);
        std::string header = "CONNECT " + hostName + " HTTP/1.1\r\nHost: " + hostName + "\r\nProxy-Connection: Keep-Alive\r\n\r\n";
        std::span<char const> buf = header;
        std::size_t n = 0;
        do {
            n = co_await socket_write(sock, buf, timeout);
            if (!n) [[unlikely]] {
                throw std::runtime_error("proxy server failed to establish connection");
            }
            buf = buf.subspan(n);
        } while (buf.size() > 0);
        using namespace std::string_view_literals;
        auto desiredResponse = "HTTP/1.1 200 Connection established\r\n\r\n"sv;
        std::string response(39, '\0');
        std::span<char> outbuf = response;
        do {
            n = co_await socket_read(sock, outbuf, timeout);
            if (!n) [[unlikely]] {
                throw std::runtime_error("proxy server failed to establish connection: [" + response.substr(response.size() - outbuf.size()) + "]");
            }
            outbuf = outbuf.subspan(n);
        } while (outbuf.size() > 0);
        if (std::string_view(response).substr(8) != desiredResponse.substr(8)) [[unlikely]] {
            throw std::runtime_error("proxy server failed to establish connection: [" + response + "]");
        }
        co_return sock;
    } else {
        co_return co_await socket_connect({host, port});
    }
}

}

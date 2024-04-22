#pragma once/*{export module co_async:iostream.socket_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/socket.hpp>/*{import :system.socket;}*/
#include <co_async/system/socket_proxy.hpp>/*{import :system.socket_proxy;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

struct SocketStreamRaw : virtual IOStreamRaw {
    Task<std::size_t> raw_read(std::span<char> buffer) override {
        return socket_read(mFile, buffer, mTimeout);
    }

    Task<std::size_t> raw_write(std::span<char const> buffer) override {
        return socket_write(mFile, buffer, mTimeout);
    }

    SocketHandle release() noexcept {
        return std::move(mFile);
    }

    SocketHandle &get() noexcept {
        return mFile;
    }

    explicit SocketStreamRaw(SocketHandle file) : mFile(std::move(file)) {}

    std::chrono::nanoseconds timeout() const {
        return mTimeout;
    }

    void timeout(std::chrono::nanoseconds timeout) {
        mTimeout = timeout;
    }

private:
    SocketHandle mFile;
    std::chrono::nanoseconds mTimeout = std::chrono::seconds(5);
};

/*[export]*/ struct SocketStream : IOStreamImpl<SocketStreamRaw> {
    using IOStreamImpl<SocketStreamRaw>::IOStreamImpl;

    static Task<SocketStream> connect(const char *host, int port, std::string_view proxy, std::chrono::nanoseconds timeout) {
        auto conn = co_await socket_proxy_connect(host, port, proxy, timeout);
        SocketStream sock(std::move(conn));
        co_return sock;
    }

    static Task<SocketStream> accept(SocketListener &listener) {
        auto conn = co_await listener_accept(listener);
        SocketStream sock(std::move(conn));
        co_return sock;
    }
};


} // namespace co_async

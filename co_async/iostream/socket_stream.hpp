#pragma once

#include <co_async/std.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/system/socket_proxy.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct SocketStream : Stream {
    Task<Expected<std::size_t, std::errc>>
    raw_read(std::span<char> buffer) override {
        co_return co_await socket_read(mFile, buffer, mTimeout);
    }

    Task<Expected<std::size_t, std::errc>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await socket_write(mFile, buffer, mTimeout);
    }

    SocketHandle release() noexcept {
        return std::move(mFile);
    }

    SocketHandle &get() noexcept {
        return mFile;
    }

    explicit SocketStream(SocketHandle file) : mFile(std::move(file)) {}

    void raw_timeout(std::chrono::nanoseconds timeout) override {
        mTimeout = timeout;
    }

private:
    SocketHandle mFile;
    std::chrono::nanoseconds mTimeout = std::chrono::seconds(10);
};

inline Task<Expected<OwningStream, std::errc>>
tcp_connect(char const *host, int port, std::string_view proxy,
            std::chrono::nanoseconds timeout) {
    auto handle =
        co_await co_await socket_proxy_connect(host, port, proxy, timeout);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(timeout);
    co_return sock;
}

inline Task<Expected<OwningStream, std::errc>>
tcp_accept(SocketListener &listener, std::chrono::nanoseconds timeout) {
    auto handle = co_await co_await listener_accept(listener);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(timeout);
    co_return sock;
}

} // namespace co_async

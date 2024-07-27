#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/net/socket_proxy.hpp>
#include <co_async/platform/socket.hpp>

namespace co_async {
struct SocketStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        auto ret =
            co_await socket_read(mFile, buffer, mTimeout, co_await co_cancel);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return std::errc::stream_timeout;
        }
        co_return ret;
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        auto ret =
            co_await socket_write(mFile, buffer, mTimeout, co_await co_cancel);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return std::errc::stream_timeout;
        }
        co_return ret;
    }

    SocketHandle release() noexcept {
        return std::move(mFile);
    }

    SocketHandle &get() noexcept {
        return mFile;
    }

    explicit SocketStream(SocketHandle file) : mFile(std::move(file)) {}

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        mTimeout = timeout;
    }

private:
    SocketHandle mFile;
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(30);
};

inline Task<Expected<OwningStream>>
tcp_connect(char const *host, int port, std::string_view proxy,
            std::chrono::steady_clock::duration timeout) {
    auto handle =
        co_await co_await socket_proxy_connect(host, port, proxy, timeout);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(timeout);
    co_return sock;
}

inline Task<Expected<OwningStream>> tcp_accept(SocketListener &listener) {
    auto handle = co_await co_await listener_accept(listener);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    co_return sock;
}
} // namespace co_async

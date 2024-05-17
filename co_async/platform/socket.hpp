#pragma once
#include <co_async/std.hpp>
#include <arpa/inet.h>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/finally.hpp>
#include <co_async/utils/string_utils.hpp>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace co_async {
std::error_category const &getAddrInfoCategory();

struct IpAddress {
    explicit IpAddress(struct in_addr const &addr) noexcept : mAddr(addr) {}

    explicit IpAddress(struct in6_addr const &addr6) noexcept : mAddr(addr6) {}

    static Expected<IpAddress> parse(std::string_view host,
                                     bool allowIpv6 = true);
    static Expected<IpAddress> parse(char const *host, bool allowIpv6 = true);
    std::string toString() const;

    auto repr() const {
        return toString();
    }

    std::variant<struct in_addr, struct in6_addr> mAddr;
};

struct SocketAddress {
    SocketAddress() = default;

    static Expected<SocketAddress> parse(std::string_view host,
                                         int defaultPort = -1);
    explicit SocketAddress(IpAddress ip, int port);

    union {
        struct sockaddr_in mAddrIpv4;
        struct sockaddr_in6 mAddrIpv6;
        struct sockaddr mAddr;
    };

    socklen_t mAddrLen;

    sa_family_t family() const noexcept;

    IpAddress host() const;

    int port() const;

    std::string toString() const;

    auto repr() const {
        return toString();
    }

private:
    void initFromHostPort(struct in_addr const &host, int port);
    void initFromHostPort(struct in6_addr const &host, int port);
};

struct [[nodiscard]] SocketHandle : FileHandle {
    using FileHandle::FileHandle;
};

struct [[nodiscard]] SocketListener : SocketHandle {
    using SocketHandle::SocketHandle;
};

SocketAddress get_socket_address(SocketHandle &sock);
SocketAddress get_socket_peer_address(SocketHandle &sock);

template <class T>
Expected<T> socketGetOption(SocketHandle &sock, int level, int optId) {
    T val;
    socklen_t len = sizeof(val);
    if (auto e =
            expectError(getsockopt(sock.fileNo(), level, optId, &val, &len))) {
        return Unexpected{e.error()};
    }
    return val;
}

template <class T>
Expected<> socketSetOption(SocketHandle &sock, int level, int opt,
                           T const &optVal) {
    return expectError(
        setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

Task<Expected<SocketHandle>> createSocket(int family, int type);
Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr);
Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout);

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel);
Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog = SOMAXCONN);
Task<Expected<SocketListener>>
listener_bind(std::pair<std::string, int> const &addr, int backlog = SOMAXCONN);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel);
Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf, CancelToken cancel);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout);
Task<Expected<>> socket_shutdown(SocketHandle &sock, int how = SHUT_RDWR);
} // namespace co_async

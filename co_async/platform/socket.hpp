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

// struct IpAddress {
//     explicit IpAddress(struct in_addr const &addr) noexcept : mAddr(addr) {}
//
//     explicit IpAddress(struct in6_addr const &addr6) noexcept : mAddr(addr6)
//     {}
//
//     static Expected<IpAddress> fromString(char const *host);
//
//     String toString() const;
//
//     auto repr() const {
//         return toString();
//     }
//
//     std::variant<struct in_addr, struct in6_addr> mAddr;
// };

struct SocketAddress {
    SocketAddress() = default;

    explicit SocketAddress(struct sockaddr const *addr, socklen_t addrLen,
                           sa_family_t family, int sockType, int protocol);

    struct sockaddr_storage mAddr;
    socklen_t mAddrLen;
    int mSockType;
    int mProtocol;

    sa_family_t family() const noexcept {
        return mAddr.ss_family;
    }

    int socktype() const noexcept {
        return mSockType;
    }

    int protocol() const noexcept {
        return mProtocol;
    }

    std::string host() const;

    int port() const;

    void trySetPort(int port);

    String toString() const;

    auto repr() const {
        return toString();
    }

private:
    void initFromHostPort(struct in_addr const &host, int port);
    void initFromHostPort(struct in6_addr const &host, int port);
};

struct AddressResolver {
private:
    std::string m_host;
    int m_port = -1;
    std::string m_service;
    struct addrinfo m_hints = {};

public:
    AddressResolver &host(std::string_view host) {
        if (auto i = host.find("://"); i != host.npos) {
            if (auto service = host.substr(0, i); !service.empty()) {
                m_service = service;
            }
            host.remove_prefix(i + 3);
        }
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1)))
                [[likely]] {
                m_port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        m_host = host;
        return *this;
    }

    AddressResolver &port(int port) {
        m_port = port;
        return *this;
    }

    AddressResolver &service(std::string_view service) {
        m_service = service;
        return *this;
    }

    AddressResolver &family(int family) {
        m_hints.ai_family = family;
        return *this;
    }

    AddressResolver &socktype(int socktype) {
        m_hints.ai_socktype = socktype;
        return *this;
    }

    struct ResolveResult {
        std::vector<SocketAddress> addrs;
        std::string service;
    };

    Expected<ResolveResult> resolve_all();
    Expected<SocketAddress> resolve_one();
    Expected<SocketAddress> resolve_one(std::string &service);
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
        return e.error();
    }
    return val;
}

template <class T>
Expected<> socketSetOption(SocketHandle &sock, int level, int opt,
                           T const &optVal) {
    return expectError(
        setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

Task<Expected<SocketHandle>> createSocket(int family, int type, int protocol);
Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr);
Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout);

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel);
Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog = SOMAXCONN);
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
Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf, CancelToken cancel);
Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf,
                                            CancelToken cancel);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout, CancelToken cancel);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout, CancelToken cancel);
Task<Expected<>> socket_shutdown(SocketHandle &sock, int how = SHUT_RDWR);
} // namespace co_async

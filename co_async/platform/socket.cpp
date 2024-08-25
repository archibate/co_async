#include <arpa/inet.h>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/platform/socket.hpp>
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
std::error_category const &getAddrInfoCategory() {
    static struct : std::error_category {
        char const *name() const noexcept override {
            return "getaddrinfo";
        }

        std::string message(int e) const override {
            return gai_strerror(e);
        }
    } instance;

    return instance;
}

// Expected<IpAddress> IpAddress::fromString(char const *host) {
//     struct in_addr addr = {};
//     struct in6_addr addr6 = {};
//     if (1 == inet_pton(AF_INET, host, &addr)) {
//         return IpAddress(addr);
//     }
//     if (1 == inet_pton(AF_INET6, host, &addr6)) {
//         return IpAddress(addr6);
//     }
//     // gethostbyname is deprecated, let's use getaddrinfo instead:
//     struct addrinfo hints = {};
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     struct addrinfo *result;
//     int err = getaddrinfo(host, nullptr, &hints, &result);
//     if (err) [[unlikely]] {
// #if CO_ASYNC_DEBUG
//         std::cerr << host << ": " << gai_strerror(err) << '\n';
// #endif
//         return std::error_code(err, getAddrInfoCategory());
//     }
//     Finally fin = [&] {
//         freeaddrinfo(result);
//     };
//     for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
//         if (rp->ai_family == AF_INET) {
//             std::memcpy(&addr, &reinterpret_cast<struct sockaddr_in
//             *>(rp->ai_addr)->sin_addr,
//                         sizeof(in_addr));
//             return IpAddress(addr);
//         } else if (rp->ai_family == AF_INET6) {
//             std::memcpy(&addr6,
//                         &reinterpret_cast<struct sockaddr_in6
//                         *>(rp->ai_addr)->sin6_addr, sizeof(in6_addr));
//             return IpAddress(addr6);
//         }
//     }
//     [[unlikely]] {
// #if CO_ASYNC_DEBUG
//         std::cerr << host << ": no matching host address with ipv4 or
//         ipv6\n";
// #endif
//         return std::errc::bad_address;
//     }
// }
//
// String IpAddress::toString() const {
//     if (mAddr.index() == 1) {
//         char buf[INET6_ADDRSTRLEN + 1] = {};
//         inet_ntop(AF_INET6, &std::get<1>(mAddr), buf, sizeof(buf));
//         return buf;
//     } else if (mAddr.index() == 0) {
//         char buf[INET_ADDRSTRLEN + 1] = {};
//         inet_ntop(AF_INET, &std::get<0>(mAddr), buf, sizeof(buf));
//         return buf;
//     } else {
//         return "[invalid ip address or domain name]";
//     }
// }

auto AddressResolver::resolve_all() -> Expected<ResolveResult> {
    // gethostbyname is deprecated, let's use getaddrinfo instead:
    if (m_host.empty()) [[unlikely]] {
        return std::errc::invalid_argument;
    }
    struct addrinfo *result;
    int err = getaddrinfo(m_host.c_str(),
                          m_service.empty() ? nullptr : m_service.c_str(),
                          &m_hints, &result);
    if (err) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << m_host << ": " << gai_strerror(err) << '\n';
#endif
        return std::error_code(err, getAddrInfoCategory());
    }
    Finally fin = [&] {
        freeaddrinfo(result);
    };
    ResolveResult res;
    for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
        res.addrs
            .emplace_back(rp->ai_addr, rp->ai_addrlen, rp->ai_family,
                          rp->ai_socktype, rp->ai_protocol)
            .trySetPort(m_port);
    }
    if (res.addrs.empty()) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << m_host << ": no matching host address\n";
#endif
        return std::errc::bad_address;
    }
    res.service = std::move(m_service);
    return res;
}

Expected<SocketAddress> AddressResolver::resolve_one() {
    auto res = resolve_all();
    if (res.has_error()) [[unlikely]] {
        return res.error();
    }
    return res->addrs.front();
}

Expected<SocketAddress> AddressResolver::resolve_one(std::string &service) {
    auto res = resolve_all();
    if (res.has_error()) [[unlikely]] {
        return res.error();
    }
    service = std::move(res->service);
    return res->addrs.front();
}

SocketAddress::SocketAddress(struct sockaddr const *addr, socklen_t addrLen,
                             sa_family_t family, int sockType, int protocol)
    : mSockType(sockType),
      mProtocol(protocol) {
    std::memcpy(&mAddr, addr, addrLen);
    mAddr.ss_family = family;
    mAddrLen = addrLen;
}

std::string SocketAddress::host() const {
    if (family() == AF_INET) {
        auto &sin =
            reinterpret_cast<struct sockaddr_in const &>(mAddr).sin_addr;
        char buf[INET_ADDRSTRLEN] = {};
        inet_ntop(family(), &sin, buf, sizeof(buf));
        return buf;
    } else if (family() == AF_INET6) {
        auto &sin6 =
            reinterpret_cast<struct sockaddr_in6 const &>(mAddr).sin6_addr;
        char buf[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &sin6, buf, sizeof(buf));
        return buf;
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

int SocketAddress::port() const {
    if (family() == AF_INET) {
        auto port =
            reinterpret_cast<struct sockaddr_in const &>(mAddr).sin_port;
        return ntohs(port);
    } else if (family() == AF_INET6) {
        auto port =
            reinterpret_cast<struct sockaddr_in6 const &>(mAddr).sin6_port;
        return ntohs(port);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

void SocketAddress::trySetPort(int port) {
    if (family() == AF_INET) {
        reinterpret_cast<struct sockaddr_in &>(mAddr).sin_port =
            htons(static_cast<uint16_t>(port));
    } else if (family() == AF_INET6) {
        reinterpret_cast<struct sockaddr_in6 &>(mAddr).sin6_port =
            htons(static_cast<uint16_t>(port));
    }
}

String SocketAddress::toString() const {
    return host() + ':' + to_string(port());
}

// void SocketAddress::initFromHostPort(struct in_addr const &host, int port) {
//     struct sockaddr_in saddr = {};
//     saddr.sin_family = AF_INET;
//     std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
//     saddr.sin_port = htons(static_cast<uint16_t>(port));
//     std::memcpy(&mAddrIpv4, &saddr, sizeof(saddr));
//     mAddrLen = sizeof(saddr);
// }
//
// void SocketAddress::initFromHostPort(struct in6_addr const &host, int port) {
//     struct sockaddr_in6 saddr = {};
//     saddr.sin6_family = AF_INET6;
//     std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
//     saddr.sin6_port = htons(static_cast<uint16_t>(port));
//     std::memcpy(&mAddrIpv6, &saddr, sizeof(saddr));
//     mAddrLen = sizeof(saddr);
// }

SocketAddress get_socket_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddr);
    throwingErrorErrno(getsockname(
        sock.fileNo(), reinterpret_cast<struct sockaddr *>(&sa.mAddr),
        &sa.mAddrLen));
    return sa;
}

SocketAddress get_socket_peer_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddr);
    throwingErrorErrno(getpeername(
        sock.fileNo(), reinterpret_cast<struct sockaddr *>(&sa.mAddr),
        &sa.mAddrLen));
    return sa;
}

Task<Expected<SocketHandle>> createSocket(int family, int type, int protocol) {
    int fd = co_await expectError(
                 co_await UringOp().prep_socket(family, type, protocol, 0))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument,
                           [&] { return socket(family, type, protocol); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr) {
    SocketHandle sock = co_await co_await createSocket(
        addr.family(), addr.socktype(), addr.protocol());
    co_await expectError(co_await UringOp().prep_connect(
        sock.fileNo(), reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
        addr.mAddrLen))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout) {
    SocketHandle sock = co_await co_await createSocket(
        addr.family(), addr.socktype(), addr.protocol());
    auto ts = durationToKernelTimespec(timeout);
    co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_connect(
            sock.fileNo(),
            reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
            addr.mAddrLen),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM, 0);
    if (cancel.is_canceled()) [[unlikely]] {
        co_return std::errc::operation_canceled;
    }
    co_await expectError(
        co_await UringOp()
            .prep_connect(
                sock.fileNo(),
                reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
                addr.mAddrLen)
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM, 0);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEADDR, 1);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEPORT, 1);
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_CORK, 0); */
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_NODELAY, 1); */
    /* co_await socketSetOption(sock, SOL_SOCKET, SO_KEEPALIVE, 1); */
    SocketListener serv(sock.releaseFile());
    co_await expectError(bind(
        serv.fileNo(), reinterpret_cast<struct sockaddr const *>(&addr.mAddr),
        addr.mAddrLen));
    co_await expectError(listen(serv.fileNo(), backlog));
    co_return serv;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener) {
    int fd = co_await expectError(
        co_await UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel) {
    int fd = co_await expectError(
        co_await UringOp()
            .prep_accept(listener.fileNo(), nullptr, nullptr, 0)
            .cancelGuard(cancel));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr) {
    int fd = co_await expectError(co_await UringOp().prep_accept(
        listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
        &peerAddr.mAddrLen, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(accept4(
                         listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel) {
    int fd = co_await expectError(
        co_await UringOp()
            .prep_accept(listener.fileNo(),
                         reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(accept4(
                         listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_send(sock.fileNo(), buf, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
                                       );
}

Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_send_zc(sock.fileNo(), buf, 0, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
                                       );
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_recv(sock.fileNo(), buf, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf,
                                         CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_send(sock.fileNo(), buf, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf,
                                            CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_send_zc(sock.fileNo(), buf, 0, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_recv(sock.fileNo(), buf, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp::link_ops(
            UringOp().prep_send(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp::link_ops(
            UringOp().prep_recv(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp::link_ops(
            UringOp().prep_send(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME))
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp::link_ops(
            UringOp().prep_recv(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME))
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<>> socket_shutdown(SocketHandle &sock, int how) {
    co_return expectError(co_await UringOp().prep_shutdown(sock.fileNo(), how));
}
} // namespace co_async

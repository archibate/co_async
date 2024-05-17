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
        virtual char const *name() const noexcept {
            return "getaddrinfo";
        }

        virtual std::string message(int e) const {
            return gai_strerror(e);
        }
    } instance;

    return instance;
}

Expected<IpAddress> IpAddress::parse(std::string_view host, bool allowIpv6) {
    return parse(std::string(host).c_str(), allowIpv6);
}

Expected<IpAddress> IpAddress::parse(char const *host, bool allowIpv6) {
    struct in_addr addr = {};
    struct in6_addr addr6 = {};
    if (1 == inet_pton(AF_INET, host, &addr)) {
        return IpAddress(addr);
    }
    if (allowIpv6 && 1 == inet_pton(AF_INET6, host, &addr6)) {
        return IpAddress(addr6);
    }
    // gethostbyname is deprecated, let's use getaddrinfo instead:
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *result;
    int err = getaddrinfo(host, NULL, &hints, &result);
    if (err) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << ip << ": " << gai_strerror(err) << '\n';
#endif
        return Unexpected{std::error_code(err, getAddrInfoCategory())};
    }
    Finally fin = [&] {
        freeaddrinfo(result);
    };
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            std::memcpy(&addr, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
                        sizeof(in_addr));
            return IpAddress(addr);
        } else if (allowIpv6 && rp->ai_family == AF_INET6) {
            std::memcpy(&addr6,
                        &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr,
                        sizeof(in6_addr));
            return IpAddress(addr6);
        }
    }
    [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << ip << ": no matching host address with ipv4 or ipv6\n";
#endif
        return Unexpected{std::make_error_code(std::errc::bad_address)};
    }
}

std::string IpAddress::toString() const {
    if (mAddr.index() == 1) {
        char buf[INET6_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET6, &std::get<1>(mAddr), buf, sizeof(buf));
        return buf;
    } else if (mAddr.index() == 0) {
        char buf[INET_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET, &std::get<0>(mAddr), buf, sizeof(buf));
        return buf;
    } else {
        return "[invalid ip address or domain name]";
    }
}

Expected<SocketAddress> SocketAddress::parse(std::string_view host,
                                             int defaultPort) {
    auto pos = host.rfind(':');
    std::string hostPart(host);
    std::optional<int> port;
    if (pos != std::string_view::npos) {
        hostPart = host.substr(0, pos);
        port = from_string<int>(host.substr(pos + 1));
        if (port < 0 || port > 65535) [[unlikely]] {
            port = std::nullopt;
        }
    }
    if (!port) {
        if (defaultPort == -1) [[unlikely]] {
            return Unexpected{std::make_error_code(std::errc::bad_address)};
        }
        port = defaultPort;
    }
    auto ip = IpAddress::parse(hostPart.c_str());
    if (ip.has_error()) [[unlikely]] {
        return Unexpected{ip.error()};
    }
    return SocketAddress(*ip, *port);
}

SocketAddress::SocketAddress(IpAddress ip, int port) {
    std::visit([&](auto const &addr) { initFromHostPort(addr, port); },
               ip.mAddr);
}

sa_family_t SocketAddress::family() const noexcept {
    return mAddr.sa_family;
}

IpAddress SocketAddress::host() const {
    if (family() == AF_INET) {
        return IpAddress(mAddrIpv4.sin_addr);
    } else if (family() == AF_INET6) {
        return IpAddress(mAddrIpv6.sin6_addr);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

int SocketAddress::port() const {
    if (family() == AF_INET) {
        return ntohs(mAddrIpv4.sin_port);
    } else if (family() == AF_INET6) {
        return ntohs(mAddrIpv6.sin6_port);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

std::string SocketAddress::toString() const {
    return host().toString() + ":" + to_string(port());
}

void SocketAddress::initFromHostPort(struct in_addr const &host, int port) {
    struct sockaddr_in saddr = {};
    saddr.sin_family = AF_INET;
    std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
    saddr.sin_port = htons((uint16_t)port);
    std::memcpy(&mAddrIpv4, &saddr, sizeof(saddr));
    mAddrLen = sizeof(saddr);
}

void SocketAddress::initFromHostPort(struct in6_addr const &host, int port) {
    struct sockaddr_in6 saddr = {};
    saddr.sin6_family = AF_INET6;
    std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
    saddr.sin6_port = htons((uint16_t)port);
    std::memcpy(&mAddrIpv6, &saddr, sizeof(saddr));
    mAddrLen = sizeof(saddr);
}

SocketAddress get_socket_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddrIpv6);
    throwingErrorErrno(
        getsockname(sock.fileNo(), (sockaddr *)&sa.mAddr, &sa.mAddrLen));
    return sa;
}

SocketAddress get_socket_peer_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddrIpv6);
    throwingErrorErrno(
        getpeername(sock.fileNo(), (sockaddr *)&sa.mAddr, &sa.mAddrLen));
    return sa;
}

Task<Expected<SocketHandle>> createSocket(int family, int type) {
    int fd = co_await expectError(
        co_await UringOp().prep_socket(family, type, 0, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    co_await expectError(co_await UringOp().prep_connect(
        sock.fileNo(), (const struct sockaddr *)&addr.mAddr, addr.mAddrLen));
    co_return sock;
}

Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    auto ts = durationToKernelTimespec(timeout);
    co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_connect(
            sock.fileNo(), (const struct sockaddr *)&addr.mAddr, addr.mAddrLen),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    if (cancel.is_canceled()) [[unlikely]] {
        co_return Unexpected{
            std::make_error_code(std::errc::operation_canceled)};
    }
    co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_connect(sock.fileNo(),
                               (const struct sockaddr *)&addr.mAddr,
                               addr.mAddrLen)));
    co_return sock;
}

Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEADDR, 1);
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_CORK, 0); */
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_NODELAY, 1); */
    /* co_await socketSetOption(sock, SOL_SOCKET, SO_KEEPALIVE, 1); */
    SocketListener serv(sock.releaseFile());
    co_await expectError(bind(
        serv.fileNo(), (struct sockaddr const *)&addr.mAddr, addr.mAddrLen));
    co_await expectError(listen(serv.fileNo(), backlog));
    co_return serv;
}

Task<Expected<SocketListener>>
listener_bind(std::pair<std::string, int> const &addr, int backlog) {
    co_return co_await listener_bind(
        co_await SocketAddress::parse(addr.first, addr.second));
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener) {
    int fd = co_await expectError(
        co_await UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel) {
    int fd = co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0)));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr) {
    int fd = co_await expectError(co_await UringOp().prep_accept(
        listener.fileNo(), (struct sockaddr *)&peerAddr.mAddr,
        &peerAddr.mAddrLen, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel) {
    int fd = co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_accept(listener.fileNo(),
                              (struct sockaddr *)&peerAddr.mAddr,
                              &peerAddr.mAddrLen, 0)));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_send(sock.fileNo(), buf, 0));
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_recv(sock.fileNo(), buf, 0));
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf,
                                         CancelToken cancel) {
    co_return (std::size_t) co_await expectError(
        co_await cancel.guard<UringOpCanceller>(
            UringOp().prep_send(sock.fileNo(), buf, 0)));
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel) {
    co_return (std::size_t) co_await expectError(
        co_await cancel.guard<UringOpCanceller>(
            UringOp().prep_recv(sock.fileNo(), buf, 0)));
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return (std::size_t) co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_send(sock.fileNo(), buf, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return (std::size_t) co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_recv(sock.fileNo(), buf, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
}

Task<Expected<>> socket_shutdown(SocketHandle &sock, int how) {
    co_return expectError(co_await UringOp().prep_shutdown(sock.fileNo(), how));
}
} // namespace co_async

#pragma once

#include <cerrno>
#include <span>
#include <string>
#include <string_view>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include "task.hpp"
#include "generator.hpp"
#include "epoll_loop.hpp"
#include "error_handling.hpp"

namespace co_async {

struct IpAddress {
    IpAddress(in_addr addr) noexcept : mAddr(addr) {}

    IpAddress(in6_addr addr6) noexcept : mAddr(addr6) {}

    IpAddress() = default;

    std::variant<in_addr, in6_addr> mAddr;
};

inline IpAddress ip_address(char const *ip) {
    in_addr addr = {};
    in6_addr addr6 = {};
    if (checkError(inet_pton(AF_INET, ip, &addr))) {
        return addr;
    }
    if (checkError(inet_pton(AF_INET6, ip, &addr6))) {
        return addr6;
    }
    hostent *hent = gethostbyname(ip);
    for (int i = 0; hent->h_addr_list[i]; i++) {
        if (hent->h_addrtype == AF_INET) {
            std::memcpy(&addr, hent->h_addr_list[i], sizeof(in_addr));
            return addr;
        } else if (hent->h_addrtype == AF_INET6) {
            std::memcpy(&addr6, hent->h_addr_list[i], sizeof(in6_addr));
            return addr6;
        }
    }
    throw std::invalid_argument("invalid domain name or ip address");
}

struct SocketAddress {
    SocketAddress() = default;

    SocketAddress(char const *path) {
        sockaddr_un saddr = {};
        saddr.sun_family = AF_UNIX;
        std::strncpy(saddr.sun_path, path, sizeof(saddr.sun_path) - 1);
        std::memcpy(&mAddr, &saddr, sizeof(saddr));
        mAddrLen = sizeof(saddr);
    }

    SocketAddress(in_addr host, int port) {
        sockaddr_in saddr = {};
        saddr.sin_family = AF_INET;
        std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
        saddr.sin_port = htons(port);
        std::memcpy(&mAddr, &saddr, sizeof(saddr));
        mAddrLen = sizeof(saddr);
    }

    SocketAddress(in6_addr host, int port) {
        sockaddr_in6 saddr = {};
        saddr.sin6_family = AF_INET6;
        std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
        saddr.sin6_port = htons(port);
        std::memcpy(&mAddr, &saddr, sizeof(saddr));
        mAddrLen = sizeof(saddr);
    }

    sockaddr_storage mAddr;
    socklen_t mAddrLen;
};

inline AsyncFile create_udp_socket(SocketAddress const &addr) {
    AsyncFile sock(socket(addr.mAddr.ss_family, SOCK_DGRAM, 0));
    return sock;
}

inline SocketAddress socket_address(IpAddress ip, int port) {
    return std::visit(
        [&](auto const &addr) { return SocketAddress(addr, port); }, ip.mAddr);
}

inline SocketAddress socketGetAddress(AsyncFile &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddr);
    checkError(getsockname(sock.fileNo(), (sockaddr *)&sa.mAddr, &sa.mAddrLen));
    return sa;
}

template <class T>
inline T socketGetOption(AsyncFile &sock, int level, int optId) {
    T optVal;
    socklen_t optLen = sizeof(optVal);
    checkError(
        getsockopt(sock.fileNo(), level, optId, (sockaddr *)&optVal, &optLen));
    return optVal;
}

template <class T>
inline void socketSetOption(AsyncFile &sock, int level, int opt,
                            T const &optVal) {
    checkError(setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

inline Task<void> socketConnect(EpollLoop &loop, AsyncFile &sock,
                                SocketAddress const &addr) {
    sock.setNonblock();
    int res = checkErrorNonBlock(
        connect(sock.fileNo(), (sockaddr const *)&addr.mAddr, addr.mAddrLen),
        -1, EINPROGRESS);
    if (res == -1) [[likely]] {
        co_await wait_file_event(loop, sock, EPOLLOUT);
        int err = socketGetOption<int>(sock, SOL_SOCKET, SO_ERROR);
        if (err != 0) [[unlikely]] {
            throw std::system_error(err, std::system_category(), "connect");
        }
    }
}

inline Task<AsyncFile> create_tcp_client(EpollLoop &loop,
                                         SocketAddress const &addr) {
    AsyncFile sock(socket(addr.mAddr.ss_family, SOCK_STREAM, 0));
    co_await socketConnect(loop, sock, addr);
    co_return sock;
}

inline Task<void> socketBind(EpollLoop &loop, AsyncFile &sock, auto const &addr,
                             int backlog = SOMAXCONN) {
    sock.setNonblock();
    checkError(
        bind(sock.fileNo(), (sockaddr const *)&addr.mAddr, addr.mAddrLen));
    co_await wait_file_event(loop, sock, EPOLLOUT);
    int err = socketGetOption<int>(sock, SOL_SOCKET, SO_ERROR);
    if (err != 0) [[unlikely]] {
        throw std::system_error(err, std::system_category(), "bind");
    }
}

inline Task<AsyncFile> create_tcp_server(EpollLoop &loop,
                                         SocketAddress const &addr) {
    AsyncFile sock(socket(addr.mAddr.ss_family, SOCK_STREAM, 0));
    co_await socketBind(loop, sock, addr);
    co_return sock;
}

inline void socket_listen(AsyncFile &sock, int backlog = SOMAXCONN) {
    checkError(listen(sock.fileNo(), backlog));
}

inline void socket_shotdown(AsyncFile &sock, int flags = SHUT_RDWR) {
    checkError(shutdown(sock.fileNo(), flags));
}

template <class AddrType>
inline Task<std::tuple<AsyncFile, AddrType>> socket_accept(EpollLoop &loop,
                                                           AsyncFile &sock) {
    struct sockaddr_storage sockAddr;
    socklen_t addrLen = sizeof(sockAddr);
    co_await wait_file_event(loop, sock, EPOLLIN);
    int res = checkError(accept4(sock.fileNo(), (struct sockaddr *)&sockAddr,
                                 &addrLen, SOCK_NONBLOCK));
    AddrType addr;
    if (sockAddr.ss_family == AF_INET) {
        addr = ((struct sockaddr_in *)&sockAddr)->sin_addr;
    } else if (sockAddr.ss_family == AF_INET6) {
        addr = ((struct sockaddr_in6 *)&sockAddr)->sin6_addr;
    } else [[unlikely]] {
        throw std::runtime_error("unknown address family");
    }
    co_return {AsyncFile(res), addr};
}

} // namespace co_async

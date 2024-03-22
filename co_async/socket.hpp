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
#include <co_async/task.hpp>
#include <co_async/generator.hpp>
#include <co_async/epoll_loop.hpp>
#include <co_async/error_handling.hpp>

namespace co_async {

struct UnixAddress {
    struct sockaddr_un mSockAddr;

    static constexpr sa_family_t family = AF_UNIX;

    explicit UnixAddress(char const *path) {
        mSockAddr.sun_family = AF_UNIX;
        if (strlen(path) >= sizeof(mSockAddr.sun_path)) [[unlikely]] {
            throw std::invalid_argument("unix path too long");
        }
        strncpy(mSockAddr.sun_path, path, sizeof(mSockAddr.sun_path) - 1);
    }
};

struct Ipv4Address {
    struct sockaddr_in mSockAddr;

    static constexpr sa_family_t family = AF_INET;

    explicit Ipv4Address(char const *ip, int port) {
        mSockAddr.sin_family = AF_INET;
        mSockAddr.sin_port = htons(port);
        int res = checkError(inet_pton(AF_INET, ip, &mSockAddr.sin_addr));
        if (res == 0) {
            struct hostent *hent = gethostbyname2(ip, AF_INET);
            if (hent->h_addrtype != AF_INET || hent->h_length != sizeof(mSockAddr.sin_addr)) [[unlikely]] {
                throw std::invalid_argument("no ipv4 address");
            }
            std::memcpy(&mSockAddr.sin_addr, hent->h_addr_list[0], sizeof(mSockAddr.sin_addr));
        }
    }

    std::string host() const {
        char buf[INET_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET, &mSockAddr.sin_addr, buf, sizeof(buf));
        return buf;
    }

    int port() const noexcept {
        return ntohs(mSockAddr.sin_port);
    }

    std::string repr() const {
        return host() + ":" + std::to_string(port());
    }
};

struct Ipv6Address {
    struct sockaddr_in6 mSockAddr;

    static constexpr sa_family_t family = AF_INET6;

    explicit Ipv6Address(char const *ip, int port) {
        mSockAddr.sin6_family = AF_INET6;
        mSockAddr.sin6_port = htons(port);
        int res = checkError(inet_pton(AF_INET6, ip, &mSockAddr.sin6_addr));
        if (res == 0) {
            struct hostent *hent = gethostbyname2(ip, AF_INET6);
            if (hent->h_addrtype != AF_INET6 || hent->h_length != sizeof(mSockAddr.sin6_addr)) [[unlikely]] {
                throw std::invalid_argument("no ipv6 address");
            }
            std::memcpy(&mSockAddr.sin6_addr, hent->h_addr_list[0], sizeof(mSockAddr.sin6_addr));
        }
    }

    std::string host() const {
        char buf[INET6_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET6, &mSockAddr.sin6_addr, buf, sizeof(buf));
        return buf;
    }

    int port() const noexcept {
        return ntohs(mSockAddr.sin6_port);
    }

    std::string repr() const {
        return host() + ":" + std::to_string(port());
    }
};

template <class AddrType, int sockType>
struct AsyncSocket : AsyncFile {
    AsyncSocket()
        : AsyncFile(checkError(socket(AddrType::family, sockType, 0))) {}
};

template <class AddrType>
inline auto tcp_socket() {
    return AsyncSocket<AddrType, SOCK_STREAM>();
}

template <class AddrType>
inline auto udp_socket() {
    return AsyncSocket<AddrType, SOCK_DGRAM>();
}

template <class AddrType>
inline AddrType socketGetAddress(AsyncFile &sock) {
    AddrType addr;
    socklen_t addrLen = sizeof(addr.mSockAddr);
    checkError(getsockname(sock.fileNo(), (struct sockaddr *)&addr.mSockAddr,
                           &addrLen));
    return addr;
}

template <class T>
inline T socketGetOption(AsyncFile &sock, int level, int optId) {
    T optVal;
    socklen_t optLen = sizeof(optVal);
    checkError(getsockopt(sock.fileNo(), level, optId,
                          (struct sockaddr *)&optVal, &optLen));
    return optVal;
}

template <class T>
inline void socketSetOption(AsyncFile &sock, int level, int opt,
                            T const &optVal) {
    checkError(setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

inline Task<void> socket_connect(EpollLoop &loop, AsyncFile &sock,
                                 auto const &addr) {
    sock.setNonblock();
    checkErrorNonBlock(connect(sock.fileNo(),
                               (struct sockaddr const *)&addr.mSockAddr,
                               sizeof(addr.mSockAddr)),
                       0, EINPROGRESS);
    co_await wait_file_event(loop, sock, EPOLLOUT);
    int err = socketGetOption<int>(sock, SOL_SOCKET, SO_ERROR);
    if (err != 0) [[unlikely]] {
        throw std::system_error(err, std::system_category(), "connect");
    }
}

inline std::size_t socketSendSync(AsyncFile &sock,
                                  std::span<char const> buffer) {
    return checkErrorNonBlock(
        send(sock.fileNo(), buffer.data(), buffer.size(), MSG_DONTWAIT));
}

inline std::size_t socketRecvSync(AsyncFile &sock, std::span<char> buffer) {
    return checkErrorNonBlock(
        recv(sock.fileNo(), buffer.data(), buffer.size(), MSG_DONTWAIT));
}

inline Generator<std::size_t> socket_send(EpollLoop &loop, AsyncFile &sock,
                                          std::span<char const> buffer) {
    std::size_t sent = 0;
    while (true) {
        co_await wait_file_event(loop, sock, EPOLLOUT);
        while (true) {
            auto chunk = buffer.size() - sent;
            auto len = socketSendSync(sock, {buffer.data() + sent, chunk});
            sent += len;
            if (len != chunk) {
                break;
            }
            if (sent == buffer.size())
                co_return;
            co_yield sent;
        }
        co_yield sent;
    }
}

inline Task<std::size_t> socket_recv(EpollLoop &loop, AsyncFile &sock,
                                     std::span<char> buffer) {
    std::size_t received = 0;
    co_await wait_file_event(loop, sock, EPOLLIN);
    while (received != buffer.size()) {
        auto chunk = buffer.size() - received;
        auto len = socketRecvSync(sock, {buffer.data() + received, chunk});
        received += len;
        if (len != chunk) {
            break;
        }
    }
    co_return received;
}

inline Task<void> socket_bind(EpollLoop &loop, AsyncFile &sock,
                              auto const &addr, int backlog = SOMAXCONN) {
    sock.setNonblock();
    checkError(bind(sock.fileNo(), (struct sockaddr const *)&addr.mSockAddr,
                    sizeof(addr.mSockAddr)));
    co_await wait_file_event(loop, sock, EPOLLOUT);
    int err = socketGetOption<int>(sock, SOL_SOCKET, SO_ERROR);
    if (err != 0) [[unlikely]] {
        throw std::system_error(err, std::system_category(), "bind");
    }
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
    AddrType addr;
    socklen_t addrLen = sizeof(addr.mSockAddr);
    co_await wait_file_event(loop, sock, EPOLLIN);
    int res =
        checkError(accept4(sock.fileNo(), (struct sockaddr *)&addr.mSockAddr,
                           &addrLen, SOCK_NONBLOCK));
    co_return {AsyncFile(res), addr};
}

} // namespace co_async

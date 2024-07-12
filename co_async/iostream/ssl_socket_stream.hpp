#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/pimpl.hpp>

namespace co_async {
std::error_category const &bearSSLCategory();

StructPImpl(SSLClientTrustAnchor) {
    Expected<> add(std::string_view content);
};

StructPImpl(SSLServerPrivateKey){};

StructPImpl(SSLServerCertificate) {
    Expected<> add(std::string_view content);
};

StructPImpl(SSLServerSessionCache){};
Task<Expected<OwningStream>>
ssl_connect(char const *host, int port, SSLClientTrustAnchor const &ta,
            std::span<char const *const> protocols, std::string_view proxy,
            std::chrono::steady_clock::duration timeout);
OwningStream ssl_accept(SocketHandle file, SSLServerCertificate const &cert,
                        SSLServerPrivateKey const &pkey,
                        std::span<char const *const> protocols,
                        SSLServerSessionCache *cache = nullptr);
} // namespace co_async

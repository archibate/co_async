#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/expected.hpp>

namespace co_async {
Task<Expected<SocketHandle>>
socket_proxy_connect(char const *host, int port, std::string_view proxy,
                     std::chrono::steady_clock::duration timeout);
} // namespace co_async

#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/net/http_server.hpp>
#include <co_async/net/http_server_utils.hpp>
#include <hashlib/hashlib.hpp>

namespace co_async {

// 小彭老师带你用 C++ 实现 WebSocket 协议
// WebSocket 是为了解决 HTTP 协议的一些缺陷而提出的
// 过去，HTTP 只能客户端单向地往服务端发出请求，服务端被动地返回响应
// 要获取服务端的动态数据，只能通过轮询或长轮询的形式（参见上一期视频）
// 在聊天室这种少量人员的场景中，轮询还扛得住，但在实时音视频，直播领域，HTTP 轮询的延迟就太高了
// 而 WebSocket 是一种双向通信协议，建立连接后，服务端和客户端都能主动向对方发送或接收数据
// WebSocket 面向二进制字节流，类似于 TCP，比基于文本传输数据的 HTTP 协议更高效
// 主流浏览器都提供 WebSocket 的 API，可以实现浏览器和服务器的双向实时通信
// 我们这一期视频主要来实现 C++ 的服务器端，要求能够与浏览器中的 JS 建立 WebSocket 连接
// 如果时间来得及，我们希望利用这个 WebSocket 服务器实现实时语音通话

inline std::string websocketSecretHash(std::string userKey) {
    // websocket 官方要求的神秘仪式
    SHA1 sha1;
    std::string inKey = userKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1.add(inKey.data(), inKey.size());
    unsigned char buf[SHA1::HashBytes];
    sha1.getHash(buf);
    return base64::encode_into<std::string>(buf, buf + SHA1::HashBytes);
}

inline Task<Expected<>> http_upgrade_to_websocket(HTTPServer::IO &io) {
    auto upgrade = io.request.headers.get("upgrade").value_or("");
    if (upgrade != "websocket") {
        co_return std::errc::wrong_protocol_type;
    }
    // 是 ws:// 请求
    auto wsKey = io.request.headers.get("sec-websocket-key");
    if (!wsKey) {
        co_await co_await HTTPServerUtils::make_error_response(io, 400);
        co_return {};
    }
    auto wsNewKey = websocketSecretHash(*wsKey);
    HTTPResponse res{
        .status = 101,
        .headers =
        {
            {"connection", "Upgrade"},
            {"upgrade", "websocket"},
            {"sec-websocket-accept", wsNewKey},
        },
    };
    co_await co_await io.response(res, "");
    co_return {};
}

}

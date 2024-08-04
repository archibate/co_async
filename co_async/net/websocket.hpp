#pragma once
#include <co_async/std.hpp>
#include <co_async/generic/timeout.hpp>
#include <co_async/generic/when_any.hpp>
#include <co_async/utils/byteorder.hpp>
#include <co_async/utils/expected.hpp>
#include <co_async/utils/reflect.hpp>
#include <co_async/utils/random.hpp>
#include <co_async/net/http_client.hpp>
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

inline std::string websocketGenerateNonce() {
    uint32_t seed = getSeedByTime();
    uint8_t buf[16];
    for (size_t i = 0; i != 16; ++i) {
        seed = wangsHash(seed);
        buf[i] = static_cast<uint8_t>(seed & 0xFF);
    }
    return base64::encode_into<std::string>(buf, buf + 16);
}

inline std::string websocketSecretHash(std::string userKey) {
    // websocket 官方要求的神秘仪式
    SHA1 sha1;
    std::string inKey = userKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1.add(inKey.data(), inKey.size());
    uint8_t buf[SHA1::HashBytes];
    sha1.getHash(buf);
    return base64::encode_into<std::string>(buf, buf + SHA1::HashBytes);
}

inline Task<Expected<bool>> httpUpgradeToWebSocket(HTTPServer::IO &io) {
    if (io.request.headers.get("upgrade") != "websocket") {
        co_return false;
    }
    // 是 ws:// 请求
    auto wsKey = io.request.headers.get("sec-websocket-key");
    if (!wsKey) {
        co_await co_await HTTPServerUtils::make_error_response(io, 400);
        co_return true;
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
    co_return true;
}

struct WebSocketPacket {
    enum Opcode : uint8_t {
        kOpcodeContinue = 0,
        kOpcodeText = 1,
        kOpcodeBinary = 2,
        kOpcodeClose = 8,
        kOpcodePing = 9,
        kOpcodePong = 10,
    } opcode;
    std::string content;

    REFLECT(opcode, content);
};

inline Task<Expected<WebSocketPacket>> wsRecvPacket(BorrowedStream &ws) {
    WebSocketPacket packet;
    packet.opcode = WebSocketPacket::kOpcodeContinue;
    bool fin;
    do {
        auto head = co_await co_await ws.getn(2);
        uint8_t head0 = static_cast<uint8_t>(head[0]);
        uint8_t head1 = static_cast<uint8_t>(head[1]);
        fin = (head0 & 0x80) != 0;
        auto new_opcode = static_cast<WebSocketPacket::Opcode>(head0 & 0x0F);
        if (new_opcode != WebSocketPacket::kOpcodeContinue) {
            packet.opcode = new_opcode;
        }
        bool masked = (head1 & 0x80) != 0;
        uint8_t payloadLen8 = head1 & 0x7F;
        size_t payloadLen;
        if (packet.opcode >= 8 && packet.opcode <= 10 && payloadLen8 >= 0x7E) [[unlikely]] {
            co_return std::errc::protocol_error;
        }
        if (payloadLen8 == 0x7E) {
            auto payloadLen16 = byteswap_if_little(co_await co_await ws.getstruct<uint16_t>());
            payloadLen = static_cast<size_t>(payloadLen16);
        } else if (payloadLen8 == 0x7F) {
            auto payloadLen64 = byteswap_if_little(co_await co_await ws.getstruct<uint64_t>());
            if constexpr (sizeof(uint64_t) > sizeof(size_t)) {
                if (payloadLen64 > std::numeric_limits<size_t>::max()) {
                    co_return std::errc::not_enough_memory;
                }
            }
            payloadLen = static_cast<size_t>(payloadLen64);
        } else {
            payloadLen = static_cast<size_t>(payloadLen8);
        }
        std::string mask;
        if (masked) {
            mask = co_await co_await ws.getn(4);
        }
        auto data = co_await co_await ws.getn(payloadLen);
        if (masked) {
            for (size_t i = 0; i != data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }
        }
        packet.content += data;
    } while (!fin);
    co_return std::move(packet);
}

inline Task<Expected<>> wsSendPacket(BorrowedStream &ws, WebSocketPacket packet, uint32_t mask = 0) {
    const bool fin = true;
    bool masked = mask != 0;
    uint8_t payloadLen8 = 0;
    if (packet.content.size() < 0x7E) {
        payloadLen8 = static_cast<uint8_t>(packet.content.size());
    } else if (packet.content.size() <= 0xFFFF) {
        payloadLen8 = 0x7E;
    } else {
        payloadLen8 = 0x7F;
    }
    uint8_t head0 = (fin ? 1 : 0) << 7 | static_cast<uint8_t>(packet.opcode);
    uint8_t head1 = (masked ? 1 : 0) << 7 | payloadLen8;
    char head[2];
    head[0] = static_cast<uint8_t>(head0);
    head[1] = static_cast<uint8_t>(head1);
    co_await co_await ws.write(head);
    if (packet.content.size() > 0x7E) {
        if (packet.content.size() <= 0xFFFF) {
            auto payloadLen16 = static_cast<uint16_t>(packet.content.size());
            co_await co_await ws.putstruct(byteswap_if_little(payloadLen16));
        } else {
            auto payloadLen64 = static_cast<uint64_t>(packet.content.size());
            co_await co_await ws.putstruct(byteswap_if_little(payloadLen64));
        }
    }
    if (masked) {
        char mask_buf[4];
        mask_buf[0] = mask >> 24;
        mask_buf[1] = (mask >> 16) & 0xFF;
        mask_buf[2] = (mask >> 8) & 0xFF;
        mask_buf[3] = mask & 0xFF;
        co_await co_await ws.write(mask_buf);
        for (size_t i = 0; i != packet.content.size(); ++i) {
            packet.content[i] ^= mask_buf[i % 4];
        }
    }
    co_await co_await ws.write(packet.content);
    co_await co_await ws.flush();
    co_return {};
}

struct WebSocket {
    BorrowedStream &sock;
    std::function<Task<Expected<>>(std::string const &)> mOnMessage;
    std::function<Task<Expected<>>()> mOnClose;
    std::function<Task<Expected<>>(std::chrono::steady_clock::duration)> mOnPong;
    bool mHalfClosed = false;
    bool mWaitingPong = true;
    std::chrono::steady_clock::time_point mLastPingTime{};

    WebSocket(WebSocket &&) = default;

    explicit WebSocket(BorrowedStream &sock) : sock(sock) {
    }

    bool is_closing() const noexcept {
        return mHalfClosed;
    }

    void on_message(std::function<Task<Expected<>>(std::string const &)> onMessage) {
        mOnMessage = std::move(onMessage);
    }

    void on_close(std::function<Task<Expected<>>()> onClose) {
        mOnClose = std::move(onClose);
    }

    void on_pong(std::function<Task<Expected<>>(std::chrono::steady_clock::duration)> onPong) {
        mOnPong = std::move(onPong);
    }

    Task<Expected<>> send(std::string text) {
        if (mHalfClosed) [[unlikely]] {
            co_return std::errc::broken_pipe;
        }
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodeText,
            .content = text,
        });
    }

    Task<Expected<>> close(uint16_t code = 1000) {
        std::string content;
        code = byteswap_if_little(code);
        content.resize(sizeof(code));
        std::memcpy(content.data(), &code, sizeof(code));
        mHalfClosed = true;
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodeClose,
            .content = content,
        });
    }

    Task<Expected<>> sendPing() {
        mLastPingTime = std::chrono::steady_clock::now();
        // debug(), "主动ping";
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodePing,
            .content = {},
        });
    }

    Task<Expected<>> start(std::chrono::steady_clock::duration pingPongTimeout = std::chrono::seconds(5)) {
        while (true) {
            auto maybePacket = co_await co_timeout(wsRecvPacket(sock), pingPongTimeout);
            if (maybePacket == std::errc::stream_timeout) {
                if (mWaitingPong) {
                    break;
                }
                co_await co_await sendPing();
                mWaitingPong = true;
                continue;
            }
            mWaitingPong = false;
            if (maybePacket == eofError()) {
                break;
            }
            auto packet = co_await std::move(maybePacket);
            if (packet.opcode == packet.kOpcodeText || packet.opcode == packet.kOpcodeBinary) {
                if (mOnMessage) {
                    co_await co_await mOnMessage(packet.content);
                }
            } else if (packet.opcode == packet.kOpcodePing) {
                // debug(), "收到ping";
                packet.opcode = packet.kOpcodePong;
                co_await co_await wsSendPacket(sock, packet);
            } else if (packet.opcode == packet.kOpcodePong) {
                auto now = std::chrono::steady_clock::now();
                if (mOnPong && mLastPingTime.time_since_epoch().count() != 0) {
                    auto dt = now - mLastPingTime;
                    co_await co_await mOnPong(dt);
                    // debug(), "网络延迟:", dt;
                }
                // debug(), "收到pong";
            } else if (packet.opcode == packet.kOpcodeClose) {
                // debug(), "收到关闭请求";
                if (mOnClose) {
                    co_await co_await mOnClose();
                }
                if (!mHalfClosed) {
                    co_await co_await wsSendPacket(sock, packet);
                    mHalfClosed = true;
                } else {
                    break;
                }
            }
        }
        co_await sock.close();
        co_return {};
    }
};

inline Task<Expected<WebSocket>> websocket_server(HTTPServer::IO &io) {
    if (co_await co_await httpUpgradeToWebSocket(io)) {
        co_return WebSocket(io.extractSocket());
    }
    co_return std::errc::protocol_error;
}

inline Task<Expected<WebSocket>> websocket_client(HTTPConnection &conn, URI uri) {
    std::string nonceKey;
    using namespace std::string_literals;
    nonceKey = websocketGenerateNonce();
    HTTPRequest request = {
        .method = "GET"s,
        .uri = uri,
        .headers = {
            {"sec-websocket-key"s, nonceKey},
            {"connection"s, "Upgrade"s},
            {"upgrade"s, "websocket"s},
            {"sec-websocket-version"s, "13"s},
        },
    };
    auto [response, _] = co_await co_await conn.request(request);
    if (response.headers.get("sec-websocket-accept") != websocketSecretHash(nonceKey)) {
        co_return std::errc::protocol_error;
    }
    co_return WebSocket(conn.extractSocket());
}

}

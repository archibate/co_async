#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static auto index_html = R"html(<!DOCTYPE html>
<html>
<head>
	<title>小彭老师在线聊天室 2.0</title>
	<meta charset="UTF-8">
    <style>
        .message p {
            margin: 3px;
        }
        div.message {
            margin: 0px;
            padding: 6px;
        }
        div.input-bar {
            display: flex;
            flex-direction: row;
            margin-top: 10px;
        }
        .input-user {
            flex: 1;
        }
        .input-content {
            flex: 1;
        }
        .message-user {
            color: #666666;
            font-weight: 600;
        }
        .message-user.message-current-user {
            color: #559966;
            font-weight: 600;
        }
        .message-content {
            color: #333333;
            font-weight: 300;
        }
        .brief {
            color: #444444;
            font-weight: 300;
        }
        body {
            display: flex;
            justify-content: center;
        }
        #container {
            max-width: 800px;
        }
        </style>
    </head>
    <body>
        <div id="container">
            <h1>小彭老师在线聊天室 2.0</h1>
            <p class="brief">小彭老师自主研发的一款基于 WebSocket 的现代 C++ 全栈聊天服务器 | <a href="https://github.com/archibate/co_async">GitHub 仓库</a></p>
            <div class="message-list" id="messages"></div>
            <div class="input-bar">
                <input class="input-user" type="text" id="user" placeholder="你的昵称"/>
                <button class="login-button" id="login">登录</button>
                <label class="message-user" hidden id="user-label" for="content"></label>&nbsp;<input hidden class="input-content" type="text" id="content" placeholder="输入你的消息..." autocomplete="off"/>
                <button hidden class="send-button" id="send">发送</button>
            </div>
        </div>
        <script src="https://unpkg.com/jquery@3.7.1/dist/jquery.min.js"></script>
        <script>
        var current_user = $("#user").val().trim();
        function escapeHtml(unsafe) {
            return unsafe
                .replace(/&/g, "&amp;")
                .replace(/</g, "&lt;")
                .replace(/>/g, "&gt;")
                .replace(/"/g, "&quot;")
                .replace(/'/g, "&#039;");
        }
        $(document).ready(function() {
            var ws = new WebSocket("ws://127.0.0.1:8080/");
            $("#login").click(function() {
                var user = $("#user").val().trim();
                if (user == '') {
                    alert("昵称不能为空");
                    return;
                }
                ws.send(JSON.stringify({type: 'LoginParams', object: {user}}));
                $("#send").show();
                $("#content").show();
                $("#user-label").show();
                $("#login").hide();
                $("#user").hide();
                $("#user-label").text(`${escapeHtml(user)}:`);
            });
            $("#send").click(function() {
                var content = $("#content").val().trim();
                if (content == '') {
                    alert("消息不能为空");
                    return;
                }
                $("#content").val('');
                ws.send(JSON.stringify({type: 'SendParams', object: {content}}));
            });
            $("#content").keypress(function(event) {
                if (event.keyCode === 13) {
                    $("#send").click();
                }
            });
            ws.onerror = function (evt) {
                console.log('发生错误:', evt);
                $("#messages").append('<div class="message"><p class="message-user">系统提示:</p><p class="message-content">连接发生了错误<p></div>');
            };
            ws.onmessage = function (evt) {
                console.log('收到消息:', evt.data);
                var recvParams = JSON.parse(evt.data);
                var message = recvParams.message;
                var extra_class = '';
                var user = $("#user").val().trim();
                if (message.user == user)
                    extra_class = ' message-current-user';
                $("#messages").append(`<div class="message"><p class="message-user${extra_class}">${escapeHtml(message.user)}:</p><p class="message-content">${escapeHtml(message.content)}<p></div>`);
            }
            ws.onclose = function (evt) {
                console.log('关闭了:', evt);
                $("#messages").append('<div class="message"><p class="message-user">系统提示:</p><p class="message-content">连接已关闭，请刷新重新登录<p></div>');
            };
        });
        </script>
    </body>
</html>)html"sv;

struct Message {
    std::string user;
    std::string content;

    REFLECT(user, content);
};

struct SendParams {
    std::string content;

    REFLECT(content);
    static constexpr const char *name = "SendParams";
};

struct RecvParams {
    Message message;

    REFLECT(message);
    static constexpr const char *name = "RecvParams";
};

struct LoginParams {
    std::string user;

    REFLECT(user);
    static constexpr const char *name = "LoginParams";
};

struct Room {
    std::vector<Message> mMessages;
    ConditionVariable mMessageUpdated;
};

static Room room;

static Task<Expected<>> roomPoller(WebSocket &ws) {
    size_t lastSize = 0;
    while (true) {
        while (room.mMessages.size() <= lastSize) {
            co_await co_await room.mMessageUpdated.wait();
        }
        for (size_t i = lastSize; i < room.mMessages.size(); ++i) {
            if (ws.is_closing())
                break;
            RecvParams recvParams = {
                .message = room.mMessages[i],
            };
            co_await co_await ws.send(json_encode(std::move(recvParams)));
        }
        lastSize = room.mMessages.size();
    }
    co_return {};
}

static Task<Expected<>> amain(std::string addr) {
    co_await co_await stdio().putline("正在监听: "s + addr);
    auto listener = co_await co_await listener_bind(co_await AddressResolver()
                                                    .host(addr).resolve_one());
    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        if (auto ws = co_await websocket_server(io)) {
            // 是升级的 ws:// 请求
            co_await co_await stdio().putline("连接成功"sv);
            std::string loginUser;
            ws->on_message([&] (std::string const &data) -> Task<Expected<>> {
                co_await co_await stdio().putline("收到消息: "s + data);
                std::visit(overloaded{
                    [&] (SendParams &&sendParams) {
                    room.mMessages.push_back(Message{
                        .user = loginUser,
                        .content = std::move(sendParams.content),
                    });
                    room.mMessageUpdated.notify_all();
                    },
                    [&] (LoginParams &&loginParams) {
                        loginUser = loginParams.user;
                    },
                }, co_await json_decode<std::variant<LoginParams, SendParams>>(data));
                co_return {};
            });
            ws->on_close([&] () -> Task<Expected<>> {
                co_await co_await stdio().putline("正在关闭连接"sv);
                co_return {};
            });
            ws->on_pong([&] (std::chrono::steady_clock::duration dt) -> Task<Expected<>> {
                co_await co_await stdio().putline("网络延迟: "s + to_string(
                    std::chrono::duration_cast<std::chrono::milliseconds>(dt).count()) + "ms"s);
                co_return {};
            });
            co_await (co_await when_any_common(ws->start(), roomPoller(*ws))).value;
            co_return {};
        } else {
            // 是普通 http:// 请求
            co_await co_await HTTPServerUtils::make_ok_response(io, index_html);
            co_return {};
        }
    });

    while (true) {
        if (auto income = co_await listener_accept(listener)) {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
}

int main(int argc, char **argv) {
    std::string addr = "http://127.0.0.1:8080";
    if (argc > 1) {
        addr = argv[1];
    }
    co_main(amain(addr));
    return 0;
}

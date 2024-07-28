#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static auto index_html = R"html(<!DOCTYPE html>
<html>
<head>
	<title>WebSocket 客户端</title>
	<meta charset="UTF-8">
    </head>
    <body>
        <div id="container">
            <h3>WebSocket 客户端</h3>
            <p>小彭老师自主研发的一款</p>
            <input type="text" id="content" placeholder="输入你的消息..." autocomplete="off"/>
            <button id="send">发送</button>
            <p>客户端日志:</p>
            <textarea disabled id="log" rows="20" cols="50" autocomplete="off"></textarea>
        </div>
        <script src="https://unpkg.com/jquery@3.7.1/dist/jquery.min.js"></script>
        <script>
        function log() {
            function addIndent(nSpaces) {
                var strOutput = '';
                for(var i = 0; i < nSpaces; i++) {
                    strOutput += '  ';
                }
                return strOutput; 
            }
            function parseObjToStr(oObject, nLevel) {
                if (typeof oObject !== 'string') {
                    var strOutput = "{\n";
                    nLevel = nLevel || 0;
                    for (var oEl in oObject) {
                        if (typeof oObject[oEl] === 'object'
                            || Object.prototype.toString.call(oObject[oEl]) === '[object Array]') {
                            strOutput += addIndent(nLevel) + oEl + " = ";
                            strOutput += parseObjToStr(oObject[oEl], nLevel+1) + ",\n";
                        } else {
                            var str = oObject[oEl].toString();
                            if (typeof oObject[oEl] === 'string') {
                                str = "'" + str.replaceAll("'", "\\'").replaceAll('\n', '\\n') + "'";
                            } else {
                                str = str.replaceAll('\n', '\n' + addIndent(nLevel));
                            }
                            strOutput += addIndent(nLevel) + oEl + " = " + str + ",\n";
                        }
                    }
                    strOutput += addIndent(nLevel) + "}";
                    return strOutput;
                } else {
                    return oObject;
                }
            }
            var content = "";
            for (var i = 0; i < arguments.length; i++) {
                content += parseObjToStr(arguments[i], 1) + " ";
            }
            $('#log').val($('#log').val() + content + "\n");
        }

        $(document).ready(function() {

            var ws = new WebSocket("ws://127.0.0.1:8080/");
            ws.onopen = function() {
                log("连接成功！");
            };
            ws.onmessage = function (evt) { 
                var response = evt.data;
                log("收到消息：", response);
            };
            ws.onclose = function() { 
                log("连接已关闭"); 
            };
            ws.onerror = function(err) {
                log("发生错误：", err);
            };

            $('#send').click(function() {
                var content = $('#content').val();
                log("正在发送数据", content);
                ws.send(content);
            });

        });
        </script>
    </body>
</html>)html"sv;

static Task<Expected<>> amain(std::string addr) {
    co_await co_await stdio().putline("正在监听: "s + addr);
    auto listener = co_await co_await listener_bind(co_await AddressResolver()
                                                    .host(addr).resolve_one());
    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        if (auto ws = co_await websocket_server(io)) {
            // 是升级的 ws:// 请求
            co_await co_await stdio().putline("连接成功"sv);
            ws->on_message([&] (std::string const &message) -> Task<Expected<>> {
                co_await co_await stdio().putline("收到消息: "s + message);
                co_await co_await ws->send("收到啦！"s + message);
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
            co_await co_await ws->start();
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

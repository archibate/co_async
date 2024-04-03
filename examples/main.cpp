#define DEBUG_SHOW 1
#include "debug.hpp"

import co_async;
import std;

using namespace co_async;
using namespace std::literals;

Task<> amain() {
    auto serv = co_await server_bind({"127.0.0.1", 8080});
    HTTPServer http;

    debug(), "正在监听", serv.address();
    while (1) {
        auto conn = co_await server_accept(serv);
        debug(), "收到请求", serv.address();
        enqueue(http.processConnection(FileStream(std::move(conn))));
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    run_task_on(loop, amain());
    return 0;
}

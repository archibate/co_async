# co_async

基于 C++20 协程的高并发异步 I/O 库（小彭老师教学用）

## 教学视频

- [第一集：C++20 协程概念初上手](https://www.bilibili.com/video/BV1Yz421Z7rZ)
- [第二集：封装 epoll 实现 HTTP 客户端](https://www.bilibili.com/video/BV18t421G7fD)
- [第三集：io_uring 实现 HTTP 服务器](https://www.bilibili.com/video/BV1yD421H7KY)
- [第四集：进一步完善 HTTP 路由，实现线程池](https://space.bilibili.com/263032155)（施工中，稍后上传）

> [steps 目录](steps) 是本库代码逐渐成形的过程，可以配合教学视频自己动手尝试。

## 使用案例

```cpp
import co_async;
import std;

using namespace co_async;

Task<> amain() {
    auto listener = co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer http;
    http.route("/", [] (HTTPRequest const &request) -> Task<HTTPResponse> {
        if (request.method != "GET")
            co_return HTTPServer::make_error_response(405);
        co_return {
            .status = 200,
            .headers = {
                {"content-type", "text/plain;charset=utf-8"},
            },
            .body = "<h1>It works!</h1>",
        };
    });

    co_await stdio().putline("正在监听: " + listener.address().toString());
    while (1) {
        auto conn = co_await listener_accept(listener);
        co_await stdio().putline("收到请求: " + listener.address().toString());
        co_spawn(http.process_connection(FileStream(std::move(conn))));
    }
}

int main() {
    co_spawn_and_wait(amain());
    return 0;
}
```

> [examples 目录](examples) 中有更多案例。

## 平台要求

- Linux 内核 >= 5.1
- GCC >= 10
- Clang >= 16

> 小彭老师推荐使用 Arch Linux 系统作为开发平台

或者：

- Windows >= 10（施工中，稍后支持）
- Visual Studio >= 2022（施工中，稍后支持）

## 安装与导入

### 作为单个头文件导入

[点击此处](co_async.hpp) 下载 co_async.hpp，然后在你的项目中引入即可：

```cpp
#include "co_async.hpp"
```

### 作为 C++20 模块导入

将本项目下载到你的项目中作为子文件夹，引入并链接：

```cmake
add_subdirectory(co_async)
target_link_libraries(你的名字 PRIVATE co_async)
```

在你的代码中 import 即可：

```cpp
import co_async;
```

> 需要 GCC >= 11、Clang >= 17、MSVC >= 19 以支持 C++20 模块

## Benchmark

- regular function call 25ns
- coroutine function call 80ns
- coroutine function call (with exception) 150ns
- io uring nop 800ns
- io uring open 7us
- io uring statx 11us
- io uring read/write 80us

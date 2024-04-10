# co_async

基于 C++20 协程的高并发异步 I/O 库（小彭老师教学用）

## 教学视频

- [第一集：C++20 协程概念初上手](https://www.bilibili.com/video/BV1Yz421Z7rZ)
- [第二集：封装 epoll 实现 HTTP 客户端](https://www.bilibili.com/video/BV18t421G7fD)
- [第三集：io uring 实现 HTTP 服务器](https://www.bilibili.com/video/BV1yD421H7KY)
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
                {"content-type", "text/html;charset=utf-8"},
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
    std::ios::sync_with_stdio(false);
    co_synchronize(amain());
    return 0;
}
```

> [examples 目录](examples) 中有更多案例。

## 平台要求

### Linux

- Linux 内核 >= 5.1
- GCC >= 10
- Clang >= 10

小彭老师推荐使用 Arch Linux 系统作为开发平台，Ubuntu 20.04 需要手动升级一下 gcc 版本：

```bash
sudo apt install -y g++-10 libstdc++-10-dev
export CXX=g++10
```

### Windows

- Windows >= 10（施工中，稍后支持）
- Visual Studio >= 2022（施工中，稍后支持）

### Docker 环境构建

如果你的 Ubuntu 标准库版本太老，无法编译，可以试试看小彭老师提供的 [Docker 环境](Dockerfile)：

```bash
docker build -t my_archlinux_image .  # 构建 Docker 镜像
docker run -it -p 8080:8080 my_archlinux_image   # 运行并进入 Docker 镜像，映射端口 8080 到本机
```

在 Docker 容器中，构建本项目：

```bash
cmake -B build
cmake --build build --parallel 8
build/server  # 对应于 examples/server.cpp
```

## 安装与导入

### 作为单个头文件导入

[点击此处](scripts/co_async.hpp) 下载 `co_async.hpp`，然后在你的项目中引入即可：

```cpp
#include "co_async.hpp"
```

> 若为 Linux 系统则需要编译时加上 `-luring` 选项。

### 作为普通库导入

将本项目下载到你的项目中作为子文件夹，引入并链接：

```cmake
add_subdirectory(co_async)
target_link_libraries(你的名字 PRIVATE co_async)
```

在你的代码中导入即可：

```cpp
#include <co_async/co_async.hpp>
```

### 作为 C++20 模块导入

将本项目下载到你的项目中作为子文件夹，开启 CO_ASYNC_MODULE 选项后引入并链接：

```cmake
set(CO_ASYNC_MODULE ON)
add_subdirectory(co_async)
target_link_libraries(你的名字 PRIVATE co_async)
```

在你的代码中 import 即可：

```cpp
import co_async;
```

> 需要 GCC >= 11 或 Clang >= 17 或 MSVC >= 19，以及 CMake >= 3.28 且使用 `cmake -G Ninja` 选项

### 额外 CMake 选项

```bash
cmake -B build -DCO_ASYNC_DEBUG=ON  # 启用调试与安全性检测
cmake -B build -DCO_ASYNC_EXCEPT=ON  # 启用异常（会影响协程函数性能）
cmake -B build -DCO_ASYNC_PERF=ON  # 启用性能测试（程序结束时自动打印测时结果）
```

## 性能测试

测试平台：AMD Ryzen 7 7800X3D（8 核心 16 线程 4.5 GHz）

- 普通函数调用 25ns
- 协程函数调用 80ns
- 协程函数调用（启用异常）150ns
- I/O 基础延迟 0.8µs
- I/O 打开文件 7µs
- I/O 查询文件信息 11µs
- I/O 读写 ~1MB 文件 80µs

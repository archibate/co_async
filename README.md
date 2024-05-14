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
#include <co_async/co_async.hpp>

using namespace co_async;
using namespace std::literals;

Task<Expected<>> amain() {
    // 第一下 co_await 等待 bind 操作完成，返回一个 Expected<T> 对象。
    // Expected<T> 有两种状态：1. 正常返回 T 值；2. 返回非零错误码 std::errc
    // 第二下 co_await 可以将 Expected<T> 的错误转化为当前函数的错误（没有错误照常返回 T）：
    auto listener = co_await co_await listener_bind({"127.0.0.1", 8080});
    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        HTTPResponse res = {
            .status = 200,
            .headers = {
                {"content-type", "text/html;charset=utf-8"},
            },
        };
        std::string_view body = "<h1>It works!</h1>";
        co_await io.response(res, body);
        co_return {};
    });

    while (true) {
        if (auto income = co_await listener_accept(listener)) {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
}

int main() {
    IOContext().join(amain()).value();
    return 0;
}
```

> [examples 目录](examples) 中有更多案例。

## 平台要求

### Linux

- Linux 内核 >= 5.19
- GCC >= 10
- Clang >= 16

小彭老师推荐使用 Arch Linux 系统作为开发平台，Ubuntu 20.04 的话需要手动升级一下 gcc 版本：

```bash
sudo apt install -y g++-10 libstdc++-10-dev
export CXX=g++10
rm -rf build
cmake -B build
cmake --build build --parallel 8
build/server  # 对应于 examples/server.cpp
```

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

> Docker 继承宿主机的 Linux 内核版本，因此 Docker 无法解决 Linux 内核版本过低的问题。

### Windows

- Windows >= 10（施工中，稍后支持）
- Visual Studio >= 2022（施工中，稍后支持）

## 安装与导入

### 作为纯头文件导入

将本项目下载到你的项目中作为子文件夹，引入即可：

```cpp
#include <co_async/co_async.hpp>
```

编译选项：`g++ -std=c++20 你的项目.cpp -I 本项目根目录 -luring -lbearssl -lz`

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

### 额外 CMake 选项

```bash
cmake -B build -DCO_ASYNC_DEBUG=ON  # 启用调试与安全性检测
cmake -B build -DCO_ASYNC_EXCEPT=ON  # 启用异常（会影响协程函数性能）
cmake -B build -DCO_ASYNC_PERF=ON  # 启用性能测试（程序结束时自动打印测时结果）
cmake -B build -DCO_ASYNC_ZLIB=ON  # 启用压缩支持（需要链接 /usr/lib/libz.so）
```

## 性能测试

测试平台：AMD Ryzen 7 7800X3D（8 核心 16 线程 4.5 GHz）
Linux 内核：6.8.2
GCC 编译器：13.2.1
内存：2 x 16 GB（DDR5 4800 MHz）

- 普通函数调用 25ns
- 协程函数调用 80ns
- 协程函数调用（启用异常）150ns
- I/O 基础延迟 0.8µs
- 打开文件 7µs
- 查询文件信息 11µs
- 读写 ~1MB 文件 80µs

百万并发 HTTP 压力测试 examples/stress_test.cpp：

```
$ wrk -t16 -c1000 -d20s http://127.0.0.1:8080/
Running 20s test @ http://127.0.0.1:8080/
  16 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.58ms    6.07ms 222.19ms   98.78%
    Req/Sec    79.18k     9.15k  154.00k    94.73%
  25281650 requests in 20.09s, 2.83GB read
Requests/sec: 1258548.22
Transfer/sec:    144.03MB
```

```
$ wrk -t16 -c1000 -d20s http://127.0.0.1:8080/
Running 5s test @ http://127.0.0.1:8080/
  16 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.22ms    1.59ms  71.79ms   87.44%
    Req/Sec    69.34k     5.62k  136.62k    82.67%
  5561407 requests in 5.08s, 1.12GB read
Requests/sec: 1095696.72
Transfer/sec:    225.71MB
```

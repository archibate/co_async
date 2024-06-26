#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/and_then.hpp"
#include <thread>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

using namespace std::chrono_literals;

int main() {
    // 把0号输入流设为非阻塞的，这样当你read时如果没有数据，会直接返回EWOULDBLOCK错误
    int attr = 1;
    ioctl(0, FIONBIO, &attr);

    // 创建异步控制器
    int epfd = epoll_create1(0);

    // 创建异步监听器（目标：0号输入流，事件：有输入数据）
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = 0;
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    // 开始异步读取输入流
    while (true) {
        struct epoll_event ebuf[10];
        int res = epoll_wait(epfd, ebuf, 10, 1000);
        if (res == -1) {
            debug(), "epoll出错了：", strerror(errno);
        }
        if (res == 0) {
            debug(), "epoll超时了，1秒内没有等到任何输入";
        }
        for (int i = 0; i < res; i++) {
            debug(), "等到了输入事件！";
            int fd = ebuf[i].data.fd;
            char c;
            while (true) {
                int len = read(fd, &c, 1);
                if (len <= 0) { // 表示需要阻塞了
                    if (errno == EWOULDBLOCK) {
                        debug(), "read: 前面的区域，以后再来探索8～";
                        break;
                    }
                    debug(), "read出错了", strerror(errno);
                }
                debug(), c;
            }
        }
    }
    return 0;
}

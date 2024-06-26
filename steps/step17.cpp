#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/epoll_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/limit_timeout.hpp"
#include "co_async/and_then.hpp"
#include <thread>
#include <cstring>
#include <termios.h>

[[gnu::constructor]] static void disable_canon() {
    struct termios tc;
    tcgetattr(STDIN_FILENO, &tc);
    tc.c_lflag &= ~ICANON;
    tc.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tc);
}

using namespace std::chrono_literals;

co_async::EpollLoop epollLoop;
co_async::TimerLoop timerLoop;

char map[20][20];
int x = 10;
int y = 10;
int dx = 0;
int dy = 0;
bool running;

void on_key(char c) {
    if (c == 'q') {
        running = false;
    } else if (c == 'a') {
        dx = -1;
        dy = 0;
    } else if (c == 'd') {
        dx = 1;
        dy = 0;
    } else if (c == 'w') {
        dx = 0;
        dy = -1;
    } else if (c == 's') {
        dx = 0;
        dy = 1;
    }
}

void on_time() {
    if (x + dx >= 20 || x + dx < 0 ||
        y + dy >= 20 || y + dy < 0) {
        running = false;
        return;
    }
    x += dx;
    y += dy;
}

void on_draw() {
    std::memset(map, ' ', sizeof(map));
    map[y][x] = '@';
    std::string s = "\x1b[H\x1b[2J\x1b[3J";
    for (int i = 0; i < 20; ++i) {
        s += '#';
    }
    s += '\n';
    for (int i = 0; i < 20; ++i) {
        s += '#';
        for (int j = 0; j < 20; ++j) {
            s += map[i][j];
        }
        s += "#\n";
    }
    for (int i = 0; i < 20; ++i) {
        s += '#';
    }
    s += '\n';
    write(STDOUT_FILENO, s.data(), s.size());
}

co_async::Task<> async_main() {
    co_async::AsyncFile file(STDIN_FILENO);
    auto nextTp = std::chrono::system_clock::now();
    running = true;
    while (true) {
        auto res = co_await limit_timeout(timerLoop, read_string(epollLoop, file), nextTp);
        if (res) {
            for (char c: *res) {
                on_key(c);
            }
            on_draw();
        } else {
            on_time();
            if (!running) break;
            on_draw();
            nextTp = std::chrono::system_clock::now() + 500ms;
        }
    }
}

int main() {
    auto t = async_main();
    t.mCoroutine.resume();
    while (true) {
        auto timeout = timerLoop.tryRun();
        auto hasEvent = epollLoop.runFor(timeout);
        if (!timeout && !hasEvent) break;
    }
    return 0;
}

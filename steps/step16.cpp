#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/epoll_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/and_then.hpp"
#include <thread>
#include <fcntl.h>
#include <unistd.h>

using namespace std::chrono_literals;

co_async::EpollLoop epollLoop;
co_async::TimerLoop timerLoop;

co_async::Task<std::string> read_string(co_async::AsyncFile &file) {
    co_await wait_file(epollLoop, file, EPOLLIN);
    std::string s;
    size_t chunk = 8;
    while (true) {
        char c;
        std::size_t exist = s.size();
        s.resize(exist + chunk);
        std::span<char> buffer(s.data() + exist, chunk);
        auto len = read_file(file, buffer);
        if (len != chunk) {
            s.resize(exist + len);
            break;
        }
        if (chunk < 65536)
            chunk *= 4;
    }
    co_return s;
}

co_async::Task<void> async_main() {
    co_async::AsyncFile file(STDIN_FILENO);
    while (true) {
        auto s = co_await read_string(file);
        debug(), "读到了", s;
        if (s == "quit\n")
            break;
    }
}

int main() {
    auto t = async_main();
    t.mCoroutine.resume();
    while (!t.mCoroutine.done()) {
        auto timeout = timerLoop.tryRun();
        epollLoop.runFor(timeout);
    }
    return 0;
}

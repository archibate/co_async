#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <co_async/generator.hpp>
#include <co_async/timer_loop.hpp>
#include <co_async/epoll_loop.hpp>
#include <co_async/async_loop.hpp>
#include <co_async/when_any.hpp>
#include <co_async/when_all.hpp>
#include <co_async/limit_timeout.hpp>
#include <co_async/and_then.hpp>
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

co_async::AsyncLoop loop;

co_async::Task<> snake() {
    co_async::AsyncFile file(STDIN_FILENO);
    while (true) {
        auto res = co_await co_async::when_any(co_async::sleep_for(loop, 0.1s), co_async::read_string(loop, file));
        debug(), res;
    }
}

int main() {
    run_task(loop, snake());
    return 0;
}

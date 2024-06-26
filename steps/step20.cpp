#include "co_async/debug.hpp"
#include "co_async/task.hpp"
#include "co_async/generator.hpp"
#include "co_async/timer_loop.hpp"
#include "co_async/epoll_loop.hpp"
#include "co_async/async_loop.hpp"
#include "co_async/when_any.hpp"
#include "co_async/when_all.hpp"
#include "co_async/limit_timeout.hpp"
#include "co_async/and_then.hpp"
#include "co_async/stdio.hpp"
#include "co_async/stream.hpp"

using namespace std::literals;

co_async::AsyncLoop loop;

co_async::Task<> amain() {
    co_async::Istream ain(loop, co_async::async_stdin(true));
    while (true) {
        auto s = co_await ain.getline(": ");
        debug(), s;
        s = co_await ain.getline('\n');
        debug(), s;
    }
    co_return;
}

int main() {
    run_task(loop, amain());
    return 0;
}

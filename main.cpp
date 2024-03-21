#include <co_async/debug.hpp>
#include <co_async/task.hpp>
#include <co_async/timer_loop.hpp>
#include <co_async/when_any.hpp>
#include <co_async/when_all.hpp>
#include <co_async/and_then.hpp>
#include <thread>

using namespace std::chrono_literals;

co_async::TimerLoop loop;

co_async::Task<int> hello1() {
    debug(), "hello1开始睡1秒";
    co_await sleep_for(loop, 1s);
    debug(), "hello1睡醒了";
    co_return 1;
}

co_async::Task<int> hello2() {
    debug(), "hello2开始睡2秒";
    co_await sleep_for(loop, 2s);
    debug(), "hello2睡醒了";
    co_return 2;
}

co_async::Task<int> hello() {
    debug(), "hello开始等1和2";
    auto [i, j] = co_await when_all(hello1(), hello2());
    debug(), "hello看到", i, j;
    co_return i + j;
}

int main() {
    auto t = hello();
    while (!t.mCoroutine.done()) {
        t.mCoroutine.resume();
        if (auto delay = loop.tryRun())
            std::this_thread::sleep_for(*delay);
    }
    debug(), "主函数中得到hello结果:", t.operator co_await().await_resume();
    return 0;
}

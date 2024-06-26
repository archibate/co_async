#pragma once

#include "timer_loop.hpp"
#include "epoll_loop.hpp"
#include <thread>

namespace co_async {

struct AsyncLoop {
    void run() {
        while (true) {
            auto timeout = mTimerLoop.run();
            if (mEpollLoop.hasEvent()) {
                mEpollLoop.run(timeout);
            } else if (timeout) {
                std::this_thread::sleep_for(*timeout);
            } else {
                break;
            }
        }
    }

    operator TimerLoop &() {
        return mTimerLoop;
    }

    operator EpollLoop &() {
        return mEpollLoop;
    }

private:
    TimerLoop mTimerLoop;
    EpollLoop mEpollLoop;
};

} // namespace co_async

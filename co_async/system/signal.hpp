#pragma once

#include <co_async/std.hpp>
#include <co_async/threading/io_context_mt.hpp>

#ifdef __linux__

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <co_async/system/error_handling.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {

struct SignalingContextMT {
    static void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            sigset_t s;
            sigemptyset(&s);
            std::unique_lock lock(instance->mMutex);
            for (auto [signo, waiters]: instance->mWaitingSignals) {
                sigaddset(&s, signo);
            }
            lock.unlock();
            int signo;
            debug(), 1;
            throwingError(-sigwait(&s, &signo));
            debug(), 1, signo;
            lock.lock();
            std::deque<std::coroutine_handle<>> waiters;
            waiters.swap(instance->mWaitingSignals.at(signo));
            lock.unlock();
            for (auto coroutine: waiters) {
                IOContextMT::spawn(coroutine);
            }
        }
    }

    struct SignalAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            std::lock_guard lock(instance->mMutex);
            instance->mWaitingSignals[mSigno].push_back(coroutine);
        }

        void await_resume() const noexcept {}

        int mSigno;
    };

    static SignalAwaiter waitSignal(int signo) {
        return SignalAwaiter(signo);
    }

    static void start() {
        instance->mWorker = std::jthread([] (std::stop_token stop) {
            startMain(stop);
        });
    }

    static inline SignalingContextMT *instance;

    SignalingContextMT() {
        if (instance) {
            throw std::logic_error(
                "each process may contain only one SignalingContextMT");
        }
        instance = this;
        start();
    }

    SignalingContextMT(SignalingContextMT &&) = delete;

    ~SignalingContextMT() {
        instance = nullptr;
    }

private:
    std::map<int, std::deque<std::coroutine_handle<>>> mWaitingSignals;
    std::mutex mMutex;
    std::jthread mWorker;
};

} // namespace co_async
#endif

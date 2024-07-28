// #pragma once
// #include <co_async/std.hpp>
// #include <co_async/awaiter/task.hpp>
// #include <co_async/generic/io_context_mt.hpp>
// #include <co_async/platform/error_handling.hpp>
// #include <signal.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
//
// namespace co_async {
// struct SignalingContextMT {
//     static void startMain(std::stop_token stop) {
//         while (!stop.stop_requested()) [[likely]] {
//             sigset_t s;
//             sigemptyset(&s);
//             std::unique_lock lock(instance->mMutex);
//             for (auto [signo, waiters]: instance->mWaitingSignals) {
//                 sigaddset(&s, signo);
//             }
//             lock.unlock();
//             int signo;
//             throwingError(-sigwait(&s, &signo));
//             lock.lock();
//             std::deque<std::coroutine_handle<>> waiters;
//             waiters.swap(instance->mWaitingSignals.at(signo));
//             lock.unlock();
//             for (auto coroutine: waiters) {
//                 IOContextMT::spawn(coroutine);
//             }
//         }
//     }
//
//     struct SignalAwaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             std::lock_guard lock(instance->mMutex);
//             instance->mWaitingSignals[mSigno].push_back(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         int mSigno;
//     };
//
//     static SignalAwaiter waitSignal(int signo) {
//         return SignalAwaiter(signo);
//     }
//
//     static void start() {
//         instance->mWorker =
//             std::jthread([](std::stop_token stop) { startMain(stop); });
//     }
//
//     static inline SignalingContextMT *instance;
//
//     SignalingContextMT() {
//         if (instance) {
//             throw std::logic_error(
//                 "each process may contain only one SignalingContextMT");
//         }
//         instance = this;
//         start();
//     }
//
//     SignalingContextMT(SignalingContextMT &&) = delete;
//
//     ~SignalingContextMT() {
//         instance = nullptr;
//     }
//
// private:
//     std::map<int, std::deque<std::coroutine_handle<>>> mWaitingSignals;
//     std::mutex mMutex;
//     std::jthread mWorker;
// };
// } // namespace co_async

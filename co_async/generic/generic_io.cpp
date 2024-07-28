#include <co_async/std.hpp>
#include <co_async/generic/generic_io.hpp>

namespace co_async {

// bool GenericIOContext::runComputeOnly() {
//     if (auto coroutine = mQueue.pop()) {
//         coroutine->resume();
//         return true;
//     }
//     return false;
// }
//
GenericIOContext::GenericIOContext() = default;
GenericIOContext::~GenericIOContext() = default;

std::optional<std::chrono::steady_clock::duration>
GenericIOContext::runDuration() {
    while (true) {
        if (!mTimers.empty()) {
            auto &promise = mTimers.front();
            std::chrono::steady_clock::time_point now =
                std::chrono::steady_clock::now();
            if (promise.mExpires <= now) {
                promise.mCancelled = false;
                promise.erase_from_parent();
                std::coroutine_handle<TimerNode>::from_promise(promise).resume();
                continue;
            } else {
                return promise.mExpires - now;
            }
        } else {
            return std::nullopt;
        }
    }
}

//
// void GenericIOContext::startMain(std::stop_token stop) {
//     while (!stop.stop_requested()) [[likely]] {
//         auto duration = runDuration();
//         if (duration) {
//             std::this_thread::sleep_for(*duration);
//         } else {
//             break;
//         }
//     }
// }
//
} // namespace co_async

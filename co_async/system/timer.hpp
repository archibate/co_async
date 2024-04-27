

#ifdef __linux__
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#pragma once

#include <co_async/std.hpp>

#ifdef __linux__

#include <co_async/awaiter/task.hpp>
#include <co_async/system/system_loop.hpp>

namespace co_async {

template <class Rep, class Period>
inline Task<> sleep_for(std::chrono::duration<Rep, Period> dur) {
    auto ts = durationToKernelTimespec(dur);
    int ret = co_await uring_timeout(&ts, 1, IORING_TIMEOUT_BOOTTIME);
    if (ret != -ETIME)
        checkErrorReturn(ret);
}

template <class Clk, class Dur>
inline Task<> sleep_until(std::chrono::time_point<Clk, Dur> tp) {
    auto ts = timePointToKernelTimespec(tp);
    int ret = co_await uring_timeout(
        &ts, 1, IORING_TIMEOUT_ABS | IORING_TIMEOUT_REALTIME);
    if (ret != -ETIME)
        checkErrorReturn(ret);
}

} // namespace co_async

#endif

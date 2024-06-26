#pragma once

#include <chrono>
#include <optional>
#include <utility>
#include "timer_loop.hpp"
#include "when_any.hpp"
#include "concepts.hpp"

namespace co_async {

template <Awaitable A, class Rep, class Period>
Task<std::optional<typename AwaitableTraits<A>::RetType>>
limit_timeout(TimerLoop &loop, A &&a,
              std::chrono::duration<Rep, Period> duration) {
    auto v = co_await when_any(std::forward<A>(a), sleep_for(loop, duration));
    if (auto *ret = std::get_if<0>(&v)) {
        co_return std::move(*ret);
    } else {
        co_return std::nullopt;
    }
}

template <Awaitable A, class Clk, class Dur>
Task<std::optional<typename AwaitableTraits<A>::RetType>>
limit_timeout(TimerLoop &loop, A &&a,
              std::chrono::time_point<Clk, Dur> expireTime) {
    auto v =
        co_await when_any(std::forward<A>(a), sleep_until(loop, expireTime));
    if (auto *ret = std::get_if<0>(&v)) {
        co_return std::move(*ret);
    } else {
        co_return std::nullopt;
    }
}

} // namespace co_async

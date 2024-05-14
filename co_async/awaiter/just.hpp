#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>

namespace co_async {
inline Task<> just_void() {
    co_return;
}

template <class T>
Task<T> just_value(T t) {
    co_return std::move(t);
}

template <class F, class... Args>
Task<std::invoke_result_t<F, Args...>> just_invoke(F &&f, Args &&...args) {
    co_return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}
} // namespace co_async

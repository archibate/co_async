#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

std::array<OwningStream, 2> pipe_stream();
Task<Expected<>> pipe_forward(BorrowedStream &in, BorrowedStream &out);

template <class Func, class... Args>
    requires std::invocable<Func, Args..., OwningStream &>
inline Task<Expected<>> pipe_bind(OwningStream w, Func &&func, Args &&...args) {
    return co_bind(
        [func = std::forward<decltype(func)>(func),
         w = std::move(w)](auto &&...args) mutable -> Task<Expected<>> {
            auto e1 =
                co_await std::invoke(std::forward<decltype(func)>(func),
                                     std::forward<decltype(args)>(args)..., w);
            auto e2 = co_await w.flush();
            co_await w.close();
            co_await e1;
            co_await e2;
            co_return {};
        },
        std::forward<decltype(args)>(args)...);
}
} // namespace co_async

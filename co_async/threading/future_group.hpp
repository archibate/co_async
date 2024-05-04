#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/threading/future.hpp>

namespace co_async {

struct FutureGroup {
    std::vector<FutureSource<Expected<void, std::errc>>> mFutures;

    FutureGroup &add(FutureSource<Expected<void, std::errc>> future) {
        mFutures.push_back(std::move(future));
        return *this;
    }

    FutureGroup &add(Task<Expected<void, std::errc>> task) {
        add(co_future(std::move(task)));
        return *this;
    }

    template <class F, class... Args>
        requires(std::same_as<std::invoke_result_t<F, Args...>,
                              Task<Expected<void, std::errc>>>)
    FutureGroup &add(F &&f, Args &&...args) {
        add(co_future(
            co_bind(std::forward<F>(f), std::forward<Args>(args)...)));
        return *this;
    }

    Task<Expected<void, std::errc>> wait() {
        for (auto &result: co_await when_all(mFutures)) {
            co_await std::move(result);
        }
        mFutures.clear();
        co_return {};
    }

    FutureGroup() = default;
    FutureGroup(FutureGroup &&) = default;
    FutureGroup &operator=(FutureGroup &&) = default;
};

} // namespace co_async

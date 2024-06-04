#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/concepts.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/generic/cancel.hpp>
#include <co_async/generic/generic_io.hpp>

namespace co_async {

template <class T>
struct WhenAnyResult {
    T value;
    std::size_t index;
};

template <Awaitable T, class Alloc = std::allocator<T>>
Task<WhenAnyResult<typename AwaitableTraits<T>::AvoidRetType>>
when_any(std::vector<T, Alloc> const &tasks) {
    CancelSource cancel(co_await co_cancel);
    std::vector<Task<>, Alloc> newTasks(tasks.size(), tasks.get_allocator());
    std::optional<typename AwaitableTraits<T>::RetType> result;
    std::size_t index = (std::size_t)-1;
    std::size_t i = 0;
    for (auto &&task: tasks) {
        newTasks.push_back(co_cancel.bind(
            cancel,
            co_bind([&, i, cancel = cancel.token()]() mutable -> Task<> {
                result.emplace((co_await std::move(task), Void()));
                if (cancel.is_canceled()) {
                    co_return;
                }
                co_await cancel.cancel();
                index = i;
            })));
        ++i;
    }
    co_await when_all(newTasks);
    co_return {std::move(result.value()), index};
}

template <Awaitable... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
when_any(Ts &&...tasks) {
    return co_bind(
        [&]<std::size_t... Is>(std::index_sequence<Is...>)
            -> Task<
                std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>> {
            CancelSource cancel(co_await co_cancel);
            std::optional<
                std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
                result;
            co_await when_all(co_cancel.bind(
                cancel,
                co_bind([&result, task = std::move(tasks)]() mutable -> Task<> {
                    auto res = (co_await std::move(task), Void());
                    if (co_await co_cancel) {
                        co_return;
                    }
                    co_await co_cancel.cancel();
                    result.emplace(std::in_place_index<Is>, std::move(res));
                }))...);
            co_return std::move(result.value());
        },
        std::make_index_sequence<sizeof...(Ts)>());
}

template <Awaitable... Ts,
          class Common = typename AwaitableTraits<
              typename TypeList<Ts...>::FirstType>::AvoidRetType>
    requires(std::same_as<typename AwaitableTraits<Ts>::AvoidRetType, Common> &&
             ...)
Task<WhenAnyResult<typename AwaitableTraits<Ts>::AvoidRetType...>>
when_any_common(Ts &&...tasks) {
    return co_bind(
        [&]<std::size_t... Is>(std::index_sequence<Is...>)
            -> Task<
                std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>> {
            CancelSource cancel(co_await co_cancel);
            std::size_t index = (std::size_t)-1;
            std::optional<Common> result;
            co_await when_all(co_cancel.bind(
                cancel, co_bind([&index, &result,
                                 task = std::move(tasks)]() mutable -> Task<> {
                    auto res = (co_await std::move(task), Void());
                    if (co_await co_cancel) {
                        co_return;
                    }
                    co_await co_cancel.cancel();
                    index = Is;
                    result.emplace(std::move(res));
                }))...);
            co_return WhenAnyResult{std::move(result.value()), index};
        },
        std::make_index_sequence<sizeof...(Ts)>());
}

} // namespace co_async

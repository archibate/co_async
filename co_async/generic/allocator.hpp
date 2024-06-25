#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/just.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/awaiter/when_all.hpp>
#include <co_async/utils/ilist.hpp>

namespace co_async {

struct GetThisAllocator {
    template <class T>
    ValueAwaiter<std::pmr::memory_resource *> operator()(TaskPromise<T> &promise) const {
#if CO_ASYNC_ALLOC
        auto allocator = promise.mLocals.mAllocator;
        if (allocator == nullptr)
            allocator = std::pmr::get_default_resource();
        return ValueAwaiter<std::pmr::memory_resource *>(allocator);
#else
        return {std::pmr::get_default_resource()};
#endif
    }

    struct DoSetAllocator {
        template <class T>
        ValueAwaiter<> operator()(TaskPromise<T> &promise) const {
#if CO_ASYNC_ALLOC
            promise.mLocals.mAllocator = mAllocator;
#endif
            return {Void{}};
        }

        std::pmr::memory_resource *mAllocator;
    };

    static DoSetAllocator set_allocator(std::pmr::memory_resource *allocator) {
        return {allocator};
    }

    template <class T>
    static T &&bind(std::pmr::memory_resource *allocator, T &&task) {
#if CO_ASYNC_ALLOC
        task.promise().mLocals.mAllocator = allocator;
#endif
        return std::forward<T>(task);
    }
};

inline constexpr GetThisAllocator co_alloc;

#if CO_ASYNC_ALLOC
#define CO_ASYNC_PMR {co_await co_alloc}
#define CO_ASYNC_PMR1 , co_await co_alloc
#define CO_ASYNC_PMR_ARG_DEF std::pmr::polymorphic_allocator<> pmrAllocator = {}
#define CO_ASYNC_PMR_ARG_DEF1 , std::pmr::polymorphic_allocator<> pmrAllocator = {}
#define CO_ASYNC_PMR_ARG_USE pmrAllocator
#define CO_ASYNC_PMR_ARG_USE1 , pmrAllocator

using String = std::pmr::string;
#else
#define CO_ASYNC_PMR
#define CO_ASYNC_PMR1
#define CO_ASYNC_PMR_ARG_DEF
#define CO_ASYNC_PMR_ARG_DEF1
#define CO_ASYNC_PMR_ARG_USE
#define CO_ASYNC_PMR_ARG_USE1

using String = std::string;
#endif

inline String operator""_s(const char *str, size_t len) {
    return String(str, len);
}

} // namespace co_async

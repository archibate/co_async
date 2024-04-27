#pragma once

#include <co_async/std.hpp>

namespace co_async {

template <class T = void>
struct NonVoidHelper {
    using Type = T;
};

template <>
struct NonVoidHelper<void> {
    using Type = NonVoidHelper;

    explicit NonVoidHelper() = default;

    template <class T>
    constexpr friend T &&operator,(T &&t, NonVoidHelper) {
        return std::forward<T>(t);
    }

    char const *repr() const noexcept {
        return "NonVoidHelper";
    }

    static void *operator new(std::size_t) noexcept {
        static char unused[sizeof(NonVoidHelper)];
        return &unused;
    }

    static void operator delete(void *) noexcept {}
};

} // namespace co_async

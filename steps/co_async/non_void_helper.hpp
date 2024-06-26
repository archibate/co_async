#pragma once

#include <utility>

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
};

} // namespace co_async

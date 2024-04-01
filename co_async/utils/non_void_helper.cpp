export module co_async:utils.non_void_helper;

import std;

namespace co_async {

export template <class T = void>
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

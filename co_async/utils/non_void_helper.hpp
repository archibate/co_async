#pragma once

#include <co_async/std.hpp>

namespace co_async {

struct Void final {
    explicit Void() = default;

    template <class T>
    constexpr friend T &&operator,(T &&t, Void) {
        return std::forward<T>(t);
    }

    template <class T>
    constexpr friend T &&operator|(Void, T &&t) {
        return std::forward<T>(t);
    }

    constexpr friend void operator|(Void, Void) {
    }

    char const *repr() const noexcept {
        return "Void";
    }

    /* static void *operator new(std::size_t) noexcept { */
    /*     static char unused[sizeof(Void)]; */
    /*     return &unused; */
    /* } */
    /*  */
    /* static void operator delete(void *) noexcept {} */
};

template <class T = void>
struct AvoidVoidTrait {
    using Type = T;
};

template <>
struct AvoidVoidTrait<void> final {
    using Type = Void;
};

template <class T>
using Avoid = typename AvoidVoidTrait<T>::Type;

} // namespace co_async

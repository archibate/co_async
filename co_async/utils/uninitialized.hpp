#pragma once/*{export module co_async:utils.uninitialized;}*/

#include <cmake/clang_std_modules_source/std.hpp>/*{import std;}*/
#include <co_async/utils/non_void_helper.hpp>/*{import :utils.non_void_helper;}*/

namespace co_async {

template <class T>
struct Uninitialized {
    union {
        T mValue;
    };
#if CO_ASYNC_DEBUG
    bool mHasValue = false;
#endif

    Uninitialized() noexcept {}

    Uninitialized(Uninitialized &&) = delete;

    ~Uninitialized() noexcept {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            mValue.~T();
        }
#endif
    }

    T moveValue() {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::invalid_argument("Uninitialized::moveValue called in an unvalued slot");
        }
#endif
        T ret(std::move(mValue));
        mValue.~T();
#if CO_ASYNC_DEBUG
        mHasValue = false;
#endif
        return ret;
    }

    template <class... Ts>
    void putValue(Ts &&...args) {
        new (std::addressof(mValue)) T(std::forward<Ts>(args)...);
#if CO_ASYNC_DEBUG
        mHasValue = true;
#endif
    }
};

template <>
struct Uninitialized<void> {
    auto moveValue() {
        return NonVoidHelper<>{};
    }

    void putValue(NonVoidHelper<>) {}
};

template <class T>
struct Uninitialized<T const> : Uninitialized<T> {};

template <class T>
struct Uninitialized<T &> : Uninitialized<std::reference_wrapper<T>> {};

template <class T>
struct Uninitialized<T &&> : Uninitialized<T> {};

} // namespace co_async

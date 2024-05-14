#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/non_void_helper.hpp>

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

    ~Uninitialized() {
#if CO_ASYNC_DEBUG
        if (mHasValue) [[unlikely]] {
            throw std::logic_error("Uninitialized destroyed with value");
        }
#endif
    }

    T const &refValue() const noexcept {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::refValue called in an unvalued slot");
        }
#endif
        return mValue;
    }

    T &refValue() noexcept {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::refValue called in an unvalued slot");
        }
#endif
        return mValue;
    }

    void destroyValue() {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::destroyValue called in an unvalued slot");
        }
#endif
        mValue.~T();
#if CO_ASYNC_DEBUG
        mHasValue = false;
#endif
    }

    T moveValue() {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::moveValue called in an unvalued slot");
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
        requires std::constructible_from<T, Ts...>
    void putValue(Ts &&...args) {
#if CO_ASYNC_DEBUG
        if (mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::putValue with value already exist");
        }
#endif
        new (std::addressof(mValue)) T(std::forward<Ts>(args)...);
#if CO_ASYNC_DEBUG
        mHasValue = true;
#endif
    }
};

template <>
struct Uninitialized<void> {
    void refValue() const noexcept {}

    void destroyValue() {}

    Void moveValue() {
        return Void();
    }

    void putValue(Void) {}

    void putValue() {}
};

template <>
struct Uninitialized<Void> : Uninitialized<void> {};

template <class T>
struct Uninitialized<T const> : Uninitialized<T> {};

template <class T>
struct Uninitialized<T &> : Uninitialized<std::reference_wrapper<T>> {
private:
    using Base = Uninitialized<std::reference_wrapper<T>>;

public:
    T const &refValue() const noexcept {
        return Base::refValue().get();
    }

    T &refValue() noexcept {
        return Base::refValue().get();
    }

    T &moveValue() {
        return Base::moveValue().get();
    }
};

template <class T>
struct Uninitialized<T &&> : Uninitialized<T &> {
private:
    using Base = Uninitialized<T &>;

public:
    T &&moveValue() {
        return std::move(Base::moveValue().get());
    }
};
} // namespace co_async

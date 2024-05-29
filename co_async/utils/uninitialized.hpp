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

    T const &ref() const noexcept {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::ref called in an unvalued slot");
        }
#endif
        return mValue;
    }

    T &ref() noexcept {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::ref called in an unvalued slot");
        }
#endif
        return mValue;
    }

    void destroy() {
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

    T move() {
#if CO_ASYNC_DEBUG
        if (!mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::move called in an unvalued slot");
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
    void emplace(Ts &&...args) {
#if CO_ASYNC_DEBUG
        if (mHasValue) [[unlikely]] {
            throw std::logic_error(
                "Uninitialized::emplace with value already exist");
        }
#endif
        std::construct_at(std::addressof(mValue), std::forward<Ts>(args)...);
#if CO_ASYNC_DEBUG
        mHasValue = true;
#endif
    }
};

template <>
struct Uninitialized<void> {
    void ref() const noexcept {}

    void destroy() {}

    Void move() {
        return Void();
    }

    void emplace(Void) {}

    void emplace() {}
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
    T const &ref() const noexcept {
        return Base::ref().get();
    }

    T &ref() noexcept {
        return Base::ref().get();
    }

    T &move() {
        return Base::move().get();
    }
};

template <class T>
struct Uninitialized<T &&> : Uninitialized<T &> {
private:
    using Base = Uninitialized<T &>;

public:
    T &&move() {
        return std::move(Base::move().get());
    }
};
} // namespace co_async

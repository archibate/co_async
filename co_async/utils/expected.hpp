#pragma once // TODO: to be done! Not Implemented Yet

#include <concepts>
#include <system_error>
#include <co_async/uninitialized.hpp>

namespace co_async {

template <class T, class E = std::error_code>
struct Expected {
    E *mError;
    Uninitialized<T> mValue;

    template <class ...Args> requires std::constructible_from<T, Args>
    Expected(Args &&...args) noexcept {
        mError = nullptr;
        mValue.putValue(std::move(args));
    }

    Expected(T const &value) : mValue(value), mError(nullptr) {
    }

    Expected(E const &error) : mError(new E(error)) {
    }

    bool has_value() noexcept {
    }

    T &value() noexcept {
        throwIfHasError();
        return mValue;
    }

    T const &value() const noexcept {
        throwIfHasError();
        return mValue;
    }

    T &operator*() noexcept {
        return mValue;
    }

    T const &operator*() const noexcept {
        return mValue;
    }
};

}

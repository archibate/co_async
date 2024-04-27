#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {

template <class E>
struct ExpectedErrorTraits {
    void throws(E const &error) {
        throw error;
    }
};

template <>
struct ExpectedErrorTraits<std::exception_ptr> {
    void throws(std::exception_ptr const &error) {
        std::rethrow_exception(error);
    }
};

template <class E = void>
struct [[nodiscard]] Unexpected {
private:
    static_assert(!std::is_reference_v<E>);

    using Error = typename NonVoidHelper<E>::Type;

    Error *mErrorPtr;

    bool hasError() const noexcept {
        return mErrorPtr != nullptr;
    }

    void throwError() const {
        ExpectedErrorTraits<E>::throws(*mErrorPtr);
    }

    explicit Unexpected(std::nullptr_t) noexcept : mErrorPtr(nullptr) {}

public:
    explicit Unexpected(Error error) {
        mErrorPtr = new E(std::move(error));
    }

    explicit Unexpected()
        requires(std::same_as<void, E>)
    {
        mErrorPtr = new NonVoidHelper<E>;
    }

    template <class... Es>
        requires std::constructible_from<E, Es...>
    explicit Unexpected(Es &&...args) {
        mErrorPtr = new Error(std::forward<Es>(args)...);
    }

    Unexpected(Unexpected &&that) noexcept
        : mErrorPtr(std::exchange(that.mErrorPtr, nullptr)) {}

    Unexpected &operator=(Unexpected &&that) noexcept {
        if (&that != this) [[likely]] {
            if (mErrorPtr) {
                delete mErrorPtr;
                mErrorPtr = nullptr;
            }
            mErrorPtr = std::exchange(that.mErrorPtr, nullptr);
        }
        return *this;
    }

    ~Unexpected() {
        if (mErrorPtr) {
            delete mErrorPtr;
        }
    }

    template <class, class>
    friend struct Expected;
};

template <class E>
Unexpected(E) -> Unexpected<E>;

Unexpected() -> Unexpected<>;

template <class T = void, class E = void>
struct [[nodiscard]] Expected {
private:
    static_assert(!std::is_reference_v<E>);

    Unexpected<E> mError;
    Uninitialized<T> mValue;

public:
    Expected(typename NonVoidHelper<T>::Type value) : mError(nullptr) {
        mValue.putValue(std::forward<T>(value));
    }

    Expected()
        requires(std::same_as<void, T>)
    {
        mValue.putValue(NonVoidHelper<>());
    }

    Expected(Unexpected<E> error) noexcept : mError(std::move(error)) {}

    Expected(Expected &&that) noexcept : mError(std::move(that.mError)) {
        if (has_value()) {
            mValue.putValue(that.mValue.moveValue());
        }
    }

    Expected &operator=(Expected &&that) noexcept {
        if (&that != this) [[likely]] {
            mError = std::move(that.mError);
            if (has_value()) {
                mValue.putValue(that.mValue.moveValue());
            }
        }
    }

    ~Expected() {
        if (has_value()) {
            mValue.destroyValue();
        }
    }

    bool has_error() const noexcept {
        return mError.hasError();
    }

    bool has_value() const noexcept {
        return !mError.hasError();
    }

    operator bool() const noexcept {
        return has_value();
    }

    std::add_lvalue_reference_t<T> value() {
        if (mError) [[unlikely]] {
            mError->throwError();
        }
        return mValue.refValue();
    }

    T operator+() {
        if (mError) [[unlikely]] {
            mError->throwError();
        }
        return mValue.moveValue();
    }

    T operator*() {
#if CO_ASYNC_DEBUG
        if (!mError) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
        return mValue.refValue();
    }

    T *operator->() {
#if CO_ASYNC_DEBUG
        if (!mError) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator->() is called");
        }
#endif
        return mValue.refValue();
    }

    E error() {
#if CO_ASYNC_DEBUG
        if (!mError) [[unlikely]] {
            throw std::logic_error("Expected: no error but error() is called");
        }
#endif
        return *mError;
    }
};

template <class T>
Expected(T) -> Expected<T>;

Expected() -> Expected<>;

} // namespace co_async

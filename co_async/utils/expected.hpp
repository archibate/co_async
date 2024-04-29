#pragma once

#include <co_async/std.hpp>
#include <co_async/utils/uninitialized.hpp>

namespace co_async {

template <class E = void>
struct UnexpectedTraits {
    static void throw_error(E const &error) {
        throw error;
    }

    using inplace_storable = std::false_type;
};

template <>
struct UnexpectedTraits<bool> {
    static void throw_error(bool const &error) {
        throw std::runtime_error("unexpected boolean error");
    }

    using inplace_storable = std::true_type;
};

template <>
struct UnexpectedTraits<std::errc> {
    static void throw_error(std::errc const &error) {
        throw std::system_error(static_cast<int>(error), std::system_category());
    }

    using inplace_storable = std::true_type;
};

template <>
struct UnexpectedTraits<std::error_code> {
    static void throw_error(std::error_code const &error) {
        throw std::system_error(error);
    }

    using inplace_storable = std::true_type;
};

template <>
struct UnexpectedTraits<std::exception_ptr> {
    static void throw_error(std::exception_ptr const &error) {
        std::rethrow_exception(error);
    }

    using inplace_storable = std::true_type;
};

template <>
struct UnexpectedTraits<void> {
    static void throw_error() {
        throw std::runtime_error("unexpected error");
    }

    using inplace_storable = std::false_type;
};

template <class E = void>
struct [[nodiscard]] Unexpected {
private:
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>);

    E *mErrorPtr;

public:
    bool has_error() const noexcept {
        return mErrorPtr != nullptr;
    }

    void throw_error() const {
        UnexpectedTraits<E>::throw_error(*mErrorPtr);
    }

    E const &error() const noexcept {
        return *mErrorPtr;
    }

    explicit Unexpected(std::in_place_type_t<void>) noexcept : mErrorPtr(nullptr) {}

    explicit Unexpected(E error) {
        mErrorPtr = new E(std::move(error));
    }

    template <class... Es>
        requires std::constructible_from<E, Es...>
    explicit Unexpected(std::in_place_t, Es &&...args) {
        mErrorPtr = new E(std::forward<Es>(args)...);
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
};

template <>
struct [[nodiscard]] Unexpected<void> {
private:
    bool mHasError;

public:
    bool has_error() const noexcept {
        return mHasError;
    }

    void throw_error() const {
        UnexpectedTraits<void>::throw_error();
    }

    void error() const noexcept {
    }

    explicit Unexpected(std::in_place_type_t<void>) noexcept : mHasError(false) {}

    explicit Unexpected() {
        mHasError = true;
    }

    Unexpected(Unexpected &&that) noexcept
        : mHasError(std::exchange(that.mHasError, false)) {}

    Unexpected &operator=(Unexpected &&that) noexcept {
        if (&that != this) [[likely]] {
            if (mHasError) {
                mHasError = false;
            }
            mHasError = std::exchange(that.mHasError, false);
        }
        return *this;
    }

    ~Unexpected() = default;
};

template <class E> requires (UnexpectedTraits<E>::inplace_storable::value)
struct [[nodiscard]] Unexpected<E> {
private:
    E mError;

public:
    bool has_error() const noexcept {
        return (bool)mError;
    }

    void throw_error() const {
        UnexpectedTraits<void>::throw_error();
    }

    E const &error() const noexcept {
        return mError;
    }

    explicit Unexpected(std::in_place_type_t<void>) noexcept : mError() {}

    explicit Unexpected(E error) {
        mError = E(std::move(error));
    }

    template <class... Es>
        requires std::constructible_from<E, Es...>
    explicit Unexpected(std::in_place_t, Es &&...args) {
        mError = E(std::forward<Es>(args)...);
    }

    Unexpected(Unexpected &&that) noexcept
        : mError(std::exchange(that.mError, E())) {}

    Unexpected &operator=(Unexpected &&that) noexcept {
        if (&that != this) [[likely]] {
            if (mError) {
                mError = false;
            }
            mError = std::exchange(that.mError, E());
        }
        return *this;
    }

    ~Unexpected() = default;
};

template <class E>
Unexpected(E) -> Unexpected<E>;

Unexpected() -> Unexpected<>;

template <class T = void, class E = void>
struct [[nodiscard]] Expected {
private:
    static_assert(!std::is_reference_v<T> && !std::is_reference_v<E>);

    Unexpected<E> mError;
    Uninitialized<T> mValue;

public:
    Expected(T &&value) : mError(std::in_place_type<void>) {
        mValue.putValue(std::forward<T>(value));
    }

    Expected(Unexpected<E> error) noexcept : mError(std::move(error)) {}

    Expected(Expected &&that) noexcept : mError(std::move(that.mError)) {
        if (has_value()) {
            mValue.putValue(std::move(that.mValue.refValue()));
        }
    }

    Expected &operator=(Expected &&that) noexcept {
        if (&that != this) [[likely]] {
            mError = std::move(that.mError);
            if (has_value()) {
                mValue.putValue(std::move(that.mValue.refValue()));
            }
        }
    }

    ~Expected() {
        if (has_value()) {
            mValue.destroyValue();
        }
    }

    bool has_error() const noexcept {
        return mError.has_error();
    }

    bool has_value() const noexcept {
        return !mError.has_error();
    }

    operator bool() const noexcept {
        return has_value();
    }

    T &value() {
        if (mError.has_error()) [[unlikely]] {
            mError->throwError();
        }
        return mValue.refValue();
    }

    T const &value() const {
        if (mError.has_error()) [[unlikely]] {
            mError->throwError();
        }
        return mValue.refValue();
    }

    template <class ...Ts> requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) const & {
        if (mError.has_error()) {
            return T(std::forward<Ts>(ts)...);
        }
        return operator*();
    }

    template <class ...Ts> requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) && {
        if (mError.has_error()) {
            return T(std::forward<Ts>(ts)...);
        }
        return std::move(operator*());
    }

    T &&operator*() && {
#if CO_ASYNC_DEBUG
        if (mError.has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
        return std::move(mValue.refValue());
    }

    T &operator*() & {
#if CO_ASYNC_DEBUG
        if (mError.has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
        return mValue.refValue();
    }

    T const &operator*() const & {
#if CO_ASYNC_DEBUG
        if (mError.has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
        return mValue.refValue();
    }

    T *operator->() {
        return std::addressof(operator*());
    }

    T const *operator->() const {
        return std::addressof(operator*());
    }

    decltype(auto) error() const {
#if CO_ASYNC_DEBUG
        if (!mError.has_error()) [[unlikely]] {
            throw std::logic_error("Expected: no error but error() is called");
        }
#endif
        return *mError;
    }
};

template <class E>
struct [[nodiscard("did you forgot to co_await twice?")]] Expected<void, E> {
private:
    static_assert(!std::is_reference_v<E>);

    Unexpected<E> mError;

public:
    Expected() : mError(std::in_place_type<void>) {
    }

    Expected(Unexpected<E> error) noexcept : mError(std::move(error)) {}

    Expected(Expected &&that) noexcept : mError(std::move(that.mError)) {
    }

    Expected &operator=(Expected &&that) noexcept {
        if (&that != this) [[likely]] {
            mError = std::move(that.mError);
        }
    }

    ~Expected() = default;

    bool has_error() const noexcept {
        return mError.has_error();
    }

    bool has_value() const noexcept {
        return !mError.has_error();
    }

    operator bool() const noexcept {
        return has_value();
    }

    void value() const {
        if (mError.has_error()) [[unlikely]] {
            mError->throwError();
        }
    }

    void value_or() const {
    }

    void operator*() const {
#if CO_ASYNC_DEBUG
        if (mError.has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
    }

    decltype(auto) error() const {
#if CO_ASYNC_DEBUG
        if (!mError.has_error()) [[unlikely]] {
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

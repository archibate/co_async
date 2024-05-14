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
        throw std::system_error(static_cast<int>(error),
                                std::system_category());
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
        throw std::runtime_error("unexpected unknown error");
    }

    using inplace_storable = std::false_type;
};

template <class E = void>
struct [[nodiscard]] Unexpected {
private:
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>);
    std::optional<E> mErrorOpt;

public:
    bool has_error() const noexcept {
        return mErrorOpt != nullptr;
    }

    void throw_error() const {
        UnexpectedTraits<E>::throw_error(*mErrorOpt);
    }

    E const &error() const noexcept {
        return *mErrorOpt;
    }

    explicit Unexpected(std::in_place_type_t<void>) noexcept : mErrorOpt() {}

    explicit Unexpected(E error) {
        mErrorOpt.emplace(std::move(error));
    }

    template <class... Es>
        requires std::constructible_from<E, Es...>
    explicit Unexpected(std::in_place_t, Es &&...args) {
        mErrorOpt.emplace(std::forward<Es>(args)...);
    }

    std::optional<E> repr() const {
        if (has_error()) {
            return error();
        }
        return std::nullopt;
    }
};

template <>
struct [[nodiscard]] Unexpected<void> {
private:
    bool mHasError;
    template <class, class>
    friend struct Expected;

    explicit Unexpected(std::in_place_type_t<void>) noexcept
        : mHasError(false) {}

public:
    bool has_error() const noexcept {
        return mHasError;
    }

    void throw_error() const {
        UnexpectedTraits<void>::throw_error();
    }

    void error() const noexcept {}

    explicit Unexpected() noexcept : mHasError(true) {}

    std::optional<Void> repr() const {
        if (has_error()) {
            return Void();
        }
        return std::nullopt;
    }
};

template <class E>
    requires(UnexpectedTraits<E>::inplace_storable::value)
struct [[nodiscard]] Unexpected<E> {
private:
    E mError;
    template <class, class>
    friend struct Expected;

    explicit Unexpected(std::in_place_type_t<void>) noexcept : mError() {}

public:
    bool has_error() const noexcept {
        return (bool)mError;
    }

    void throw_error() const {
        UnexpectedTraits<E>::throw_error(mError);
    }

    E const &error() const noexcept {
        return mError;
    }

    explicit Unexpected(E error) {
        mError = E(std::move(error));
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error("Unexpected constructed with no error");
        }
#endif
    }

    template <class... Es>
        requires std::constructible_from<E, Es...>
    explicit Unexpected(std::in_place_t, Es &&...args) {
        mError = E(std::forward<Es>(args)...);
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error("Unexpected constructed with no error");
        }
#endif
    }

    std::optional<E> repr() const {
        if (has_error()) {
            return error();
        }
        return std::nullopt;
    }
};
template <class E>
Unexpected(E) -> Unexpected<E>;
Unexpected() -> Unexpected<>;

template <class T = void, class E = std::error_code>
struct [[nodiscard]] Expected {
private:
    static_assert(!std::is_reference_v<T> && !std::is_reference_v<E>);
    Unexpected<E> mError;
    Uninitialized<T> mValue;
    template <class, class>
    friend struct Expected;

public:
    template <std::convertible_to<T> U>
    Expected(U &&value) : mError(std::in_place_type<void>) {
        mValue.putValue(std::forward<U>(value));
    }

    Expected(T value) : mError(std::in_place_type<void>) {
        mValue.putValue(std::move(value));
    }

    Expected(Unexpected<E> error) noexcept : mError(std::move(error)) {}

    Expected(Expected &&that) noexcept : mError(that.mError) {
        if (has_value()) {
            mValue.putValue(std::move(that.mValue.refValue()));
        }
    }

    template <class U>
        requires(!std::same_as<T, U> &&
                 (std::convertible_to<U, T> || std::constructible_from<T, U>))
    explicit(!std::convertible_to<U, T>) Expected(Expected<U, E> that) noexcept
        : mError(that.mError) {
        if (has_value()) {
            mValue.putValue(std::move(that.mValue.refValue()));
        }
    }

    Expected &operator=(Expected &&that) noexcept {
        if (&that != this) [[likely]] {
            if (has_value()) {
                mValue.destroyValue();
            }
            mError = that.mError;
            if (has_value()) {
                mValue.putValue(std::move(that.mValue.refValue()));
            }
        }
        return *this;
    }

    ~Expected() {
        if (has_value()) {
            mValue.destroyValue();
        }
    }

    bool has_error() const noexcept {
        return mError.has_error();
    }

    template <std::equality_comparable_with<E> U>
        requires(!std::equality_comparable_with<U, T>)
    bool operator==(U &&e) const {
        return mError.has_error() && mError.error() == e;
    }

    template <std::equality_comparable_with<T> U>
    bool operator==(U &&v) const {
        return !mError.has_error() && mValue.refValue() == v;
    }

    bool has_value() const noexcept {
        return !mError.has_error();
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T &&value() && {
        if (mError.has_error()) [[unlikely]] {
            mError.throw_error();
        }
        return std::move(mValue.refValue());
    }

    T &value() & {
        if (mError.has_error()) [[unlikely]] {
            mError.throw_error();
        }
        return mValue.refValue();
    }

    T const &value() const & {
        if (mError.has_error()) [[unlikely]] {
            mError.throw_error();
        }
        return mValue.refValue();
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) const & {
        if (mError.has_error()) {
            return T(std::forward<Ts>(ts)...);
        }
        return operator*();
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
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

    std::add_lvalue_reference_t<std::add_const_t<E>> error() const {
#if CO_ASYNC_DEBUG
        if (!mError.has_error()) [[unlikely]] {
            throw std::logic_error("Expected: no error but error() is called");
        }
#endif
        return mError.error();
    }
#if CO_ASYNC_DEBUG
    std::variant<AvoidCRef<E>, AvoidCRef<T>> repr() const {
        if (has_error()) {
            return error();
        }
        return value();
    }
#endif
};

template <class E>
struct [[nodiscard("did you forgot to co_await twice?")]] Expected<void, E> {
private:
    static_assert(!std::is_reference_v<E>);
    Unexpected<E> mError;
    template <class, class>
    friend struct Expected;

public:
    Expected() : mError(std::in_place_type<void>) {}

    Expected(Void) : mError(std::in_place_type<void>) {}

    Expected(Unexpected<E> error) noexcept : mError(std::move(error)) {}

    Expected(Expected &&that) noexcept : mError(that.mError) {}

    template <class U>
        requires(!std::is_void_v<U>)
    Expected(Expected<U, E> that) noexcept : mError(that.mError) {}

    Expected &operator=(Expected &&that) noexcept {
        if (&that != this) [[likely]] {
            mError = that.mError;
        }
        return *this;
    }

    ~Expected() = default;

    bool has_error() const noexcept {
        return mError.has_error();
    }

    bool has_value() const noexcept {
        return !mError.has_error();
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    void value() const {
        if (mError.has_error()) [[unlikely]] {
            mError.throw_error();
        }
    }

    void value_or() const {}

    void operator*() const {
#if CO_ASYNC_DEBUG
        if (mError.has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected: has error but operator*() is called");
        }
#endif
    }

    std::add_lvalue_reference_t<std::add_const_t<E>> error() const {
#if CO_ASYNC_DEBUG
        if constexpr (!UnexpectedTraits<E>::inplace_storable::value) {
            if (!mError.has_error()) [[unlikely]] {
                throw std::logic_error(
                    "Expected: no error but error() is called");
            }
        }
#endif
        return mError.error();
    }
#if CO_ASYNC_DEBUG
    std::variant<AvoidCRef<E>, Void> repr() const {
        if (has_error()) {
            return error();
        }
        return Void();
    }
#endif
};
template <class T>
Expected(T) -> Expected<T>;
template <class T, class E>
Expected(Expected<T, E>) -> Expected<T, E>;
template <class E>
Expected(Unexpected<E>) -> Expected<void, E>;
Expected() -> Expected<>;
} // namespace co_async

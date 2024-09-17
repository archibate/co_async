#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {

template <class T, class U>
concept WeaklyEqComparable = requires(T const &t, U const &u) {
    { t == u } -> std::convertible_to<bool>;
    { t != u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
    { u != t } -> std::convertible_to<bool>;
};

template <class T = void>
struct [[nodiscard("Expected<T> return values must be handled, use co_await to "
                   "propagate")]] Expected {
protected:
    static_assert(!std::is_reference_v<T>);
    std::error_category const *mErrorCatgory;

    template <class>
    friend struct Expected;

    union {
        T mValue;
        int mErrorCode;
    };

#if CO_ASYNC_DEBUG
public:
    std::source_location mErrorLocation;
# define CO_ASYNC_EXPECTED_LOCATION \
     , std::source_location const &errLoc = std::source_location::current()
# define CO_ASYNC_EXPECTED_LOCATION_INIT , mErrorLocation(errLoc)
# define CO_ASYNC_EXPECTED_LOCATION_MESSAGE \
     , std::string(mErrorLocation.file_name()) + ":" + \
           std::to_string(mErrorLocation.line()) + ": " + \
           mErrorLocation.function_name()
# define CO_ASYNC_EXPECTED_LOCATION_COPY   , mErrorLocation(that.mErrorLocation)
# define CO_ASYNC_EXPECTED_LOCATION_ASSIGN mErrorLocation = that.mErrorLocation
# define CO_ASYNC_EXPECTED_LOCATION_ARG    , mErrorLocation
# define CO_ASYNC_ERROR_FORWARD(e)         {(e).error(), (e).mErrorLocation}
#else
# define CO_ASYNC_EXPECTED_LOCATION
# define CO_ASYNC_EXPECTED_LOCATION_INIT
# define CO_ASYNC_EXPECTED_LOCATION_MESSAGE
# define CO_ASYNC_EXPECTED_LOCATION_COPY
# define CO_ASYNC_EXPECTED_LOCATION_ASSIGN
# define CO_ASYNC_EXPECTED_LOCATION_ARG
# define CO_ASYNC_ERROR_FORWARD(e)         e.error()
#endif

public:
    template <std::convertible_to<T> U>
        requires(!std::convertible_to<U, std::error_code> &&
                 !std::convertible_to<U, std::errc> &&
                 !std::convertible_to<U, std::in_place_t>)
    Expected(U &&value) noexcept(std::is_nothrow_constructible_v<T, U>)
        : mErrorCatgory(nullptr),
          mValue(std::forward<U>(value)) {}

    template <class... Args>
        requires std::constructible_from<T, Args...>
    Expected(std::in_place_t, Args &&...args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
        : mErrorCatgory(nullptr),
          mValue(std::forward<Args>(args)...) {}

    Expected() noexcept(std::is_nothrow_default_constructible_v<T>)
        : mErrorCatgory(nullptr),
          mValue() {}

    Expected(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : mErrorCatgory(nullptr),
          mValue(std::move(value)) {}

    Expected(T const &value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : mErrorCatgory(nullptr),
          mValue(value) {}

    Expected(std::error_code const &ec CO_ASYNC_EXPECTED_LOCATION) noexcept
        : mErrorCatgory(&ec.category()),
          mErrorCode(ec.value()) CO_ASYNC_EXPECTED_LOCATION_INIT {}

    Expected(std::errc e CO_ASYNC_EXPECTED_LOCATION) noexcept
        : mErrorCatgory(&std::generic_category()),
          mErrorCode(static_cast<int>(e)) CO_ASYNC_EXPECTED_LOCATION_INIT {}

    ~Expected() noexcept(std::is_nothrow_destructible_v<T>) {
        if (!mErrorCatgory) {
            std::destroy_at(std::addressof(mValue));
        }
    }

    Expected(Expected &&that) noexcept(std::is_nothrow_move_constructible_v<T>)
        : mErrorCatgory(that.mErrorCatgory) CO_ASYNC_EXPECTED_LOCATION_COPY {
        if (!mErrorCatgory) {
            std::construct_at(std::addressof(mValue), std::move(that.mValue));
        } else {
            mErrorCode = that.mErrorCode;
        }
    }

    Expected(Expected const &that) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
        : mErrorCatgory(that.mErrorCatgory) CO_ASYNC_EXPECTED_LOCATION_COPY {
        if (!mErrorCatgory) {
            std::construct_at(std::addressof(mValue), that.mValue);
        } else {
            mErrorCode = that.mErrorCode;
        }
    }

    Expected &operator=(Expected &&that) noexcept(
        std::is_nothrow_destructible_v<T> &&
        std::is_nothrow_move_constructible_v<T>) {
        if (&that == this) [[unlikely]] {
            return *this;
        }
        CO_ASYNC_EXPECTED_LOCATION_ASSIGN;
        if (!mErrorCatgory) {
            std::destroy_at(std::addressof(mValue));
        }
        mErrorCatgory = nullptr;
        if (!that.mErrorCatgory) {
            std::construct_at(std::addressof(mValue), std::move(that.mValue));
        } else {
            mErrorCatgory = that.mErrorCatgory;
            mErrorCode = that.mErrorCode;
        }
        return *this;
    }

    Expected &operator=(Expected const &that) noexcept(
        std::is_nothrow_destructible_v<T> &&
        std::is_nothrow_copy_constructible_v<T>) {
        if (&that == this) [[unlikely]] {
            return *this;
        }
        CO_ASYNC_EXPECTED_LOCATION_ASSIGN;
        if (!mErrorCatgory) {
            std::destroy_at(std::addressof(mValue));
        }
        mErrorCatgory = nullptr;
        if (!that.mErrorCatgory) {
            std::construct_at(std::addressof(mValue), that.mValue);
        } else {
            mErrorCatgory = that.mErrorCatgory;
            mErrorCode = that.mErrorCode;
        }
        return *this;
    }

    template <class U>
        requires(!std::same_as<T, Void> && !std::same_as<T, U> &&
                 (std::convertible_to<U, T> || std::constructible_from<T, U>))
    explicit(!std::convertible_to<U, T>) Expected(Expected<U> that) noexcept(
        std::is_nothrow_constructible_v<T, U>)
        : mErrorCatgory(that.mErrorCatgory) CO_ASYNC_EXPECTED_LOCATION_COPY {
        if (!mErrorCatgory) {
            std::construct_at(std::addressof(mValue), std::move(that.mValue));
        } else {
            mErrorCode = that.mErrorCode;
        }
    }

    template <class U>
        requires(std::same_as<T, Void>)
    Expected(Expected<U> that) noexcept
        : mErrorCatgory(that.mErrorCatgory) CO_ASYNC_EXPECTED_LOCATION_COPY {
        if (!mErrorCatgory) {
            std::construct_at(std::addressof(mValue));
        } else {
            mErrorCode = that.mErrorCode;
        }
    }

    bool has_error() const noexcept {
        return mErrorCatgory;
    }

    bool has_value() const noexcept {
        return !mErrorCatgory;
    }

    template <WeaklyEqComparable<std::error_code> U>
        requires(!std::equality_comparable_with<U, T>)
    bool operator==(U &&e) const {
        return has_error() && std::error_code(mErrorCode, *mErrorCatgory) ==
                                  std::forward<U>(e);
    }

    template <std::equality_comparable_with<T> U>
        requires(!WeaklyEqComparable<std::error_code, T>)
    bool operator==(U &&v) const {
        return has_value() && mValue == std::forward<U>(v);
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T &&value() && {
        if (has_value()) [[likely]] {
            return std::move(mValue);
        }
        throw std::system_error(std::error_code(mErrorCode, *mErrorCatgory)
                                    CO_ASYNC_EXPECTED_LOCATION_MESSAGE);
    }

    T &value() & {
        if (has_value()) [[likely]] {
            return mValue;
        }
        throw std::system_error(std::error_code(mErrorCode, *mErrorCatgory)
                                    CO_ASYNC_EXPECTED_LOCATION_MESSAGE);
    }

    T const &value() const & {
        if (has_value()) [[likely]] {
            return mValue;
        }
        throw std::system_error(std::error_code(mErrorCode, *mErrorCatgory)
                                    CO_ASYNC_EXPECTED_LOCATION_MESSAGE);
    }

    T const &&value() const && {
        if (has_value()) [[likely]] {
            return std::move(mValue);
        }
        throw std::system_error(std::error_code(mErrorCode, *mErrorCatgory)
                                    CO_ASYNC_EXPECTED_LOCATION_MESSAGE);
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) const & {
        if (has_value()) [[likely]] {
            return mValue;
        }
        return T(std::forward<Ts>(ts)...);
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) && {
        if (has_value()) [[likely]] {
            return mValue;
        }
        return T(std::forward<Ts>(ts)...);
    }

    T &&operator*() && {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return std::move(mValue);
    }

    T &operator*() & {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return mValue;
    }

    T const &operator*() const & {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return mValue;
    }

    T const &&operator*() const && {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return std::move(mValue);
    }

    T *operator->() {
        return std::addressof(operator*());
    }

    T const *operator->() const {
        return std::addressof(operator*());
    }

    std::error_code error() const {
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has no error but error() is called");
        }
#endif
        return std::error_code(mErrorCode, *mErrorCatgory);
    }

    std::optional<T> opt_value() const & {
        if (has_value()) {
            return mValue;
        }
        return std::nullopt;
    }

    std::optional<T> opt_value() && {
        if (has_value()) {
            return std::move(mValue);
        }
        return std::nullopt;
    }

    std::optional<std::error_code> opt_error() const {
        if (has_error()) {
            return std::error_code(mErrorCode, *mErrorCatgory);
        }
        return std::nullopt;
    }

    static_assert(WeaklyEqComparable<std::error_condition, std::error_code>);

    template <WeaklyEqComparable<std::error_code> U, class... Ts>
        requires std::constructible_from<T, Ts...>
    Expected ignore_error(U &&e, Ts &&...ts) && {
        if (has_error()) {
            if (std::error_code(mErrorCode, *mErrorCatgory) == e) [[likely]] {
                return T(std::forward<Ts>(ts)...);
            }
        }
        return std::move(*this);
    }

    template <WeaklyEqComparable<std::error_code> U, class... Ts>
        requires std::constructible_from<T, Ts...>
    Expected ignore_error(U &&e, Ts &&...ts) const & {
        if (has_error()) {
            if (std::error_code(mErrorCode, *mErrorCatgory) == e) [[likely]] {
                return T(std::forward<Ts>(ts)...);
            }
        }
        return *this;
    }

    template <std::invocable F>
    Expected or_else(F &&f) && {
        if (has_error()) {
            return f();
        }
        return std::move(*this);
    }

    template <std::invocable F>
    Expected or_else(F &&f) const & {
        if (has_error()) {
            return f(), Void();
        }
        return *this;
    }

    template <WeaklyEqComparable<std::error_code> U, std::invocable F>
    Expected or_else(U &&e, F &&f) && {
        if (has_error()) {
            if (std::error_code(mErrorCode, *mErrorCatgory) == e) [[likely]] {
                return f(), Void();
            }
        }
        return std::move(*this);
    }

    template <WeaklyEqComparable<std::error_code> U, std::invocable F>
    Expected or_else(U &&e, F &&f) const & {
        if (has_error()) {
            if (std::error_code(mErrorCode, *mErrorCatgory) == e) [[likely]] {
                return f(), Void();
            }
        }
        return *this;
    }

    template <std::invocable<T &&> F>
    Expected<std::invoke_result_t<F, T &&>> transform(F &&f) && {
        if (has_value()) {
            return f(std::move(mValue)), Void();
        }
        return {error() CO_ASYNC_EXPECTED_LOCATION_ARG};
    }

    template <std::invocable<T &> F>
    Expected<std::invoke_result_t<F, T &>> transform(F &&f) & {
        if (has_value()) {
            return f(mValue), Void();
        }
        return {error() CO_ASYNC_EXPECTED_LOCATION_ARG};
    }

    template <std::invocable<T const &> F>
    Expected<std::invoke_result_t<F, T const &>> transform(F &&f) const & {
        if (has_value()) {
            return f(mValue), Void();
        }
        return {error() CO_ASYNC_EXPECTED_LOCATION_ARG};
    }

    template <std::invocable F> requires std::same_as<T, Void>
    Expected<std::invoke_result_t<F>> transform(F &&f) const {
        if (has_value()) {
            return f(), Void();
        }
        return {error() CO_ASYNC_EXPECTED_LOCATION_ARG};
    }

    template <std::invocable<T &&> F>
    Avoid<std::invoke_result_t<F, T &&>> and_then(F &&f) && {
        if (has_value()) {
            return f(std::move(mValue)), Void();
        }
        return std::move(*this);
    }

    template <std::invocable<T &> F>
    Avoid<std::invoke_result_t<F, T &>> and_then(F &&f) & {
        if (has_value()) {
            return f(mValue), Void();
        }
        return *this;
    }

    template <std::invocable F>
    Avoid<std::invoke_result_t<F, T const &>> and_then(F &&f) const & {
        if (has_value()) {
            return f(mValue), Void();
        }
        return *this;
    }

    std::variant<std::reference_wrapper<T const>, std::error_code>
    repr() const {
        if (has_error()) {
            return std::error_code(mErrorCode, *mErrorCatgory);
        } else {
            return std::cref(mValue);
        }
    }
};

template <>
struct [[nodiscard("Expected<T> return values must be handled, use co_await to "
                   "propagate")]] Expected<void> : Expected<Void> {
    using Expected<Void>::Expected;

    Expected(Expected<Void> const &that) noexcept : Expected<Void>(that) {}
    Expected(Expected<Void> &&that) noexcept : Expected<Void>(std::move(that)) {}
};

template <class T>
Expected(T) -> Expected<T>;
Expected() -> Expected<>;

#undef CO_ASYNC_EXPECTED_LOCATION
#undef CO_ASYNC_EXPECTED_LOCATION_INIT
#undef CO_ASYNC_EXPECTED_LOCATION_MESSAGE
#undef CO_ASYNC_EXPECTED_LOCATION_COPY
#undef CO_ASYNC_EXPECTED_LOCATION_ASSIGN
#undef CO_ASYNC_EXPECTED_LOCATION_ARG
} // namespace co_async

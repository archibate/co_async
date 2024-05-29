#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {
template <class T = void>
struct [[nodiscard]] Expected {
protected:
    static_assert(!std::is_reference_v<T>);
    std::variant<T, std::error_code> mValue;

public:
    template <std::convertible_to<T> U>
    Expected(U &&value)
        : mValue(std::in_place_index<0>, std::forward<U>(value)) {}

    template <class... Args> requires std::constructible_from<T, Args...>
    Expected(std::in_place_t, Args &&...args)
        : mValue(std::in_place_index<0>, std::forward<Args>(args)...) {}

    Expected() : mValue(std::in_place_index<0>) {}

    Expected(T &&value)
        : mValue(std::in_place_index<0>, std::move(value)) {}

    Expected(T const &value) : mValue(std::in_place_index<0>, value) {}

    Expected(std::error_code ec)
        : mValue(std::in_place_index<1>, std::move(ec)) {}

    Expected(std::errc e)
        : mValue(std::in_place_index<1>, std::make_error_code(e)) {}

    template <class U>
        requires(!std::same_as<T, U> && (std::convertible_to<U, T> || std::constructible_from<T, U>))
    explicit(!std::convertible_to<U, T>) Expected(Expected<U> that) {
        if (that.has_value()) {
            mValue.template emplace<0>(std::move(that).value());
        } else {
            mValue.template emplace<1>(std::move(that).error());
        }
    }

    bool has_error() const noexcept {
        return mValue.index() != 0;
    }

    bool has_value() const noexcept {
        return mValue.index() == 0;
    }

    template <std::equality_comparable_with<std::error_code> U>
        requires(!std::equality_comparable_with<U, T>)
    bool operator==(U &&e) const {
        auto p = std::get_if<1>(&mValue);
        return p && *p == std::forward<U>(e);
    }

    template <std::equality_comparable_with<T> U>
        requires(!std::equality_comparable_with<std::error_code, T>)
    bool operator==(U &&e) const {
        auto p = std::get_if<0>(&mValue);
        return p && *p == std::forward<U>(e);
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T &&value() && {
        if (auto p = std::get_if<0>(&mValue)) [[likely]] {
            return std::move(*p);
        }
        throw std::system_error(*std::get_if<1>(&mValue));
    }

    T &value() & {
        if (auto p = std::get_if<0>(&mValue)) [[likely]] {
            return *p;
        }
        throw std::system_error(*std::get_if<1>(&mValue));
    }

    T const &value() const & {
        if (auto p = std::get_if<0>(&mValue)) [[likely]] {
            return *p;
        }
        throw std::system_error(*std::get_if<1>(&mValue));
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) const & {
        if (auto p = std::get_if<0>(&mValue)) [[likely]] {
            return *p;
        }
        return T(std::forward<Ts>(ts)...);
    }

    template <class... Ts>
        requires std::constructible_from<T, Ts...>
    T value_or(Ts &&...ts) && {
        if (auto p = std::get_if<0>(&mValue)) [[likely]] {
            return std::move(*p);
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
        return std::move(*std::get_if<0>(&mValue));
    }

    T &operator*() & {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return *std::get_if<0>(&mValue);
    }

    T const &operator*() const & {
#if CO_ASYNC_DEBUG
        if (has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has error but operator*() is called");
        }
#endif
        return *std::get_if<0>(&mValue);
    }

    T *operator->() {
        return std::addressof(operator*());
    }

    T const *operator->() const {
        return std::addressof(operator*());
    }

    std::error_code const &error() const & {
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has no error but error() is called");
        }
#endif
        return *std::get_if<1>(&mValue);
    }

    std::error_code &error() & {
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has no error but error() is called");
        }
#endif
        return *std::get_if<1>(&mValue);
    }

    std::error_code &&error() && {
#if CO_ASYNC_DEBUG
        if (!has_error()) [[unlikely]] {
            throw std::logic_error(
                "Expected has no error but error() is called");
        }
#endif
        return std::move(*std::get_if<1>(&mValue));
    }

    std::optional<T> opt_value() const & {
        if (auto p = std::get_if<0>(&mValue)) {
            return *p;
        }

        return std::nullopt;
    }

    std::optional<T> opt_value() && {
        if (auto p = std::get_if<0>(&mValue)) {
            return std::move(*p);
        }
        return std::nullopt;
    }

    std::optional<std::error_code> opt_error() const & {
        if (auto p = std::get_if<1>(&mValue)) {
            return *p;
        }
        return std::nullopt;
    }

    std::optional<std::error_code> opt_error() && {
        if (auto p = std::get_if<1>(&mValue)) {
            return std::move(*p);
        }
        return std::nullopt;
    }

#if DEBUG_LEVEL
    auto const &repr() const {
        return mValue;
    }
#endif
};

template <>
struct [[nodiscard]] Expected<void> : Expected<Void> {
    using Expected<Void>::Expected;

    template <class U>
        requires(!std::same_as<void, U>)
    Expected(Expected<U> that) {
        if (that.has_value()) {
            mValue.emplace<0>();
        } else {
            mValue.emplace<1>(std::move(that).error());
        }
    }
};

template <class T>
Expected(T) -> Expected<T>;
Expected() -> Expected<>;

} // namespace co_async

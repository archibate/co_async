#pragma once
#include <co_async/std.hpp>
#include <co_async/utils/non_void_helper.hpp>

namespace co_async {
/* template <class T, class Final = void> */
/* struct Generative { */
/*     explicit operator bool() const noexcept { */
/*     } */
/* }; */

template <class T, class E = void>
struct GeneratorResult {
    std::variant<T, E> mValue;

    explicit GeneratorResult(std::in_place_index_t<0>, auto &&...args)
        : mValue(std::in_place_index<0>,
                 std::forward<decltype(args)>(args)...) {}

    explicit GeneratorResult(std::in_place_index_t<1>, auto &&...args)
        : mValue(std::in_place_index<1>,
                 std::forward<decltype(args)>(args)...) {}

    /* GeneratorResult(std::convertible_to<T> auto &&value) */

    bool has_result() const noexcept {
        return mValue.index() == 1;
    }

    bool has_value() const noexcept {
        return mValue.index() == 0;
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T &operator*() & noexcept {
        return *std::get_if<0>(&mValue);
    }

    T &&operator*() && noexcept {
        return std::move(*std::get_if<0>(&mValue));
    }

    T const &operator*() const & noexcept {
        return *std::get_if<0>(&mValue);
    }

    T const &&operator*() const && noexcept {
        return std::move(*std::get_if<0>(&mValue));
    }

    T &operator->() noexcept {
        return std::get_if<0>(&mValue);
    }

    T &value() & {
        return std::get<0>(mValue);
    }

    T &&value() && {
        return std::move(std::get<0>(mValue));
    }

    T const &value() const & {
        return std::get<0>(mValue);
    }

    T const &&value() const && {
        return std::move(std::get<0>(mValue));
    }

    E &result_unsafe() & noexcept {
        return *std::get_if<1>(&mValue);
    }

    E &&result_unsafe() && noexcept {
        return std::move(*std::get_if<1>(&mValue));
    }

    E const &result_unsafe() const & noexcept {
        return *std::get_if<1>(&mValue);
    }

    E const &&result_unsafe() const && noexcept {
        return std::move(*std::get_if<1>(&mValue));
    }

    E &result() & {
        return std::get<1>(mValue);
    }

    E &&result() && {
        return std::move(std::get<1>(mValue));
    }

    E const &result() const & {
        return std::get<1>(mValue);
    }

    E const &&result() const && {
        return std::move(std::get<1>(mValue));
    }
};

template <class T>
struct GeneratorResult<T, void> : GeneratorResult<T, Void> {
    using GeneratorResult<T, Void>::GeneratorResult;
};
} // namespace co_async

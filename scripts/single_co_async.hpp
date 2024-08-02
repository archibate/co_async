#pragma once

#include <algorithm>                     // IWYU pragma: export
#include <any>                           // IWYU pragma: export
#include <array>                         // IWYU pragma: export
#include <atomic>                        // IWYU pragma: export
#include <bit>                           // IWYU pragma: export
#include <bitset>                        // IWYU pragma: export
#include <cassert>                       // IWYU pragma: export
#include <cctype>                        // IWYU pragma: export
#include <cerrno>                        // IWYU pragma: export
#include <cfloat>                        // IWYU pragma: export
#include <charconv>                      // IWYU pragma: export
#include <chrono>                        // IWYU pragma: export
#include <cinttypes>                     // IWYU pragma: export
#include <climits>                       // IWYU pragma: export
#include <clocale>                       // IWYU pragma: export
#include <cmath>                         // IWYU pragma: export
#include <codecvt>                       // IWYU pragma: export
#include <compare>                       // IWYU pragma: export
#include <concepts>                      // IWYU pragma: export
#include <condition_variable>            // IWYU pragma: export
#include <coroutine>                     // IWYU pragma: export
#include <csignal>                       // IWYU pragma: export
#include <cstddef>                       // IWYU pragma: export
#include <cstdint>                       // IWYU pragma: export
#include <cstdio>                        // IWYU pragma: export
#include <cstdlib>                       // IWYU pragma: export
#include <cstring>                       // IWYU pragma: export
#include <ctime>                         // IWYU pragma: export
#include <cuchar>                        // IWYU pragma: export
#include <cwchar>                        // IWYU pragma: export
#include <cwctype>                       // IWYU pragma: export
#include <deque>                         // IWYU pragma: export
#include <exception>                     // IWYU pragma: export
#include <filesystem>                    // IWYU pragma: export
#include <forward_list>                  // IWYU pragma: export
#include <fstream>                       // IWYU pragma: export
#include <functional>                    // IWYU pragma: export
#include <initializer_list>              // IWYU pragma: export
#include <iomanip>                       // IWYU pragma: export
#include <ios>                           // IWYU pragma: export
#include <iostream>                      // IWYU pragma: export
#include <istream>                       // IWYU pragma: export
#include <iterator>                      // IWYU pragma: export
#include <limits>                        // IWYU pragma: export
#include <list>                          // IWYU pragma: export
#include <locale>                        // IWYU pragma: export
#include <map>                           // IWYU pragma: export
#include <memory>                        // IWYU pragma: export
#include <memory_resource>               // IWYU pragma: export
#include <mutex>                         // IWYU pragma: export
#include <new>                           // IWYU pragma: export
#include <numeric>                       // IWYU pragma: export
#include <optional>                      // IWYU pragma: export
#include <ostream>                       // IWYU pragma: export
#include <random>                        // IWYU pragma: export
#include <set>                           // IWYU pragma: export
#include <span>                          // IWYU pragma: export
#include <sstream>                       // IWYU pragma: export
#include <stdexcept>                     // IWYU pragma: export
#include <stop_token>                    // IWYU pragma: export
#include <string>                        // IWYU pragma: export
#include <string_view>                   // IWYU pragma: export
#include <system_error>                  // IWYU pragma: export
#include <thread>                        // IWYU pragma: export
#include <tuple>                         // IWYU pragma: export
#include <type_traits>                   // IWYU pragma: export
#include <typeindex>                     // IWYU pragma: export
#include <typeinfo>                      // IWYU pragma: export
#include <unordered_map>                 // IWYU pragma: export
#include <unordered_set>                 // IWYU pragma: export
#include <utility>                       // IWYU pragma: export
#include <variant>                       // IWYU pragma: export
#include <vector>                        // IWYU pragma: export
#include <version>                       // IWYU pragma: export
#if __has_include(<source_location>)
# include <source_location>              // IWYU pragma: export
#elif __has_include(<experimental/source_location>)
# include <experimental/source_location> // IWYU pragma: export
# if __cpp_lib_experimental_source_location
namespace std {
using experimental::source_location;
} // namespace std
# endif
#endif




namespace co_async {
struct Void final {
    explicit Void() = default;

    /* template <class T> */
    /* Void(T &&) noexcept {} */

    template <class T>
    friend constexpr T &&operator,(T &&t, Void) {
        return std::forward<T>(t);
    }

    template <class T>
    friend constexpr T &&operator|(Void, T &&t) {
        return std::forward<T>(t);
    }

    friend constexpr void operator|(Void, Void) {}

    char const *repr() const noexcept {
        return "void";
    }
};

template <class T = void>
struct AvoidVoidTrait {
    using Type = T;
    using RefType = std::reference_wrapper<T>;
    using CRefType = std::reference_wrapper<T const>;
};

template <>
struct AvoidVoidTrait<void> {
    using Type = Void;
    using RefType = Void;
    using CRefType = Void;
};
template <class T>
using Avoid = typename AvoidVoidTrait<T>::Type;
template <class T>
using AvoidRef = typename AvoidVoidTrait<T>::RefType;
template <class T>
using AvoidCRef = typename AvoidVoidTrait<T>::CRefType;
} // namespace co_async





namespace co_async {

template <class A>
concept Awaiter = requires(A a, std::coroutine_handle<> h) {
    { a.await_ready() };
    { a.await_suspend(h) };
    { a.await_resume() };
};
template <class A>
concept Awaitable = Awaiter<A> || requires(A a) {
    { a.operator co_await() } -> Awaiter;
};

template <class A>
struct AwaitableTraits {
    using Type = A;
};

template <Awaiter A>
struct AwaitableTraits<A> {
    using RetType = decltype(std::declval<A>().await_resume());
    using AvoidRetType = Avoid<RetType>;
    using Type = RetType;
    using AwaiterType = A;
};

template <class A>
    requires(!Awaiter<A> && Awaitable<A>)
struct AwaitableTraits<A>
    : AwaitableTraits<decltype(std::declval<A>().operator co_await())> {};

template <class... Ts>
struct TypeList {};

template <class Last>
struct TypeList<Last> {
    using FirstType = Last;
    using LastType = Last;
};

template <class First, class... Ts>
struct TypeList<First, Ts...> {
    using FirstType = First;
    using LastType = typename TypeList<Ts...>::LastType;
};

} // namespace co_async




namespace co_async {
struct PreviousAwaiter {
    std::coroutine_handle<> mPrevious;

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    void await_resume() const noexcept {}
};
} // namespace co_async





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





namespace co_async {
template <class T = void>
struct ValueAwaiter {
private:
    Avoid<T> mValue;

public:
    ValueAwaiter(Avoid<T> value) : mValue(std::move(value)) {}

    bool await_ready() const noexcept {
        return true;
    }

    void await_suspend(std::coroutine_handle<>) const noexcept {}

    T await_resume() {
        if constexpr (!std::is_void_v<T>) {
            return std::move(mValue);
        }
    }
};

template <class T = void>
struct ValueOrReturnAwaiter {
private:
    std::coroutine_handle<> mPrevious;
    Uninitialized<T> mValue;

public:
    template <class... Args>
    ValueOrReturnAwaiter(std::in_place_t, Args &&...args) : mPrevious() {
        mValue.emplace(std::forward<Args>(args)...);
    }

    ValueOrReturnAwaiter(std::coroutine_handle<> previous)
        : mPrevious(previous) {}

    bool await_ready() const noexcept {
        return !mPrevious;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    T await_resume() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return mValue.move();
        }
    }
};
} // namespace co_async




namespace co_async {
#if CO_ASYNC_ALLOC
using String = std::pmr::string;
#else
using String = std::string;
#endif

inline String operator""_s(char const *str, size_t len) {
    return String(str, len);
}

extern thread_local std::pmr::memory_resource *currentAllocator;

struct ReplaceAllocator {
    ReplaceAllocator(std::pmr::memory_resource *allocator) {
        lastAllocator = currentAllocator;
        currentAllocator = allocator;
    }

    ReplaceAllocator(ReplaceAllocator &&) = delete;

    ~ReplaceAllocator() {
        currentAllocator = lastAllocator;
    }

private:
    std::pmr::memory_resource *lastAllocator;
};

} // namespace co_async





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


#if CO_ASYNC_PERF


namespace co_async {
struct Perf {
private:
    char const *file;
    std::uint_least32_t line;
    std::chrono::steady_clock::time_point t0;

    struct PerfTableEntry {
        std::uint64_t duration;
        char const *file;
        int line;
    };

    struct PerfThreadLocal {
        std::deque<PerfTableEntry> table;
        PerfThreadLocal() = default;
        PerfThreadLocal(PerfThreadLocal &&) = delete;

        ~PerfThreadLocal() {
            gather(*this);
        }
    };

    struct PerfGather {
        /* PerfGather() { */
        /*     signal( */
        /*         SIGINT, +[](int signo) { std::exit(130); }); */
        /* } */

        PerfGather &operator=(PerfGather &&) = delete;

        void dump() const {
            if (table.empty()) {
                return;
            }

            struct PairLess {
                bool
                operator()(std::pair<std::string_view, int> const &a,
                           std::pair<std::string_view, int> const &b) const {
                    return std::tie(a.first, a.second) <
                           std::tie(b.first, b.second);
                }
            };

            struct Entry {
                std::uint64_t min = std::numeric_limits<std::uint64_t>::max();
                std::uint64_t sum = 0;
                std::uint64_t max = 0;
                std::uint64_t nr = 0;

                Entry &operator+=(std::uint64_t d) {
                    min = std::min(min, d);
                    sum += d;
                    max = std::max(max, d);
                    ++nr;
                    return *this;
                }
            };

            std::map<std::pair<std::string_view, int>, Entry, PairLess> m;
            for (auto const &e: table) {
                m[{e.file, e.line}] += e.duration;
            }
            auto t = [](std::uint64_t d) -> std::string {
                if (d < 10000) {
                    return std::format("{}ns", d);
                } else if (d < 10'000'000) {
                    return std::format("{}us", d / 1000);
                } else if (d < 10'000'000'000) {
                    return std::format("{}ms", d / 1'000'000);
                } else if (d < 10'000'000'000'000) {
                    return std::format("{}s", d / 1'000'000'000);
                } else {
                    return std::format("{}h", d / 3'600'000'000'000);
                }
            };
            auto p = [](std::string_view s) -> std::string {
                auto p = s.rfind('/');
                if (p == std::string_view::npos) {
                    return std::string(s);
                } else {
                    return std::string(s.substr(p + 1));
                }
            };
            std::vector<std::pair<std::pair<std::string_view, int>, Entry>>
                sorted(m.begin(), m.end());
            std::sort(sorted.begin(), sorted.end(),
                      [](auto const &lhs, auto const &rhs) {
                          return lhs.second.sum > rhs.second.sum;
                      });
            std::size_t w = 0, nw = 1;
            for (auto const &[loc, e]: sorted) {
                w = std::max(w, p(loc.first).size());
                nw = std::max(nw, std::to_string(e.nr).size());
            }
            std::string o;
            auto oit = std::back_inserter(o);
            std::format_to(oit, "{:>{}}:{:<4} {:^6} {:^6} {:^6} {:^6} {:^{}}\n",
                           "file", w, "line", "min", "avg", "max", "sum", "nr",
                           nw + 1);
            for (auto const &[loc, e]: sorted) {
                std::format_to(oit,
                               "{:>{}}:{:<4} {:>6} {:>6} {:>6} {:>6} {:>{}}x\n",
                               p(loc.first), w, loc.second, t(e.min),
                               t(e.sum / e.nr), t(e.max), t(e.sum), e.nr, nw);
            }
            fprintf(stderr, "%s", o.c_str());
        }

        ~PerfGather() {
            for (auto *thread: threads) {
                gather(*thread);
            }
            dump();
        }

        std::deque<PerfTableEntry> table;
        std::set<PerfThreadLocal *> threads;
        std::mutex lock;
    };

    static inline PerfGather gathered;
    static inline thread_local PerfThreadLocal perthread;

    static void gather(PerfThreadLocal &perthread) {
        std::lock_guard guard(gathered.lock);
        gathered.table.insert(gathered.table.end(), perthread.table.begin(),
                              perthread.table.end());
        gathered.threads.erase(&perthread);
    }

public:
    Perf(std::source_location loc = std::source_location::current())
        : file(loc.file_name()),
          line(loc.line()),
          t0(std::chrono::steady_clock::now()) {}

    Perf(Perf &&) = delete;

    ~Perf() {
        auto t1 = std::chrono::steady_clock::now();
        auto duration = (t1 - t0).count();
        perthread.table.emplace_back(duration, file, line);
    }
};
} // namespace co_async
#else
namespace co_async {
struct Perf {};
} // namespace co_async
#endif





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

    Expected<void>(Expected<Void> const &that) noexcept : Expected<Void>(that) {}
    Expected<void>(Expected<Void> &&that) noexcept : Expected<Void>(std::move(that)) {}
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




#if CO_ASYNC_PERF

#endif







namespace co_async {

#if CO_ASYNC_DEBUG
inline struct CoroDb {
    std::unordered_set<void *> mCoros;
} coroDb;
#endif

#if CO_ASYNC_ALLOC
struct TaskAwaiterAllocState {
    std::pmr::memory_resource *mLastAllocator;

    void push() noexcept {
        mLastAllocator = currentAllocator;
    }

    void pop() noexcept {
        currentAllocator = mLastAllocator;
    }
};
#endif

template <class T>
struct TaskAwaiter {
    bool await_ready() const noexcept {
#if CO_ASYNC_SAFERET
# if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            return true;
        }
# endif
        return mResult.has_value();
#else
        return false;
#endif
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) noexcept {
#if CO_ASYNC_ALLOC
        mAllocState.push();
#endif
        mCallerCoroutine = coroutine;
        return mCalleeCoroutine;
    }

    T await_resume() {
#if CO_ASYNC_ALLOC
        mAllocState.pop();
#endif
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        if constexpr (!std::is_void_v<T>) {
#if CO_ASYNC_SAFERET
# if CO_ASYNC_DEBUG
            if constexpr (std::same_as<Expected<>, T>) {}
            return std::move(mResult.value());
# else
            return std::move(*mResult);
# endif
#else
            return mResult.move();
#endif
        }
    }

    template <class P>
    explicit TaskAwaiter(std::coroutine_handle<P> coroutine)
        : mCalleeCoroutine(coroutine) {
        coroutine.promise().mAwaiter = this;
    }

    TaskAwaiter(TaskAwaiter &&that) = delete;

#if CO_ASYNC_DEBUG
    template <class U, class Loc>
    void returnValue(U &&result, Loc &&loc) {
        mResult.emplace(std::forward<U>(result), std::forward<Loc>(loc));
    }
#endif
    template <class U>
    void returnValue(U &&result) {
        mResult.emplace(std::forward<U>(result));
    }

    void returnVoid() {
        mResult.emplace();
    }

    void unhandledException() noexcept {
#if CO_ASYNC_EXCEPT
        mException = std::current_exception();
#else
        std::terminate();
#endif
    }

    std::coroutine_handle<> callerCoroutine() const noexcept {
        return mCallerCoroutine;
    }

protected:
    std::coroutine_handle<> mCallerCoroutine;
    std::coroutine_handle<> mCalleeCoroutine;
#if CO_ASYNC_SAFERET
    std::optional<Avoid<T>> mResult;
#else
    Uninitialized<Avoid<T>> mResult;
#endif
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
#if CO_ASYNC_ALLOC
    TaskAwaiterAllocState mAllocState;
#endif
};

struct TaskFinalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    template <class P>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<P> coroutine) const noexcept {
        return coroutine.promise().mAwaiter->callerCoroutine();
    }

    void await_resume() const noexcept {}
};

struct TaskYieldAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    template <class P>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<P> coroutine) const noexcept {
        return coroutine.promise().mAwaiter->callerCoroutine();
    }

    void await_resume() const noexcept {}
};

template <class T, class E>
struct TaskAwaiter<GeneratorResult<T, E>> {
    bool await_ready() const noexcept {
#if CO_ASYNC_SAFERET
# if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            return true;
        }
# endif
        return mResult.has_value();
#else
        return false;
#endif
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) noexcept {
#if CO_ASYNC_ALLOC
        mAllocState.push();
#endif
        mCallerCoroutine = coroutine;
        return mCalleeCoroutine;
    }

    GeneratorResult<T, E> await_resume() {
#if CO_ASYNC_ALLOC
        mAllocState.pop();
#endif
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
#if CO_ASYNC_SAFERET
# if CO_ASYNC_DEBUG
        GeneratorResult<T, E> ret = std::move(mResult.value());
# else
        GeneratorResult<T, E> ret = std::move(*mResult);
# endif
        mResult.reset();
        return ret;
#else
        return mResult.move();
#endif
    }

    template <class P>
    explicit TaskAwaiter(std::coroutine_handle<P> coroutine)
        : mCalleeCoroutine(coroutine) {
        coroutine.promise().mAwaiter = this;
    }

    TaskAwaiter(TaskAwaiter &&that) = delete;

    template <class U>
    TaskYieldAwaiter yieldValue(U &&value) {
        mResult.emplace(std::in_place_index<0>, std::forward<U>(value));
        return TaskYieldAwaiter();
    }

#if CO_ASYNC_DEBUG
    template <class U, class Loc>
    void returnValue(U &&result, Loc &&loc) {
        mResult.emplace(std::in_place_index<1>, std::forward<U>(result),
                        std::forward<Loc>(loc));
    }
#endif
    template <class U>
    void returnValue(U &&result) {
        mResult.emplace(std::in_place_index<1>, std::forward<U>(result));
    }

    void returnVoid() {
        mResult.emplace(std::in_place_index<1>);
    }

    void unhandledException() noexcept {
#if CO_ASYNC_EXCEPT
        mException = std::current_exception();
#else
        std::terminate();
#endif
    }

    std::coroutine_handle<> callerCoroutine() const noexcept {
        return mCallerCoroutine;
    }

protected:
    std::coroutine_handle<> mCallerCoroutine;
    std::coroutine_handle<> mCalleeCoroutine;
#if CO_ASYNC_SAFERET
    std::optional<GeneratorResult<T, E>> mResult;
#else
    Uninitialized<GeneratorResult<T, E>> mResult;
#endif
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException;
#endif
#if CO_ASYNC_ALLOC
    TaskAwaiterAllocState mAllocState;
#endif
};

template <class T>
struct TaskOwnedAwaiter : TaskAwaiter<T> {
    using TaskAwaiter<T>::TaskAwaiter;

    ~TaskOwnedAwaiter() {
        TaskAwaiter<T>::mCalleeCoroutine.destroy();
    }
};

template <class T>
struct TaskPromise;

template <class T = void, class P = TaskPromise<T>>
struct [[nodiscard("did you forgot to co_await?")]] Task {
    using promise_type = P;

    Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    Task(Task &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    Task &operator=(Task &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
        return *this;
    }

    ~Task() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }

    auto operator co_await() const & noexcept {
        return TaskAwaiter<T>(mCoroutine);
    }

    auto operator co_await() && noexcept {
        return TaskOwnedAwaiter<T>(std::exchange(mCoroutine, nullptr));
    }

    std::coroutine_handle<promise_type> get() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> release() noexcept {
        return std::exchange(mCoroutine, nullptr);
    }

    promise_type &promise() const {
        return mCoroutine.promise();
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

struct TaskPromiseLocal {
    void *mCancelToken = nullptr;
};

struct TaskPromiseCommonBase {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return TaskFinalAwaiter();
    }

    TaskPromiseCommonBase() = default;
    TaskPromiseCommonBase(TaskPromiseCommonBase &&) = delete;

#if CO_ASYNC_ALLOC
    void *operator new(std::size_t size) {
        return std::pmr::get_default_resource()->allocate(size);
    }

    void operator delete(void *ptr, std::size_t size) noexcept {
        std::pmr::get_default_resource()->deallocate(ptr, size);
    }
#endif
};

template <class TaskPromise>
struct TaskPromiseCommon : TaskPromiseCommonBase {
    TaskPromise &self() noexcept {
        return static_cast<TaskPromise &>(*this);
    }

    TaskPromise const &self() const noexcept {
        return static_cast<TaskPromise const &>(*this);
    }

    void unhandled_exception() noexcept {
        self().mAwaiter->unhandledException();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(self());
    }
};

template <class TaskPromise>
struct TaskPromiseExpectedTransforms {
    TaskPromise &self() noexcept {
        return static_cast<TaskPromise &>(*this);
    }

    TaskPromise const &self() const noexcept {
        return static_cast<TaskPromise const &>(*this);
    }

#if CO_ASYNC_DEBUG
# define CO_ASYNC_EXPECTED_LOCATION_FORWARD(e) , (e).mErrorLocation
#else
# define CO_ASYNC_EXPECTED_LOCATION_FORWARD(e)
#endif

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            self().mAwaiter->returnValue(
                std::move(e).error() CO_ASYNC_EXPECTED_LOCATION_FORWARD(e));
            return {self().mAwaiter->callerCoroutine()};
        }
        if constexpr (std::is_void_v<T2>) {
            return {std::in_place};
        } else {
            return {std::in_place, *std::move(e)};
        }
    }

    template <class T2>
    ValueOrReturnAwaiter<T2> await_transform(Expected<T2> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<GeneratorResult<T2, E2>>
    await_transform(GeneratorResult<T2, Expected<E2>> &&g) noexcept {
        if (g.has_value()) {
            if constexpr (std::is_void_v<T2>) {
                return {std::in_place, std::in_place_index<0>};
            } else {
                return {std::in_place, std::in_place_index<0>, std::move(*g)};
            }
        } else {
            auto e = g.result_unsafe();
            if (e.has_error()) [[unlikely]] {
                self().mAwaiter->returnValue(
                    std::move(e).error() CO_ASYNC_EXPECTED_LOCATION_FORWARD(e));
                return {self().mAwaiter->callerCoroutine()};
            } else {
                if constexpr (std::is_void_v<E2>) {
                    return {std::in_place, std::in_place_index<1>,
                            std::move(*e)};
                } else {
                    return {std::in_place, std::in_place_index<1>};
                }
            }
        }
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<GeneratorResult<T2, E2>>
    await_transform(GeneratorResult<T2, Expected<E2>> &g) noexcept {
        return await_transform(std::move(g));
    }

    ValueOrReturnAwaiter<void>
    await_transform(std::vector<Expected<void>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                self().mAwaiter->returnValue(
                    std::move(e[i]).error()
                        CO_ASYNC_EXPECTED_LOCATION_FORWARD(e[i]));
                return {self().mAwaiter->callerCoroutine()};
            }
        }
        return {std::in_place};
    }

    ValueOrReturnAwaiter<void>
    await_transform(std::vector<Expected<void>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class T2, class E2>
    ValueOrReturnAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                self().mAwaiter->returnValue(
                    std::move(e[i]).error()
                        CO_ASYNC_EXPECTED_LOCATION_FORWARD(e[i]));
                return {self().mAwaiter->callerCoroutine()};
            }
        }
        std::vector<T2> ret;
        ret.reserve(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) {
            ret.emplace_back(*std::move(e[i]));
        }
        return {std::in_place, std::move(ret)};
    }

    template <class T2>
    ValueOrReturnAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &&e) noexcept {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>)
                   -> ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>> {
            if (!([&]() -> bool {
                    if (std::get<Is>(e).has_error()) [[unlikely]] {
                        self().mAwaiter->returnValue(
                            std::move(std::get<Is>(e))
                                .error() CO_ASYNC_EXPECTED_LOCATION_FORWARD(
                                    std::get<Is>(e)));
                        return false;
                    }
                    return true;
                }() && ...)) {
                return {self().mAwaiter->callerCoroutine()};
            }
            return {std::in_place, [&]() -> decltype(auto) {
                        return *std::move(std::get<Is>(e)), Void();
                    }()...};
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <class... Ts>
    ValueOrReturnAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts>...> &e) noexcept {
        return await_transform(std::move(e));
    }
};

template <class TaskPromise>
struct TaskPromiseTransforms {
    TaskPromise &self() noexcept {
        return static_cast<TaskPromise &>(*this);
    }

    TaskPromise const &self() const noexcept {
        return static_cast<TaskPromise const &>(*this);
    }

    template <class U>
    Task<U> &&await_transform(Task<U> &&u) noexcept {
        u.promise().mLocals = self().mLocals;
        return std::move(u);
    }

    template <class U>
    Task<U> const &await_transform(Task<U> const &u) noexcept {
        u.promise().mLocals = self().mLocals;
        return u;
    }

    template <std::invocable<TaskPromise &> U>
    auto await_transform(U &&u) noexcept(noexcept(u(self()))) {
        return self().await_transform(u(self()));
    }

    template <class U>
        requires(!std::invocable<U, TaskPromise &>)
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }
};

template <class TaskPromise, class T>
struct TaskPromiseImpl : TaskPromiseCommon<TaskPromise>,
                         TaskPromiseTransforms<TaskPromise> {};

template <class TaskPromise, class T>
struct TaskPromiseImpl<TaskPromise, Expected<T>>
    : TaskPromiseCommon<TaskPromise>,
      TaskPromiseTransforms<TaskPromise>,
      TaskPromiseExpectedTransforms<TaskPromise> {
    using TaskPromiseTransforms<TaskPromise>::await_transform;
    using TaskPromiseExpectedTransforms<TaskPromise>::await_transform;
    static_assert(std::is_void_v<std::void_t<T>>);
};

template <class T>
struct TaskPromise : TaskPromiseImpl<TaskPromise<T>, T> {
    void return_value(T &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mAwaiter->returnValue(ret);
    }

#if CO_ASYNC_DEBUG
    void
    return_value(std::convertible_to<T> auto &&ret,
                 std::source_location loc = std::source_location::current())
        requires std::constructible_from<T, decltype(ret), std::source_location>
    {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret), loc);
    }

    void return_value(std::convertible_to<T> auto &&ret)
        requires(
            !std::constructible_from<T, decltype(ret), std::source_location>)
    {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }
#else
    void return_value(std::convertible_to<T> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }
#endif

    TaskAwaiter<T> *mAwaiter{};
    TaskPromiseLocal mLocals{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <>
struct TaskPromise<void> : TaskPromiseImpl<TaskPromise<void>, void> {
    void return_void() {
        mAwaiter->returnVoid();
    }

    TaskPromise() = default;
    TaskPromise(TaskPromise &&) = delete;

    TaskAwaiter<void> *mAwaiter{};
    TaskPromiseLocal mLocals{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class E>
struct TaskPromise<GeneratorResult<T, E>>
    : TaskPromiseImpl<TaskPromise<GeneratorResult<T, E>>, E> {
    auto yield_value(T &&ret) {
        return mAwaiter->yieldValue(std::move(ret));
    }

    auto yield_value(T const &ret) {
        return mAwaiter->yieldValue(ret);
    }

    auto yield_value(std::convertible_to<T> auto &&ret) {
        return mAwaiter->yieldValue(std::forward<decltype(ret)>(ret));
    }

    void return_value(E &&ret) {
        mAwaiter->returnValue(std::move(ret));
    }

    void return_value(E const &ret) {
        mAwaiter->returnValue(ret);
    }

    void return_value(std::convertible_to<E> auto &&ret) {
        mAwaiter->returnValue(std::forward<decltype(ret)>(ret));
    }

    TaskAwaiter<GeneratorResult<T, E>> *mAwaiter{};
    TaskPromiseLocal mLocals{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T>
struct TaskPromise<GeneratorResult<T, void>>
    : TaskPromiseImpl<TaskPromise<GeneratorResult<T, void>>, void> {
    auto yield_value(T &&ret) {
        return mAwaiter->yieldValue(std::move(ret));
    }

    auto yield_value(T const &ret) {
        return mAwaiter->yieldValue(ret);
    }

    auto yield_value(std::convertible_to<T> auto &&ret) {
        return mAwaiter->yieldValue(std::forward<decltype(ret)>(ret));
    }

    void return_void() {
        mAwaiter->returnVoid();
    }

    TaskAwaiter<GeneratorResult<T, void>> *mAwaiter{};
    TaskPromiseLocal mLocals{};

#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

// static_assert(sizeof(TaskPromise<int>) == sizeof(TaskPromise<void>));
// static_assert(sizeof(TaskPromise<Expected<>>) == sizeof(TaskPromise<void>));
// static_assert(sizeof(TaskPromise<GeneratorResult<int, Expected<>>>) ==
// sizeof(TaskPromise<void>));

template <class T, class P>
struct CustomPromise : TaskPromise<T> {
    auto get_return_object() {
        static_assert(std::is_base_of_v<CustomPromise, P>);
        return std::coroutine_handle<P>::from_promise(static_cast<P &>(*this));
    }
};

template <class F, class... Args>
    requires(Awaitable<std::invoke_result_t<F, Args...>>)
inline std::invoke_result_t<F, Args...> co_bind(F f, Args... args) {
    co_return co_await std::move(f)(std::move(args)...);
    /* std::optional o(std::move(f)); */
    /* decltype(auto) r = co_await std::move(*o)(); */
    /* o.reset(); */
    /* co_return */
    /*     typename AwaitableTraits<std::invoke_result_t<F,
     * Args...>>::RetType( */
    /*         std::forward<decltype(r)>(r)); */
}

} // namespace co_async

#define co_awaits co_await co_await
#define co_returns \
    co_return {}






namespace co_async {
template <Awaitable A, Awaitable F>
    requires(!std::invocable<F> &&
             !std::invocable<F, typename AwaitableTraits<A>::RetType>)
Task<typename AwaitableTraits<F>::RetType> and_then(A a, F f) {
    co_await std::move(a);
    co_return co_await std::move(f);
}
} // namespace co_async





namespace co_async {
inline Task<> just_void() {
    co_return;
}

template <class T>
Task<T> just_value(T t) {
    co_return std::move(t);
}

template <class F, class... Args>
Task<std::invoke_result_t<F, Args...>> just_invoke(F &&f, Args &&...args) {
    co_return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}
} // namespace co_async





namespace co_async {
struct ReturnPreviousPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() {
        throw;
    }

    void return_value(std::coroutine_handle<> previous) noexcept {
        mPrevious = previous;
    }

    auto get_return_object() {
        return std::coroutine_handle<ReturnPreviousPromise>::from_promise(
            *this);
    }

    std::coroutine_handle<> mPrevious;
    ReturnPreviousPromise &operator=(ReturnPreviousPromise &&) = delete;
};

using ReturnPreviousTask = Task<void, ReturnPreviousPromise>;
} // namespace co_async








namespace co_async {
struct WhenAllCtlBlock {
    std::size_t mCount;
    std::coroutine_handle<> mPrevious{};
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif
};

struct WhenAllAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const {
        if (mTasks.empty()) {
            return coroutine;
        }
        mControl.mPrevious = coroutine;
        for (auto const &t: mTasks.subspan(0, mTasks.size() - 1)) {
            t.get().resume();
        }
        return mTasks.back().get();
    }

    void await_resume() const {
#if CO_ASYNC_EXCEPT
        if (mControl.mException) [[unlikely]] {
            std::rethrow_exception(mControl.mException);
        }
#endif
    }

    WhenAllCtlBlock &mControl;
    std::span<ReturnPreviousTask const> mTasks;
};

template <class T>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<T> &result) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.emplace(co_await std::forward<decltype(t)>(t));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <class = void>
ReturnPreviousTask whenAllHelper(auto &&t, WhenAllCtlBlock &control,
                                 Uninitialized<void> &) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        co_await std::forward<decltype(t)>(t);
#if CO_ASYNC_EXCEPT
    } catch (...) {
        control.mException = std::current_exception();
        co_return control.mPrevious;
    }
#endif
    --control.mCount;
    if (control.mCount == 0) {
        co_return control.mPrevious;
    }
    co_return std::noop_coroutine();
}

template <std::size_t... Is, class... Ts>
Task<std::tuple<typename AwaitableTraits<Ts>::AvoidRetType...>>
whenAllImpl(std::index_sequence<Is...>, Ts &&...ts) {
    WhenAllCtlBlock control{sizeof...(Ts)};
    std::tuple<Uninitialized<typename AwaitableTraits<Ts>::RetType>...> result;
    ReturnPreviousTask taskArray[]{
        whenAllHelper(ts, control, std::get<Is>(result))...};
    co_await WhenAllAwaiter(control, taskArray);
    co_return std::tuple<typename AwaitableTraits<Ts>::AvoidRetType...>(
        std::get<Is>(result).move()...);
}

template <Awaitable... Ts>
    requires(sizeof...(Ts) != 0)
auto when_all(Ts &&...ts) {
    return whenAllImpl(std::make_index_sequence<sizeof...(Ts)>{},
                       std::forward<Ts>(ts)...);
}

template <Awaitable T, class Alloc = std::allocator<T>>
Task<std::conditional_t<
    !std::is_void_v<typename AwaitableTraits<T>::RetType>,
    std::vector<typename AwaitableTraits<T>::RetType,
                typename std::allocator_traits<Alloc>::template rebind_alloc<
                    typename AwaitableTraits<T>::RetType>>,
    void>>
when_all(std::vector<T, Alloc> const &tasks) {
    WhenAllCtlBlock control{tasks.size()};
    Alloc alloc = tasks.get_allocator();
    std::vector<Uninitialized<typename AwaitableTraits<T>::RetType>,
                typename std::allocator_traits<Alloc>::template rebind_alloc<
                    Uninitialized<typename AwaitableTraits<T>::RetType>>>
        result(tasks.size(), alloc);
    {
        std::vector<ReturnPreviousTask,
                    typename std::allocator_traits<
                        Alloc>::template rebind_alloc<ReturnPreviousTask>>
            taskArray(alloc);
        taskArray.reserve(tasks.size());
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            taskArray.push_back(whenAllHelper(tasks[i], control, result[i]));
        }
        co_await WhenAllAwaiter(control, taskArray);
    }
    if constexpr (!std::is_void_v<typename AwaitableTraits<T>::RetType>) {
        std::vector<
            typename AwaitableTraits<T>::RetType,
            typename std::allocator_traits<Alloc>::template rebind_alloc<
                typename AwaitableTraits<T>::RetType>>
            res(alloc);
        res.reserve(tasks.size());
        for (auto &r: result) {
            res.push_back(r.move());
        }
        co_return res;
    }
}
} // namespace co_async






namespace co_async {
template <Awaitable A>
A ensureAwaitable(A a) {
    return std::move(a);
}

template <class A>
    requires(!Awaitable<A>)
Task<A> ensureAwaitable(A a) {
    co_return std::move(a);
}

template <Awaitable A>
Task<typename AwaitableTraits<A>::RetType> ensureTask(A a) {
    co_return co_await std::move(a);
}

template <class T>
Task<T> ensureTask(Task<T> &&t) {
    return std::move(t);
}

template <class A>
    requires(!Awaitable<A> && std::invocable<A> &&
             Awaitable<std::invoke_result_t<A>>)
Task<typename AwaitableTraits<std::invoke_result_t<A>>::RetType>
ensureTask(A a) {
    return ensureTask(std::invoke(std::move(a)));
}
} // namespace co_async




namespace co_async {
struct CurrentCoroutineAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) noexcept {
        mCurrent = coroutine;
        return coroutine;
    }

    auto await_resume() const noexcept {
        return mCurrent;
    }

    std::coroutine_handle<> mCurrent;
};
} // namespace co_async






#if defined(__unix__) && __has_include(<cxxabi.h>)
# include <cxxabi.h>
#endif

namespace co_async {
template <class FinalAwaiter = std::suspend_always>
struct IgnoreReturnPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void unhandled_exception() noexcept {
#if CO_ASYNC_EXCEPT
        try {
            throw;
        } catch (std::exception const &e) {
            auto name = typeid(e).name();
# if defined(__unix__) && __has_include(<cxxabi.h>)
            int status;
            char *p = abi::__cxa_demangle(name, 0, 0, &status);
            std::string s = p ? p : name;
            std::free(p);
# else
            std::string s = name;
# endif
            std::cerr
                << "co_spawn coroutine terminated after thrown exception '" +
                       s + "'\n  e.what(): " + std::string(e.what()) + "\n";
        } catch (...) {
            std::cerr
                << "co_spawn coroutine terminated after thrown exception\n";
        }
#else
        std::terminate();
#endif
    }

    void result() noexcept {}

    void return_void() noexcept {}

    auto get_return_object() {
        return std::coroutine_handle<IgnoreReturnPromise>::from_promise(*this);
    }

    IgnoreReturnPromise &operator=(IgnoreReturnPromise &&) = delete;

    [[maybe_unused]] TaskAwaiter<void> *mAwaiter{};

#if CO_ASYNC_PERF
    Perf mPerf;

    IgnoreReturnPromise(
        std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

struct AutoDestroyFinalAwaiter {
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        coroutine.destroy();
    }

    void await_resume() const noexcept {}
};
} // namespace co_async




namespace co_async {

struct ListHead {
    struct ListNode {
        ListNode() noexcept : listNext(nullptr), listPrev(nullptr) {}
        friend struct ListHead;

    private:
        ListNode *listNext;
        ListNode *listPrev;
    };

    struct NodeType : ListNode {
        NodeType() = default;
        NodeType(NodeType &&) = delete;

        ~NodeType() noexcept {
            erase_from_parent();
        }

    protected:
        void erase_from_parent() {
            if (this->listNext) {
                auto listPrev = this->listPrev;
                auto listNext = this->listNext;
                listPrev->listNext = listNext;
                listNext->listPrev = listPrev;
                this->listPrev = nullptr;
                this->listNext = nullptr;
            }
        }
    };

    void doPushFront(ListNode *node) noexcept {
        node->listNext = root.listNext;
        node->listPrev = &root;
        root.listNext = node;
        node->listNext->listPrev = node;
    }

    void doPushBack(ListNode *node) noexcept {
        node->listNext = &root;
        node->listPrev = root.listPrev;
        root.listPrev = node;
        node->listPrev->listNext = node;
    }

    void doInsertAfter(ListNode *pivot, ListNode *node) noexcept {
        node->listNext = pivot->listNext;
        node->listPrev = pivot;
        pivot->listNext = node;
        node->listNext->listPrev = node;
    }

    void doInsertBefore(ListNode *pivot, ListNode *node) noexcept {
        node->listNext = pivot;
        node->listPrev = pivot->listPrev;
        pivot->listPrev = node;
        node->listPrev->listNext = node;
    }

    void doErase(ListNode *node) noexcept {
        node->listNext->listPrev = node->listPrev;
        node->listPrev->listNext = node->listNext;
        node->listNext = nullptr;
        node->listPrev = nullptr;
    }

    ListNode *doFront() const noexcept {
        return root.listNext;
    }

    ListNode *doBack() const noexcept {
        return root.listPrev;
    }

    bool doEmpty() const noexcept {
        return root.listNext == nullptr;
    }

    ListNode *doPopFront() noexcept {
        auto node = root.listNext;
        if (node != &root) {
            node->listNext->listPrev = &root;
            root.listNext = node->listNext;
            node->listNext = nullptr;
            node->listPrev = nullptr;
        } else {
            node = nullptr;
        }
        return node;
    }

    ListNode *doPopBack() noexcept {
        auto node = root.listPrev;
        if (node != &root) {
            node->listNext->listPrev = node->listPrev;
            node->listPrev->listNext = node->listNext;
            node->listNext = nullptr;
            node->listPrev = nullptr;
        } else {
            node = nullptr;
        }
        return node;
    }

    void doClear() {
        for (ListNode *current = root.listNext, *next; current != &root;
             current = next) {
            next = current->listNext;
            current->listNext = nullptr;
            current->listPrev = nullptr;
        }
        root.listNext = root.listPrev = &root;
    }

    static void doIterNext(ListNode *&current) noexcept {
        current = current->listNext;
    }

    static void doIterPrev(ListNode *&current) noexcept {
        current = current->listPrev;
    }

    ListNode *doIterBegin() const noexcept {
        return root.listNext;
    }

    ListNode *doIterEnd() const noexcept {
        return const_cast<ListNode *>(&root);
    }

    ListHead() noexcept : root() {
        root.listNext = root.listPrev = &root;
    }

    ListHead(ListHead &&) = delete;

    ~ListHead() noexcept {
        doClear();
    }

private:
    ListNode root;
};

// struct ConcurrentListHead {
//     struct ListNode {
//         ListNode() noexcept
//             : listNext(nullptr),
//               listPrev(nullptr),
//               listHead(nullptr) {}
//         friend struct ConcurrentListHead;
//
//     private:
//         ListNode *listNext;
//         ListNode *listPrev;
//
//     protected:
//         ConcurrentListHead *listHead;
//     };
//
//     struct NodeType : ListNode {
//         NodeType() = default;
//         NodeType(NodeType &&) = delete;
//
//         ~NodeType() noexcept {
//             erase_from_parent();
//         }
//
//     protected:
//         void erase_from_parent() {
//             if (this->listHead) {
//                 this->listHead->doErase(this);
//                 this->listHead = nullptr;
//             }
//         }
//     };
//
//     void doPushFront(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = root.listNext;
//         node->listPrev = &root;
//         root.listNext = node;
//         if (node->listNext) {
//             node->listNext->listPrev = node;
//         }
//     }
//
//     void doPushBack(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = &root;
//         node->listPrev = root.listPrev;
//         root.listPrev = node;
//         if (node->listPrev) {
//             node->listPrev->listNext = node;
//         }
//     }
//
//     void doInsertAfter(ListNode *pivot,
//                        ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = pivot->listNext;
//         node->listPrev = pivot;
//         pivot->listNext = node;
//         if (node->listNext) {
//             node->listNext->listPrev = node;
//         }
//     }
//
//     void doInsertBefore(ListNode *pivot,
//                         ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = this;
//         node->listNext = pivot;
//         node->listPrev = pivot->listPrev;
//         pivot->listPrev = node;
//         if (node->listPrev) {
//             node->listPrev->listNext = node;
//         }
//     }
//
//     void doErase(ListNode *node) noexcept {
//         std::lock_guard lock(listLock);
//         node->listHead = nullptr;
//         node->listNext->listPrev = node->listPrev;
//         node->listPrev->listNext = node->listNext;
//         node->listNext = nullptr;
//         node->listPrev = nullptr;
//     }
//
//     ListNode *doFront() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listNext;
//     }
//
//     ListNode *doBack() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listPrev;
//     }
//
//     bool doEmpty() const noexcept {
//         std::lock_guard lock(listLock);
//         return root.listNext == nullptr;
//     }
//
//     ListNode *doPopFront() noexcept {
//         std::lock_guard lock(listLock);
//         auto node = root.listNext;
//         if (node) {
//             node->listHead = nullptr;
//             node->listNext->listPrev = &root;
//             root.listNext = node->listNext;
//             node->listNext = nullptr;
//             node->listPrev = nullptr;
//         }
//         return node;
//     }
//
//     ListNode *doPopBack() noexcept {
//         std::lock_guard lock(listLock);
//         auto node = root.listPrev;
//         if (node) {
//             node->listHead = nullptr;
//             node->listNext->listPrev = node->listPrev;
//             node->listPrev->listNext = node->listNext;
//             node->listNext = nullptr;
//             node->listPrev = nullptr;
//         }
//         return node;
//     }
//
//     void doClear() {
//         std::lock_guard lock(listLock);
//         for (ListNode *current = root.listNext, *next;
//              current != &root; current = next) {
//             next = current->listNext;
//             current->listHead = nullptr;
//             current->listNext = nullptr;
//             current->listPrev = nullptr;
//         }
//     }
//
//     ConcurrentListHead() noexcept : root() {}
//
//     ConcurrentListHead(ConcurrentListHead &&) = delete;
//
//     ~ConcurrentListHead() noexcept {
//         doClear();
//     }
//
// private:
//     ListNode root;
//     mutable std::mutex listLock;
// };

template <class Value>
struct IntrusiveList : private ListHead {
    using ListHead::NodeType;

    IntrusiveList() noexcept {
        static_assert(
            std::is_base_of_v<NodeType, Value>,
            "Value type must be derived from IntrusiveList<Value>::NodeType");
    }

    struct iterator {
    private:
        ListNode *node;

        explicit iterator(ListNode *node) noexcept : node(node) {}

        friend IntrusiveList;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = Value *;
        using reference = Value &;

        iterator() noexcept : node(nullptr) {}

        explicit iterator(Value &value) noexcept
            : node(&static_cast<ListNode &>(value)) {}

        Value &operator*() const noexcept {
            return *static_cast<Value *>(node);
        }

        Value *operator->() const noexcept {
            return static_cast<Value *>(node);
        }

        iterator &operator++() noexcept {
            ListHead::doIterNext(node);
            return *this;
        }

        iterator &operator--() noexcept {
            ListHead::doIterPrev(node);
            return *this;
        }

        iterator operator++(int) noexcept {
            auto copy = *this;
            ListHead::doIterNext(node);
            return copy;
        }

        iterator operator--(int) noexcept {
            auto copy = *this;
            ListHead::doIterPrev(node);
            return copy;
        }

        bool operator==(iterator const &other) const noexcept {
            return node == other.node;
        }

        bool operator!=(iterator const &other) const noexcept {
            return node != other.node;
        }
    };

    using const_iterator = iterator;

    iterator begin() const noexcept {
        return iterator(ListHead::doIterBegin());
    }

    iterator end() const noexcept {
        return iterator(ListHead::doIterEnd());
    }

    void push_front(Value &value) noexcept {
        doPushFront(&static_cast<ListNode &>(value));
    }

    void push_back(Value &value) noexcept {
        doPushBack(&static_cast<ListNode &>(value));
    }

    void insert_after(Value &pivot, Value &value) noexcept {
        doInsertAfter(&static_cast<ListNode &>(pivot),
                      &static_cast<ListNode &>(value));
    }

    void insert_before(Value &pivot, Value &value) noexcept {
        doInsertBefore(&static_cast<ListNode &>(pivot),
                       &static_cast<ListNode &>(value));
    }

    void erase(Value &value) noexcept {
        doErase(&static_cast<ListNode &>(value));
    }

    bool empty() const noexcept {
        return doEmpty();
    }

    Value &front() const noexcept {
        return static_cast<Value &>(*doFront());
    }

    Value &back() const noexcept {
        return static_cast<Value &>(*doBack());
    }

    Value *pop_front() noexcept {
        auto node = doPopFront();
        return node ? static_cast<Value *>(node) : nullptr;
    }

    Value *pop_back() noexcept {
        auto node = doPopBack();
        return node ? static_cast<Value *>(node) : nullptr;
    }

    void clear() {
        doClear();
    }
};

// template <class Value>
// struct ConcurrentIntrusiveList : private ConcurrentListHead {
//     using ConcurrentListHead::NodeType;
//
//     ConcurrentIntrusiveList() noexcept {
//         static_assert(
//             std::is_base_of_v<NodeType, Value>,
//             "Value type must be derived from
//             ConcurrentIntrusiveList<Value>::NodeType");
//     }
//
//     void push_front(Value &value) noexcept {
//         doPushFront(&static_cast<ListNode &>(value));
//     }
//
//     void push_back(Value &value) noexcept {
//         doPushBack(&static_cast<ListNode &>(value));
//     }
//
//     void insert_after(Value &pivot, Value &value) noexcept {
//         doInsertAfter(&static_cast<ListNode &>(pivot),
//                       &static_cast<ListNode &>(value));
//     }
//
//     void insert_before(Value &pivot, Value &value) noexcept {
//         doInsertBefore(&static_cast<ListNode &>(pivot),
//                        &static_cast<ListNode &>(value));
//     }
//
//     void erase(Value &value) noexcept {
//         doErase(&static_cast<ListNode &>(value));
//     }
//
//     bool empty() const noexcept {
//         return doEmpty();
//     }
//
//     Value &front() const noexcept {
//         return static_cast<Value &>(*doFront());
//     }
//
//     Value &back() const noexcept {
//         return static_cast<Value &>(*doBack());
//     }
//
//     Value *pop_front() noexcept {
//         auto node = doPopFront();
//         return node ? static_cast<Value *>(node) : nullptr;
//     }
//
//     Value *pop_back() noexcept {
//         auto node = doPopBack();
//         return node ? static_cast<Value *>(node) : nullptr;
//     }
//
//     void clear() {
//         doClear();
//     }
// };
} // namespace co_async








namespace co_async {

struct CancelSourceImpl {
    struct CancellerBase : IntrusiveList<CancellerBase>::NodeType {
        virtual Task<> doCancel() = 0;

        CancellerBase &operator=(CancellerBase &&) = delete;

        bool operator<(CancellerBase const &that) const noexcept {
            return this < &that;
        }

        // virtual ~CancellerBase() = default;
    };

    /* template <class AwaiterPtr, class Canceller> */
    /* struct CancellerImpl : CancellerBase { */
    /*     AwaiterPtr mOp; */
    /*  */
    /*     explicit CancellerImpl(AwaiterPtr op) : mOp(op) {} */
    /*  */
    /*     virtual Task<> doCancel() { */
    /*         return Canceller::doCancel(mOp); */
    /*     } */
    /* }; */

    IntrusiveList<CancellerBase> mCancellers;
    bool mCanceled;

    Task<> doCancel() {
        if (mCanceled) {
            co_return;
        }
        mCanceled = true;
        if (!mCancellers.empty()) {
            std::vector<Task<>> tasks;
            for (auto &canceller: mCancellers) {
                tasks.push_back(canceller.doCancel());
            }
            /* for (auto &&task: tasks) { */
            /*     co_await task; */
            /* } */
            co_await when_all(tasks);
            mCancellers.clear();
        }
    }

    bool doIsCanceled() const noexcept {
        return mCanceled;
    }

    void doRegister(CancellerBase &canceller) {
        mCancellers.push_front(canceller);
    }

    /* template <class Canceller, class Awaiter> */
    /* static Task<typename AwaitableTraits<Awaiter>::RetType> */
    /* doGuard(CancelSourceImpl *impl, Awaiter &&awaiter) { */
    /*     if (impl) { */
    /*         auto *op = std::addressof(awaiter); */
    /*         CancellerImpl<decltype(op), Canceller> canceller(op); */
    /*         impl->doRegister(canceller); */
    /*         co_return co_await awaiter; */
    /*     } else { */
    /*         co_return co_await awaiter; */
    /*     } */
    /* } */
};

struct CancelToken;

struct [[nodiscard]] CancelSourceBase {
protected:
    std::unique_ptr<CancelSourceImpl> mImpl =
        std::make_unique<CancelSourceImpl>();

    friend CancelToken;

    template <class Callback>
    friend struct CancelCallback;

public:
    Task<> cancel() const {
        return mImpl->doCancel();
    }

    inline CancelToken token() const;
    CancelSourceBase() = default;
    CancelSourceBase(CancelSourceBase &&) = default;
    CancelSourceBase &operator=(CancelSourceBase &&) = default;
};

struct [[nodiscard(
    "did you forget to capture or co_await the cancel token?")]] CancelToken {
private:
    CancelSourceImpl *mImpl;

    explicit CancelToken(CancelSourceImpl *impl) noexcept : mImpl(impl) {}

public:
    CancelToken() noexcept : mImpl(nullptr) {}

    CancelToken(CancelSourceBase const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl ? mImpl->doCancel() : just_void();
    }

    [[nodiscard]] bool is_cancel_possible() const noexcept {
        return mImpl;
    }

    [[nodiscard]] bool is_canceled() const noexcept {
        return mImpl && mImpl->doIsCanceled();
    }

    [[nodiscard]] operator bool() const noexcept {
        return is_canceled();
    }

    Expected<> as_expect() {
        if (mImpl->doIsCanceled()) [[unlikely]] {
            return std::errc::operation_canceled;
        }
        return {};
    }

    void *address() const noexcept {
        return mImpl;
    }

    static CancelToken from_address(void *impl) noexcept {
        return CancelToken(static_cast<CancelSourceImpl *>(impl));
    }

    auto repr() const {
        return mImpl;
    }

    /* template <class Awaiter, */
    /*           class Canceller = typename std::decay_t<Awaiter>::Canceller> */
    /* auto guard(Awaiter &&awaiter) const { */
    /*     return CancelSourceImpl::doGuard<Canceller>( */
    /*         mImpl, std::forward<Awaiter>(awaiter)); */
    /* } */
    /*  */
    /* template <class Awaiter, class Canceller> */
    /* auto guard(std::in_place_type_t<Canceller>, Awaiter &&awaiter) const { */
    /*     return CancelSourceImpl::doGuard<Canceller>( */
    /*         mImpl, std::forward<Awaiter>(awaiter)); */
    /* } */

    /* template <class Callback> */
    /* [[nodiscard("capture me in a local variable")]] auto callback(Callback
     * callback) { */
    /*     std::unique_ptr<CancelSource::CancellerCallback<Callback>> canceller;
     */
    /*     if (mImpl) { */
    /*         canceller =
     * std::make_unique<CancelSource::CancellerCallback>(std::move(callback));
     */
    /*         mImpl->doRegister(*canceller); */
    /*     } */
    /*     return canceller; */
    /* } */

    template <class T>
    Expected<> operator()(TaskPromise<T> &promise) const {
        if (is_canceled()) [[unlikely]] {
            return std::errc::operation_canceled;
        }
        return {};
    }

    friend struct CancelSource;

    template <class Callback>
    friend struct CancelCallback;
};

struct CancelSource : private CancelSourceImpl::CancellerBase,
                      public CancelSourceBase {
private:
    Task<> doCancel() override {
        return cancel();
    }

public:
    CancelSource() = default;

    explicit CancelSource(CancelToken cancel) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

    auto repr() {
        return mImpl.get();
    }
};

inline CancelToken CancelSourceBase::token() const {
    return *this;
}

template <class Callback>
struct [[nodiscard]] CancelCallback : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelToken cancel, Callback callback)
        : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    Task<> doCancel() override {
        std::invoke(std::move(mCallback));
        co_return;
    }

    Callback mCallback;
};

template <class Callback>
    requires Awaitable<std::invoke_result_t<Callback>>
struct [[nodiscard]] CancelCallback<Callback>
    : private CancelSourceImpl::CancellerBase {
    explicit CancelCallback(CancelToken cancel, Callback callback)
        : mCallback(std::move(callback)) {
        if (cancel.mImpl) {
            cancel.mImpl->doRegister(*this);
        }
    }

private:
    Task<> doCancel() override {
        co_await std::invoke(std::move(mCallback));
    }

    Callback mCallback;
};

template <class Callback>
CancelCallback(CancelToken, Callback) -> CancelCallback<Callback>;

struct GetThisCancel {
    template <class T>
    ValueAwaiter<CancelToken> operator()(TaskPromise<T> &promise) const {
        return ValueAwaiter<CancelToken>(
            CancelToken::from_address(promise.mLocals.mCancelToken));
    }

    template <class T>
    static T &&bind(CancelToken cancel, T &&task) {
        task.promise().mLocals.mCancelToken = cancel.address();
        return std::forward<T>(task);
    }

    struct DoCancelThis {
        template <class T>
        Task<> operator()(TaskPromise<T> &promise) const {
            co_return co_await CancelToken::from_address(
                promise.mLocals.mCancelToken)
                .cancel();
        }
    };

    static DoCancelThis cancel() {
        return {};
    }

    /* struct DoExpectCancel { */
    /*     template <class T> */
    /*     Expected<> operator()(TaskPromise<T> &promise) const { */
    /*         return CancelToken(promise.mLocals.mCancelToken).expect(); */
    /*     } */
    /* }; */
    /*  */
    /* static DoExpectCancel expect() { */
    /*     return {}; */
    /* } */
};

inline constexpr GetThisCancel co_cancel;

} // namespace co_async





namespace co_async {

struct SpinMutex {
    bool try_lock() {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

    std::atomic_flag flag{false};
};

} // namespace co_async




namespace co_async {
template <class Value, class Compare = std::less<>>
struct RbTree : private Compare {
private:
    enum RbColor {
        RED,
        BLACK
    };

protected:
    struct RbNode {
        RbNode() noexcept
            : rbLeft(nullptr),
              rbRight(nullptr),
              rbParent(nullptr),
              rbTree(nullptr),
              rbColor(RED) {}
        friend struct RbTree;

    private:
        RbNode *rbLeft;
        RbNode *rbRight;
        RbNode *rbParent;

    protected:
        RbTree *rbTree;

    private:
        RbColor rbColor;
    };

public:
    struct NodeType : RbNode {
        NodeType() = default;
        NodeType(NodeType &&) = delete;

        ~NodeType() noexcept {
            erase_from_parent();
        }

    protected:
        void erase_from_parent() {
            static_assert(
                std::is_base_of_v<NodeType, Value>,
                "Value type must be derived from RbTree<Value>::NodeType");
            if (this->rbTree) {
                this->rbTree->doErase(this);
                this->rbTree = nullptr;
            }
        }
    };

private:
    RbNode *root;

    bool compare(RbNode *left, RbNode *right) const noexcept {
        return static_cast<Compare const &>(*this)(
            static_cast<Value &>(*left), static_cast<Value &>(*right));
    }

    void rotateLeft(RbNode *node) noexcept {
        RbNode *rightChild = node->rbRight;
        node->rbRight = rightChild->rbLeft;
        if (rightChild->rbLeft != nullptr) {
            rightChild->rbLeft->rbParent = node;
        }
        rightChild->rbParent = node->rbParent;
        if (node->rbParent == nullptr) {
            root = rightChild;
        } else if (node == node->rbParent->rbLeft) {
            node->rbParent->rbLeft = rightChild;
        } else {
            node->rbParent->rbRight = rightChild;
        }
        rightChild->rbLeft = node;
        node->rbParent = rightChild;
    }

    void rotateRight(RbNode *node) noexcept {
        RbNode *leftChild = node->rbLeft;
        node->rbLeft = leftChild->rbRight;
        if (leftChild->rbRight != nullptr) {
            leftChild->rbRight->rbParent = node;
        }
        leftChild->rbParent = node->rbParent;
        if (node->rbParent == nullptr) {
            root = leftChild;
        } else if (node == node->rbParent->rbRight) {
            node->rbParent->rbRight = leftChild;
        } else {
            node->rbParent->rbLeft = leftChild;
        }
        leftChild->rbRight = node;
        node->rbParent = leftChild;
    }

    void fixViolation(RbNode *node) noexcept {
        RbNode *parent = nullptr;
        RbNode *grandParent = nullptr;
        while (node != root && node->rbColor != BLACK &&
               node->rbParent->rbColor == RED) {
            parent = node->rbParent;
            grandParent = parent->rbParent;
            if (parent == grandParent->rbLeft) {
                RbNode *uncle = grandParent->rbRight;
                if (uncle != nullptr && uncle->rbColor == RED) {
                    grandParent->rbColor = RED;
                    parent->rbColor = BLACK;
                    uncle->rbColor = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->rbRight) {
                        rotateLeft(parent);
                        node = parent;
                        parent = node->rbParent;
                    }
                    rotateRight(grandParent);
                    std::swap(parent->rbColor, grandParent->rbColor);
                    node = parent;
                }
            } else {
                RbNode *uncle = grandParent->rbLeft;
                if (uncle != nullptr && uncle->rbColor == RED) {
                    grandParent->rbColor = RED;
                    parent->rbColor = BLACK;
                    uncle->rbColor = BLACK;
                    node = grandParent;
                } else {
                    if (node == parent->rbLeft) {
                        rotateRight(parent);
                        node = parent;
                        parent = node->rbParent;
                    }
                    rotateLeft(grandParent);
                    std::swap(parent->rbColor, grandParent->rbColor);
                    node = parent;
                }
            }
        }
        root->rbColor = BLACK;
    }

    void doInsert(RbNode *node) noexcept {
        node->rbLeft = nullptr;
        node->rbRight = nullptr;
        node->rbTree = this;
        node->rbColor = RED;
        RbNode *parent = nullptr;
        RbNode *current = root;
        while (current != nullptr) {
            parent = current;
            if (compare(node, current)) {
                current = current->rbLeft;
            } else {
                current = current->rbRight;
            }
        }
        node->rbParent = parent;
        if (parent == nullptr) {
            root = node;
        } else if (compare(node, parent)) {
            parent->rbLeft = node;
        } else {
            parent->rbRight = node;
        }
        fixViolation(node);
    }

    void doErase(RbNode *current) noexcept {
        current->rbTree = nullptr;
        RbNode *node = nullptr;
        RbNode *child = nullptr;
        RbColor color = RED;
        if (current->rbLeft != nullptr && current->rbRight != nullptr) {
            RbNode *replace = current;
            replace = replace->rbRight;
            while (replace->rbLeft != nullptr) {
                replace = replace->rbLeft;
            }
            if (current != replace->rbParent) {
                current->rbParent->rbLeft = replace->rbRight;
                replace->rbRight = current->rbRight;
                current->rbRight->rbParent = replace;
            } else {
                replace->rbParent = current;
            }
            if (current == root) {
                root = replace;
            } else if (current->rbParent->rbLeft == current) {
                current->rbParent->rbLeft = replace;
            } else {
                current->rbParent->rbRight = replace;
            }
            replace->rbLeft = current->rbLeft;
            current->rbLeft->rbParent = replace;
            node = replace;
            color = node->rbColor;
            child = node->rbRight;
        } else {
            node = current;
            color = node->rbColor;
            child = (node->rbLeft != nullptr) ? node->rbLeft : node->rbRight;
        }
        if (child != nullptr) {
            child->rbParent = node->rbParent;
        }
        if (node == root) {
            root = child;
        } else if (node->rbParent->rbLeft == node) {
            node->rbParent->rbLeft = child;
        } else {
            node->rbParent->rbRight = child;
        }
        if (color == BLACK && root) {
            fixViolation(child ? child : node->rbParent);
        }
    }

    RbNode *getFront() const noexcept {
        RbNode *current = root;
        while (current->rbLeft != nullptr) {
            current = current->rbLeft;
        }
        return current;
    }

    RbNode *getBack() const noexcept {
        RbNode *current = root;
        while (current->rbRight != nullptr) {
            current = current->rbRight;
        }
        return current;
    }

    template <class Visitor>
    void doTraverseInorder(RbNode *node, Visitor &&visitor) {
        if (node == nullptr) {
            return;
        }
        doTraverseInorder(node->rbLeft, visitor);
        visitor(node);
        doTraverseInorder(node->rbRight, visitor);
    }

    void doClear(RbNode *node) {
        if (node == nullptr) {
            return;
        }
        doClear(node->rbLeft);
        node->rbTree = nullptr;
        doClear(node->rbRight);
    }

    void doClear() {
        doClear(root);
        root = nullptr;
    }

public:
    RbTree() noexcept(noexcept(Compare())) : Compare(), root(nullptr) {}

    explicit RbTree(Compare comp) noexcept(noexcept(Compare(comp)))
        : Compare(comp),
          root(nullptr) {}

    RbTree(RbTree &&) = delete;

    ~RbTree() noexcept {}

    void insert(Value &value) noexcept {
        doInsert(&static_cast<RbNode &>(value));
    }

    void erase(Value &value) noexcept {
        doErase(&static_cast<RbNode &>(value));
    }

    bool empty() const noexcept {
        return root == nullptr;
    }

    Value &front() const noexcept {
        return static_cast<Value &>(*getFront());
    }

    Value &back() const noexcept {
        return static_cast<Value &>(*getBack());
    }

    template <class Visitor, class V>
    std::pair<RbNode *, RbNode *> traverseEqualRange(Visitor &&visitor,
                                                     V &&value) {}

    template <class Visitor>
    void traverseInorder(Visitor &&visitor) {
        doTraverseInorder(root, [visitor = std::forward<Visitor>(visitor)](
                                    RbNode *node) mutable {
            visitor(static_cast<Value &>(*node));
        });
    }

    void clear() {
        doClear();
    }
};

// template <class Value, class Compare = std::less<>>
// struct ConcurrentRbTree : private RbTree<Value, Compare> {
// private:
//     using BaseTree = RbTree<Value, Compare>;
//
// public:
//     struct NodeType : BaseTree::RbNode {
//         NodeType() = default;
//         NodeType(NodeType &&) = delete;
//
//         ~NodeType() noexcept {
//             erase_from_parent();
//         }
//
//     protected:
//         void erase_from_parent() {
//             static_assert(
//                 std::is_base_of_v<NodeType, Value>,
//                 "Value type must be derived from RbTree<Value>::NodeType");
//             if (this->rbTree) {
//                 auto lock = static_cast<ConcurrentRbTree
//                 *>(this->rbTree)->lock(); lock->erase(static_cast<Value
//                 &>(*this)); this->rbTree = nullptr;
//             }
//         }
//     };
//
//     struct LockGuard {
//     private:
//         BaseTree *mThat;
//         std::unique_lock<std::mutex> mGuard;
//
//         explicit LockGuard(ConcurrentRbTree *that) noexcept
//             : mThat(that),
//               mGuard(that->mMutex) {}
//
//         friend ConcurrentRbTree;
//
//     public:
//         BaseTree &operator*() const noexcept {
//             return *mThat;
//         }
//
//         BaseTree *operator->() const noexcept {
//             return mThat;
//         }
//
//         void unlock() noexcept {
//             mGuard.unlock();
//             mThat = nullptr;
//         }
//     };
//
//     LockGuard lock() noexcept {
//         return LockGuard(this);
//     }
//
// private:
//     std::mutex mMutex;
// };
} // namespace co_async




namespace co_async {
template <class T>
struct RingQueue {
    std::unique_ptr<T[]> mHead;
    T *mTail;
    T *mRead;
    T *mWrite;

    explicit RingQueue(std::size_t maxSize = 0)
        : mHead(maxSize ? std::make_unique<T[]>(maxSize) : nullptr),
          mTail(maxSize ? mHead.get() + maxSize : nullptr),
          mRead(mHead.get()),
          mWrite(mHead.get()) {}

    void set_max_size(std::size_t maxSize) {
        mHead = maxSize ? std::make_unique<T[]>(maxSize) : nullptr;
        mTail = maxSize ? mHead.get() + maxSize : nullptr;
        mRead = mHead.get();
        mWrite = mHead.get();
    }

    [[nodiscard]] std::size_t max_size() const noexcept {
        return mTail - mHead.get();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return static_cast<std::size_t>(mWrite - mRead + max_size()) %
               max_size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return mRead == mWrite;
    }

    [[nodiscard]] bool full() const noexcept {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        return nextWrite == mRead;
    }

    [[nodiscard]] std::optional<T> pop() {
        if (mRead == mWrite) {
            return std::nullopt;
        }
        T p = std::move(*mRead);
        mRead = mRead == mTail ? mHead.get() : mRead + 1;
        return p;
    }

    [[nodiscard]] T pop_unchecked() {
        T p = std::move(*mRead);
        mRead = mRead == mTail ? mHead.get() : mRead + 1;
        return p;
    }

    [[nodiscard]] bool push(T &&value) {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        if (nextWrite == mRead) {
            return false;
        }
        *mWrite = std::move(value);
        mWrite = nextWrite;
        return true;
    }

    void push_unchecked(T &&value) {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        *mWrite = std::move(value);
        mWrite = nextWrite;
    }
};

template <class T>
struct InfinityQueue {
    [[nodiscard]] std::optional<T> pop() {
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T value = std::move(mQueue.front());
        mQueue.pop_front();
        return value;
    }

    [[nodiscard]] T pop_unchecked() {
        T value = std::move(mQueue.front());
        mQueue.pop_front();
        return value;
    }

    void push(T &&value) {
        mQueue.push_back(std::move(value));
    }

private:
    std::deque<T> mQueue;
};
} // namespace co_async




namespace co_async {
#if __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
constexpr std::size_t hardware_constructive_interference_size = 64;
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif
} // namespace co_async







namespace co_async {
template <class T, std::size_t Capacity = 0>
struct alignas(hardware_destructive_interference_size) ConcurrentRingQueue {
    static constexpr std::size_t Shift = std::bit_width(Capacity);
    using Stamp = std::conditional_t<
        Shift <= 4, std::uint8_t,
        std::conditional_t<
            Shift <= 8, std::uint16_t,
            std::conditional_t<Shift <= 16, std::uint32_t, std::uint64_t>>>;
    static_assert(Shift * 2 <= sizeof(Stamp) * 8);
    static_assert(Capacity < (1 << Shift));
    static constexpr Stamp kSize = 1 << Shift;

    [[nodiscard]] std::optional<T> pop() {
        auto s = mStamp.load(std::memory_order_relaxed);
        if (!canRead(s)) {
            return std::nullopt;
        }
        while (!mStamp.compare_exchange_weak(s, advectRead(s),
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
            if (!canRead(s)) {
                return std::nullopt;
            }
        }
        return std::move(mHead[offsetRead(s)]);
    }

    [[nodiscard]] bool push(T &&value) {
        auto s = mStamp.load(std::memory_order_relaxed);
        if (!canWrite(s)) [[unlikely]] {
            return false;
        }
        while (!mStamp.compare_exchange_weak(s, advectWrite(s),
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
            if (!canWrite(s)) [[unlikely]] {
                return false;
            }
        }
        mHead[offsetWrite(s)] = std::move(value);
        return true;
    }

    ConcurrentRingQueue() = default;
    ConcurrentRingQueue(ConcurrentRingQueue &&) = delete;

private:
    inline Stamp offsetRead(Stamp s) const {
        return s >> Shift;
    }

    inline Stamp offsetWrite(Stamp s) const {
        return s & (kSize - 1);
    }

    inline bool canRead(Stamp s) const {
        return offsetRead(s) != offsetWrite(s);
    }

    inline bool canWrite(Stamp s) const {
        return (offsetRead(s) & static_cast<Stamp>(kSize - 1)) !=
               ((offsetWrite(s) + static_cast<Stamp>(kSize - Capacity)) &
                static_cast<Stamp>(kSize - 1));
    }

    inline Stamp advectRead(Stamp s) const {
        return static_cast<Stamp>(
                   ((static_cast<Stamp>(s >> Shift) + static_cast<Stamp>(1)) &
                    static_cast<Stamp>(kSize - 1))
                   << Shift) |
               (s & static_cast<Stamp>(kSize - 1));
    }

    inline Stamp advectWrite(Stamp s) const {
        return (((s & static_cast<Stamp>(kSize - 1)) + static_cast<Stamp>(1)) &
                static_cast<Stamp>(kSize - 1)) |
               static_cast<Stamp>(s & (static_cast<Stamp>(kSize - 1) << Shift));
    }

    std::unique_ptr<T[]> const mHead = std::make_unique<T[]>(kSize);
    std::atomic<Stamp> mStamp{0};
};

template <class T>
struct alignas(hardware_destructive_interference_size)
    ConcurrentRingQueue<T, 0> {
    std::optional<T> pop() {
        std::lock_guard lck(mMutex);
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T p = std::move(mQueue.front());
        mQueue.pop_front();
        return p;
    }

    void push(T &&value) {
        std::lock_guard lck(mMutex);
        mQueue.push_back(std::move(value));
    }

private:
    std::deque<T> mQueue;
    SpinMutex mMutex;
};
} // namespace co_async















namespace co_async {
struct IOContext;

struct GenericIOContext {
    struct TimerNode : CustomPromise<Expected<>, TimerNode>,
                       RbTree<TimerNode>::NodeType {
        using RbTree<TimerNode>::NodeType::erase_from_parent;
        std::chrono::steady_clock::time_point mExpires;
        CancelToken mCancelToken;
        bool mCancelled = false;

        void doCancel() {
            mCancelled = true;
            erase_from_parent();
        }

        bool operator<(TimerNode const &that) const {
            return mExpires < that.mExpires;
        }

        struct Awaiter {
            std::chrono::steady_clock::time_point mExpires;
            TimerNode *mPromise = nullptr;

            bool await_ready() const noexcept {
                return false;
            }

            inline void
            await_suspend(std::coroutine_handle<TimerNode> coroutine);

            Expected<> await_resume() const {
                if (!mPromise->mCancelled) {
                    return {};
                } else {
                    return std::errc::operation_canceled;
                }
            }
        };
    };

    [[gnu::hot]] std::optional<std::chrono::steady_clock::duration>
    runDuration();

    // [[gnu::hot]] void enqueueJob(std::coroutine_handle<> coroutine) {
    //     mQueue.push(std::move(coroutine));
    // }

    [[gnu::hot]] void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    GenericIOContext();
    ~GenericIOContext();

    GenericIOContext(GenericIOContext &&) = delete;
    static inline thread_local GenericIOContext *instance;

private:
    RbTree<TimerNode> mTimers;
};

inline void GenericIOContext::TimerNode::Awaiter::await_suspend(
    std::coroutine_handle<GenericIOContext::TimerNode> coroutine) {
    mPromise = &coroutine.promise();
    mPromise->mExpires = mExpires;
    GenericIOContext::instance->enqueueTimerNode(*mPromise);
}

template <class A>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
coSpawnStarter(A awaitable) {
    (void)co_await std::move(awaitable);
}

template <Awaitable A>
inline void co_spawn(A awaitable) {
    auto wrapped = coSpawnStarter(std::move(awaitable));
    auto coroutine = wrapped.release();
    coroutine.resume();
}

inline void co_spawn(std::coroutine_handle<> coroutine) {
    coroutine.resume();
}

inline auto co_resume() {
    struct ResumeAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            co_spawn(coroutine);
        }

        void await_resume() const noexcept {}
    };

    return ResumeAwaiter();
}

} // namespace co_async





namespace co_async {
#if CO_ASYNC_DEBUG
inline Expected<int>
expectError(int res,
            std::source_location loc = std::source_location::current()) {
    if (res < 0) [[unlikely]] {
        return Expected<int>(std::errc(-res), loc);
    }
    return res;
}
#else
inline Expected<int> expectError(int res) {
    if (res < 0) [[unlikely]] {
        return std::errc(-res);
    }
    return res;
}
#endif

inline int throwingError(int res) {
    if (res < 0) [[unlikely]] {
        throw std::system_error(-res, std::system_category());
    }
    return res;
}

inline int throwingErrorErrno(int res) {
    if (res == -1) [[unlikely]] {
        throw std::system_error(errno, std::system_category());
    }
    return res;
}
} // namespace co_async






#include <fcntl.h>
#include <liburing.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
template <class Rep, class Period>
struct __kernel_timespec
durationToKernelTimespec(std::chrono::duration<Rep, Period> dur) {
    struct __kernel_timespec ts;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    auto nsecs =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur - secs);
    ts.tv_sec = static_cast<__kernel_time64_t>(secs.count());
    ts.tv_nsec = static_cast<__kernel_time64_t>(nsecs.count());
    return ts;
}

template <class Clk, class Dur>
struct __kernel_timespec
timePointToKernelTimespec(std::chrono::time_point<Clk, Dur> tp) {
    return durationToKernelTimespec(tp.time_since_epoch());
}

struct PlatformIOContext {
    [[gnu::cold]] static void schedSetThreadAffinity(size_t cpu);

    struct IOUringProbe {
        struct io_uring_probe *mProbe;
        struct io_uring *mRing;

        [[gnu::cold]] IOUringProbe();
        IOUringProbe(IOUringProbe &&) = delete;
        [[gnu::cold]] ~IOUringProbe();
        [[gnu::cold]] bool isSupported(int op) noexcept;
        [[gnu::cold]] void dumpDiagnostics();
    };

    [[gnu::hot]] bool
    waitEventsFor(std::optional<std::chrono::steady_clock::duration> timeout);

    [[gnu::hot]] struct io_uring_sqe *getSqe() {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&mRing);
        while (!sqe) {
            int res = io_uring_submit(&mRing);
            if (res < 0) [[unlikely]] {
                if (res == -EINTR) {
                    continue;
                }
                throw std::system_error(-res, std::system_category());
            }
            sqe = io_uring_get_sqe(&mRing);
        }
        ++mNumSqesPending;
        return sqe;
    }

    PlatformIOContext &operator=(PlatformIOContext &&) = delete;
    [[gnu::cold]] PlatformIOContext() noexcept;
    [[gnu::cold]] void setup(std::size_t entries);
    [[gnu::cold]] ~PlatformIOContext();
    static thread_local PlatformIOContext *instance;

    void reserveBuffers(std::size_t nbufs);
    std::size_t addBuffers(std::span<std::span<char> const> bufs);
    void reserveFiles(std::size_t nfiles);
    std::size_t addFiles(std::span<int const> files);

    std::size_t hasPendingEvents() const noexcept {
        return mNumSqesPending != 0;
    }

private:
    struct io_uring mRing;
    std::size_t mNumSqesPending = 0;
    std::unique_ptr<struct iovec[]> mBuffers;
    unsigned int mNumBufs = 0;
    unsigned int mCapBufs = 0;
    std::unique_ptr<int[]> mFiles;
    unsigned int mNumFiles = 0;
    unsigned int mCapFiles = 0;
};

struct [[nodiscard]] UringOp {
    UringOp() {
        mSqe = PlatformIOContext::instance->getSqe();
        io_uring_sqe_set_data(mSqe, this);
    }

    UringOp(UringOp &&) = delete;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) {
            mOp->mPrevious = coroutine;
            mOp->mRes = -ENOSYS;
        }

        int await_resume() const noexcept {
            return mOp->mRes;
        }

        UringOp *mOp;
    };

    Awaiter operator co_await() {
        return Awaiter{this};
    }

    static UringOp &&link_ops(UringOp &&lhs, UringOp &&rhs) {
        lhs.mSqe->flags |= IOSQE_IO_LINK;
        rhs.mPrevious = std::noop_coroutine();
        return std::move(lhs);
    }

    struct io_uring_sqe *getSqe() const noexcept {
        return mSqe;
    }

private:
    std::coroutine_handle<> mPrevious;

    union {
        int mRes;
        struct io_uring_sqe *mSqe;
    };

    friend PlatformIOContext;

    struct DoNotConstruct {};

    explicit UringOp(DoNotConstruct) {}

public:
    void startDetach() {
        static thread_local UringOp detachedOp{DoNotConstruct{}};
        detachedOp.mPrevious = std::noop_coroutine();
        io_uring_sqe_set_data(mSqe, &detachedOp);
    }

    UringOp &&prep_nop() && {
        io_uring_prep_nop(mSqe);
        return std::move(*this);
    }

    UringOp &&prep_openat(int dirfd, char const *path, int flags,
                          mode_t mode) && {
        io_uring_prep_openat(mSqe, dirfd, path, flags, mode);
        return std::move(*this);
    }

    UringOp &&prep_openat_direct(int dirfd, char const *path, int flags,
                                 mode_t mode, unsigned int file_index) && {
        io_uring_prep_openat_direct(mSqe, dirfd, path, flags, mode, file_index);
        return std::move(*this);
    }

    UringOp &&prep_socket(int domain, int type, int protocol,
                          unsigned int flags) && {
        io_uring_prep_socket(mSqe, domain, type, protocol, flags);
        return std::move(*this);
    }

    UringOp &&prep_socket_direct(int domain, int type, int protocol,
                                 unsigned int flags,
                                 unsigned int file_index) && {
        io_uring_prep_socket_direct(mSqe, domain, type, protocol, flags,
                                    file_index);
        return std::move(*this);
    }

    UringOp &&prep_accept(int fd, struct sockaddr *addr, socklen_t *addrlen,
                          int flags) && {
        io_uring_prep_accept(mSqe, fd, addr, addrlen, flags);
        return std::move(*this);
    }

    UringOp &&prep_accept_direct(int fd, struct sockaddr *addr,
                                 socklen_t *addrlen, int flags,
                                 unsigned int file_index) && {
        io_uring_prep_accept_direct(mSqe, fd, addr, addrlen, flags, file_index);
        return std::move(*this);
    }

    UringOp &&prep_connect(int fd, const struct sockaddr *addr,
                           socklen_t addrlen) && {
        io_uring_prep_connect(mSqe, fd, addr, addrlen);
        return std::move(*this);
    }

    UringOp &&prep_mkdirat(int dirfd, char const *path, mode_t mode) && {
        io_uring_prep_mkdirat(mSqe, dirfd, path, mode);
        return std::move(*this);
    }

    UringOp &&prep_linkat(int olddirfd, char const *oldpath, int newdirfd,
                          char const *newpath, int flags) && {
        io_uring_prep_linkat(mSqe, olddirfd, oldpath, newdirfd, newpath, flags);
        return std::move(*this);
    }

    UringOp &&prep_renameat(int olddirfd, char const *oldpath, int newdirfd,
                            char const *newpath, unsigned int flags) && {
        io_uring_prep_renameat(mSqe, olddirfd, oldpath, newdirfd, newpath,
                               flags);
        return std::move(*this);
    }

    UringOp &&prep_unlinkat(int dirfd, char const *path, int flags = 0) && {
        io_uring_prep_unlinkat(mSqe, dirfd, path, flags);
        return std::move(*this);
    }

    UringOp &&prep_symlinkat(char const *target, int newdirfd,
                             char const *linkpath) && {
        io_uring_prep_symlinkat(mSqe, target, newdirfd, linkpath);
        return std::move(*this);
    }

    UringOp &&prep_statx(int dirfd, char const *path, int flags,
                         unsigned int mask, struct statx *statxbuf) && {
        io_uring_prep_statx(mSqe, dirfd, path, flags, mask, statxbuf);
        return std::move(*this);
    }

    UringOp &&prep_read(int fd, std::span<char> buf, std::uint64_t offset) && {
        io_uring_prep_read(mSqe, fd, buf.data(),
                           static_cast<unsigned int>(buf.size()), offset);
        return std::move(*this);
    }

    UringOp &&prep_write(int fd, std::span<char const> buf,
                         std::uint64_t offset) && {
        io_uring_prep_write(mSqe, fd, buf.data(),
                            static_cast<unsigned int>(buf.size()), offset);
        return std::move(*this);
    }

    UringOp &&prep_read_fixed(int fd, std::span<char> buf, std::uint64_t offset,
                              int buf_index) && {
        io_uring_prep_read_fixed(mSqe, fd, buf.data(),
                                 static_cast<unsigned int>(buf.size()), offset,
                                 buf_index);
        return std::move(*this);
    }

    UringOp &&prep_write_fixed(int fd, std::span<char const> buf,
                               std::uint64_t offset, int buf_index) && {
        io_uring_prep_write_fixed(mSqe, fd, buf.data(),
                                  static_cast<unsigned int>(buf.size()), offset,
                                  buf_index);
        return std::move(*this);
    }

    UringOp &&prep_readv(int fd, std::span<struct iovec const> buf,
                         std::uint64_t offset, int flags) && {
        io_uring_prep_readv2(mSqe, fd, buf.data(),
                             static_cast<unsigned int>(buf.size()), offset,
                             flags);
        return std::move(*this);
    }

    UringOp &&prep_writev(int fd, std::span<struct iovec const> buf,
                          std::uint64_t offset, int flags) && {
        io_uring_prep_writev2(mSqe, fd, buf.data(),
                              static_cast<unsigned int>(buf.size()), offset,
                              flags);
        return std::move(*this);
    }

    UringOp &&prep_recv(int fd, std::span<char> buf, int flags) && {
        io_uring_prep_recv(mSqe, fd, buf.data(), buf.size(), flags);
        return std::move(*this);
    }

    UringOp &&prep_send(int fd, std::span<char const> buf, int flags) && {
        io_uring_prep_send(mSqe, fd, buf.data(), buf.size(), flags);
        return std::move(*this);
    }

    UringOp &&prep_send_zc(int fd, std::span<char const> buf, int flags,
                           unsigned int zc_flags) && {
        io_uring_prep_send_zc(mSqe, fd, buf.data(), buf.size(), flags,
                              zc_flags);
        return std::move(*this);
    }

    UringOp &&prep_send_zc_fixed(int fd, std::span<char const> buf, int flags,
                                 unsigned int zc_flags,
                                 unsigned int buf_index) && {
        io_uring_prep_send_zc_fixed(mSqe, fd, buf.data(), buf.size(), flags,
                                    zc_flags, buf_index);
        return std::move(*this);
    }

    UringOp &&prep_recvmsg(int fd, struct msghdr *msg, unsigned int flags) && {
        io_uring_prep_recvmsg(mSqe, fd, msg, flags);
        return std::move(*this);
    }

    UringOp &&prep_sendmsg(int fd, struct msghdr *msg, unsigned int flags) && {
        io_uring_prep_sendmsg(mSqe, fd, msg, flags);
        return std::move(*this);
    }

    UringOp &&prep_close(int fd) && {
        io_uring_prep_close(mSqe, fd);
        return std::move(*this);
    }

    UringOp &&prep_shutdown(int fd, int how) && {
        io_uring_prep_shutdown(mSqe, fd, how);
        return std::move(*this);
    }

    UringOp &&prep_fsync(int fd, unsigned int flags) && {
        io_uring_prep_fsync(mSqe, fd, flags);
        return std::move(*this);
    }

    UringOp &&prep_ftruncate(int fd, loff_t len) && {
        io_uring_prep_ftruncate(mSqe, fd, len);
        return std::move(*this);
    }

    UringOp &&prep_cancel(UringOp *op, int flags) && {
        io_uring_prep_cancel(mSqe, op, flags);
        return std::move(*this);
    }

    UringOp &&prep_cancel_fd(int fd, unsigned int flags) && {
        io_uring_prep_cancel_fd(mSqe, fd, flags);
        return std::move(*this);
    }

    UringOp &&prep_waitid(idtype_t idtype, id_t id, siginfo_t *infop,
                          int options, unsigned int flags) && {
        io_uring_prep_waitid(mSqe, idtype, id, infop, options, flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout(struct __kernel_timespec *ts, unsigned int count,
                           unsigned int flags) && {
        io_uring_prep_timeout(mSqe, ts, count, flags);
        return std::move(*this);
    }

    UringOp &&prep_link_timeout(struct __kernel_timespec *ts,
                                unsigned int flags) && {
        io_uring_prep_link_timeout(mSqe, ts, flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout_update(UringOp *op, struct __kernel_timespec *ts,
                                  unsigned int flags) && {
        io_uring_prep_timeout_update(
            mSqe, ts, reinterpret_cast<std::uintptr_t>(op), flags);
        return std::move(*this);
    }

    UringOp &&prep_timeout_remove(UringOp *op, unsigned int flags) && {
        io_uring_prep_timeout_remove(mSqe, reinterpret_cast<std::uintptr_t>(op),
                                     flags);
        return std::move(*this);
    }

    UringOp &&prep_splice(int fd_in, std::int64_t off_in, int fd_out,
                          std::int64_t off_out, std::size_t nbytes,
                          unsigned int flags) && {
        io_uring_prep_splice(mSqe, fd_in, off_in, fd_out, off_out,
                             static_cast<unsigned int>(nbytes), flags);
        return std::move(*this);
    }

    UringOp &&prep_futex_wait(uint32_t *futex, uint64_t val, uint64_t mask,
                              uint32_t futex_flags, unsigned int flags) && {
        io_uring_prep_futex_wait(mSqe, futex, val, mask, futex_flags, flags);
        return std::move(*this);
    }

    UringOp &&prep_futex_waitv(std::span<struct futex_waitv> futex,
                               unsigned int flags) && {
        io_uring_prep_futex_waitv(mSqe, futex.data(),
                                  static_cast<uint32_t>(futex.size()), flags);
        return std::move(*this);
    }

    UringOp &&prep_futex_wake(uint32_t *futex, uint64_t val, uint64_t mask,
                              uint32_t futex_flags, unsigned int flags) && {
        io_uring_prep_futex_wake(mSqe, futex, val, mask, futex_flags, flags);
        return std::move(*this);
    }

    // struct Canceller {
    //     using OpType = UringOp;
    //
    //     static Task<> doCancel(OpType *op) {
    //         co_await UringOp().prep_cancel(op, IORING_ASYNC_CANCEL_ALL);
    //     }
    // };

    Task<int> cancelGuard(CancelToken cancel) && {
        CancelCallback _(cancel, [this]() -> Task<> {
            co_await UringOp().prep_cancel(this, IORING_ASYNC_CANCEL_ALL);
        });
        co_return co_await std::move(*this);
    }

    // UringOp &&cancelGuard(CancelToken) && {
    //     return std::move(*this);
    // }
};

} // namespace co_async








namespace co_async {
struct IOContextOptions {
    std::chrono::steady_clock::duration maxSleep =
        std::chrono::milliseconds(200);
    std::optional<std::size_t> threadAffinity = std::nullopt;
    std::size_t queueEntries = 512;
};

struct alignas(hardware_destructive_interference_size) IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::chrono::steady_clock::duration mMaxSleep;

public:
    explicit IOContext(IOContextOptions options = {});
    IOContext(IOContext &&) = delete;
    ~IOContext();

    [[gnu::hot]] void run();
    [[gnu::hot]] bool runOnce();

    // [[gnu::hot]] void spawn(std::coroutine_handle<> coroutine) {
    //     coroutine.resume();
    // }
    //
    // template <class T, class P>
    // void spawn(Task<T, P> task) {
    //     auto wrapped = coSpawnStarter(std::move(task));
    //     wrapped.get().resume();
    //     wrapped.release();
    // }
    //
    // template <class T, class P>
    // T join(Task<T, P> task) {
    //     return contextJoin(*this, std::move(task));
    // }

    static thread_local IOContext *instance;
};

inline Task<> co_catch(Task<Expected<>> task) {
    auto ret = co_await task;
    if (ret.has_error()) {
        std::cerr << ret.error().category().name() << " error: " << ret.error().message() << " (" << ret.error().value() << ")\n";
    }
    co_return;
}

inline void co_main(Task<Expected<>> main) {
    IOContext ctx;
    co_spawn(co_catch(std::move(main)));
    ctx.run();
}

inline void co_main(Task<> main) {
    IOContext ctx;
    co_spawn(std::move(main));
    ctx.run();
}

// template <class T, class P>
// inline Task<> contextJoinHelper(Task<T, P> task, std::condition_variable &cv,
//                                 Uninitialized<T> &result
// #if CO_ASYNC_EXCEPT
//                                 ,
//                                 std::exception_ptr exception
// #endif
// ) {
// #if CO_ASYNC_EXCEPT
//     try {
// #endif
//         result.emplace((co_await task, Void()));
// #if CO_ASYNC_EXCEPT
//     } catch (...) {
// # if CO_ASYNC_DEBUG
//         std::cerr << "WARNING: exception occurred in IOContext::join\n";
// # endif
//         exception = std::current_exception();
//     }
// #endif
//     cv.notify_one();
// }
//
// template <class T, class P>
// T contextJoin(IOContext &context, Task<T, P> task) {
//     std::condition_variable cv;
//     std::mutex mtx;
//     Uninitialized<T> result;
// #if CO_ASYNC_EXCEPT
//     std::exception_ptr exception;
// #endif
//     context.spawn(contextJoinHelper(std::move(task), cv, result
// #if CO_ASYNC_EXCEPT
//                                     ,
//                                     exception
// #endif
//                                     ));
//     std::unique_lock lck(mtx);
//     cv.wait(lck);
//     lck.unlock();
// #if CO_ASYNC_EXCEPT
//     if (exception) [[unlikely]] {
//         std::rethrow_exception(exception);
//     }
// #endif
//     if constexpr (!std::is_void_v<T>) {
//         return result.move();
//     }
// }
//
// inline auto co_resume_on(IOContext &context) {
//     struct ResumeOnAwaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             mContext.spawn(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         IOContext &mContext;
//     };
//
//     return ResumeOnAwaiter(context);
// }
} // namespace co_async








namespace co_async {
inline Task<Expected<>, GenericIOContext::TimerNode>
coSleep(std::chrono::steady_clock::time_point expires) {
    co_return co_await GenericIOContext::TimerNode::Awaiter(expires);
}

inline Task<Expected<>>
co_sleep(std::chrono::steady_clock::time_point expires) {
    auto task = coSleep(expires);
    CancelCallback _(co_await co_cancel, [p = &task.promise()] {
        p->doCancel();
        std::coroutine_handle<GenericIOContext::TimerNode>::from_promise(*p).resume();
    });
    co_return co_await task;
}

inline Task<Expected<>, GenericIOContext::TimerNode>
coSleep(std::chrono::steady_clock::duration timeout) {
    return coSleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<Expected<>> co_sleep(std::chrono::steady_clock::duration timeout) {
    return co_sleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<> coForever() {
    co_await std::suspend_always();
#if defined(__GNUC__) && defined(__has_builtin)
# if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
# endif
#endif
}

inline Task<> co_forever() {
    struct ForeverAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) noexcept {
            mPrevious = coroutine;
        }

        void await_resume() const noexcept {}

        std::coroutine_handle<> mPrevious;
    };

    ForeverAwaiter awaiter;
    CancelCallback _(co_await co_cancel,
                     [&awaiter] { co_spawn(awaiter.mPrevious); });
    co_return co_await awaiter;
}
} // namespace co_async










namespace co_async {

template <class T>
struct WhenAnyResult {
    T value;
    std::size_t index;
};

template <Awaitable T, class Alloc = std::allocator<T>>
Task<WhenAnyResult<typename AwaitableTraits<T>::AvoidRetType>>
when_any(std::vector<T, Alloc> const &tasks) {
    CancelSource cancel(co_await co_cancel);
    std::vector<Task<>, Alloc> newTasks(tasks.size(), tasks.get_allocator());
    std::optional<typename AwaitableTraits<T>::RetType> result;
    std::size_t index = static_cast<std::size_t>(-1);
    std::size_t i = 0;
    for (auto &&task: tasks) {
        newTasks.push_back(co_cancel.bind(
            cancel,
            co_bind([&, i, cancel = cancel.token()]() mutable -> Task<> {
                result.emplace((co_await std::move(task), Void()));
                if (cancel.is_canceled()) {
                    co_return;
                }
                co_await cancel.cancel();
                index = i;
            })));
        ++i;
    }
    co_await when_all(newTasks);
    co_return {std::move(result.value()), index};
}

template <Awaitable... Ts>
Task<std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
when_any(Ts &&...tasks) {
    return co_bind(
        [&]<std::size_t... Is>(std::index_sequence<Is...>)
            -> Task<
                std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>> {
            CancelSource cancel(co_await co_cancel);
            std::optional<
                std::variant<typename AwaitableTraits<Ts>::AvoidRetType...>>
                result;
            co_await when_all(co_cancel.bind(
                cancel,
                co_bind([&result, task = std::move(tasks)]() mutable -> Task<> {
                    auto res = (co_await std::move(task), Void());
                    if (co_await co_cancel) {
                        co_return;
                    }
                    co_await co_cancel.cancel();
                    result.emplace(std::in_place_index<Is>, std::move(res));
                }))...);
            co_return std::move(result.value());
        },
        std::make_index_sequence<sizeof...(Ts)>());
}

template <Awaitable... Ts, class Common = std::common_type_t<
                               typename AwaitableTraits<Ts>::AvoidRetType...>>
Task<WhenAnyResult<Common>> when_any_common(Ts &&...tasks) {
    return co_bind(
        [&]<std::size_t... Is>(
            std::index_sequence<Is...>) -> Task<WhenAnyResult<Common>> {
            CancelSource cancel(co_await co_cancel);
            std::size_t index = static_cast<std::size_t>(-1);
            std::optional<Common> result;
            co_await when_all(co_cancel.bind(
                cancel, co_bind([&index, &result,
                                 task = std::move(tasks)]() mutable -> Task<> {
                    auto res = (co_await std::move(task), Void());
                    if (co_await co_cancel) {
                        co_return;
                    }
                    co_await co_cancel.cancel();
                    index = Is;
                    result.emplace(std::move(res));
                }))...);
            co_return WhenAnyResult<Common>{std::move(result.value()), index};
        },
        std::make_index_sequence<sizeof...(Ts)>());
}

template <Awaitable A, class Timeout>
Task<std::conditional_t<
    std::convertible_to<std::errc, typename AwaitableTraits<A>::RetType> &&
        !std::is_void_v<typename AwaitableTraits<A>::RetType>,
    typename AwaitableTraits<A>::RetType,
    Expected<typename AwaitableTraits<A>::RetType>>>
co_timeout(A &&a, Timeout timeout) {
    auto res = co_await when_any(std::forward<A>(a), co_sleep(timeout));
    if (auto ret = std::get_if<0>(&res)) {
        if constexpr (std::is_void_v<typename AwaitableTraits<A>::RetType>) {
            co_return {};
        } else {
            co_return std::move(*ret);
        }
    } else {
        co_return std::errc::stream_timeout;
    }
}

} // namespace co_async






#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace co_async {

inline constexpr std::size_t kFutexNotifyAll = static_cast<std::size_t>(std::numeric_limits<int>::max());

template <class T>
inline constexpr uint32_t getFutexFlagsFor() {
    switch (sizeof(T)) {
#ifdef FUTEX2_PRIVATE
    case sizeof(uint8_t):  return FUTEX2_SIZE_U8 | FUTEX2_PRIVATE;
    case sizeof(uint16_t): return FUTEX2_SIZE_U16 | FUTEX2_PRIVATE;
    case sizeof(uint32_t): return FUTEX2_SIZE_U32 | FUTEX2_PRIVATE;
    case sizeof(uint64_t): return FUTEX2_SIZE_U64 | FUTEX2_PRIVATE;
#else
    case sizeof(uint8_t):  return 0 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint16_t): return 1 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint32_t): return 2 | FUTEX_PRIVATE_FLAG;
    case sizeof(uint64_t): return 3 | FUTEX_PRIVATE_FLAG;
#endif
    }
}

template <class T>
inline constexpr uint64_t futexValueExtend(T value) {
    static_assert(std::is_trivial_v<T> && sizeof(T) <= sizeof(uint64_t));
    uint64_t ret = 0;
    std::memcpy(&ret, &value, sizeof(T));
    return ret;
}

template <class T>
inline Expected<> futex_notify_sync(std::atomic<T> *futex,
                                    std::size_t count = kFutexNotifyAll,
                                    uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#ifndef SYS_futex_wake
    const long SYS_futex_wake = 454;
#endif
    long res = syscall(SYS_futex_wake, reinterpret_cast<uint32_t *>(futex),
            static_cast<uint64_t>(count), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
#if CO_ASYNC_INVALFIX
    if (res == -EBADF || res == -ENOSYS) {
        res = syscall(SYS_futex, reinterpret_cast<uint32_t *>(futex), FUTEX_WAKE_BITSET_PRIVATE,
                static_cast<uint32_t>(count), nullptr, nullptr, mask);
    }
#endif
    return expectError(static_cast<int>(res));
}

template <class T>
inline Expected<> futex_wait_sync(std::atomic<T> *futex,
                            std::type_identity_t<T> val,
                            uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#ifndef SYS_futex_wait
    const long SYS_futex_wait = 455;
#endif
    long res = syscall(SYS_futex_wait, reinterpret_cast<uint32_t *>(futex),
            futexValueExtend(val), static_cast<uint64_t>(mask),
            getFutexFlagsFor<T>());
#if CO_ASYNC_INVALFIX
    if (res == -EBADF || res == -ENOSYS) {
        res = syscall(SYS_futex, reinterpret_cast<uint32_t *>(futex), FUTEX_WAIT_BITSET_PRIVATE,
                static_cast<uint32_t>(futexValueExtend(val)), nullptr, nullptr, mask);
    }
#endif
    return expectError(static_cast<int>(res));
}


template <class T>
inline Task<Expected<>> futex_wait(std::atomic<T> *futex,
                                   std::type_identity_t<T> val,
                                   uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(
        co_await UringOp()
            .prep_futex_wait(reinterpret_cast<uint32_t *>(futex),
                             futexValueExtend(val), static_cast<uint64_t>(mask),
                             getFutexFlagsFor<T>(), 0)
            .cancelGuard(co_await co_cancel)).transform([] (int) {})
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] {
            return futex_wait_sync(futex, val, mask);
        })
#endif
        .ignore_error(std::errc::resource_unavailable_try_again)
    ;
}

template <class T>
inline Task<Expected<>>
futex_notify_async(std::atomic<T> *futex,
                   std::size_t count = kFutexNotifyAll,
                   uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
    co_return expectError(
        co_await UringOp()
            .prep_futex_wake(reinterpret_cast<uint32_t *>(futex),
                             static_cast<uint64_t>(count),
                             static_cast<uint64_t>(mask), getFutexFlagsFor<T>(),
                             0)
            .cancelGuard(co_await co_cancel)).transform([] (int) {})
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] {
            return futex_notify_sync(futex, count, mask);
        })
#endif
    ;
}

template <class T>
inline void futex_notify(std::atomic<T> *futex,
                         std::size_t count = kFutexNotifyAll,
                         uint32_t mask = FUTEX_BITSET_MATCH_ANY) {
#if CO_ASYNC_INVALFIX
    futex_notify_sync(futex, count, mask);
#else
    UringOp()
        .prep_futex_wake(reinterpret_cast<uint32_t *>(futex),
                         static_cast<uint64_t>(count),
                         static_cast<uint64_t>(mask), getFutexFlagsFor<T>(), 0)
        .startDetach();
#endif
}

#if CO_ASYNC_INVALFIX
template <class>
using FutexAtomic = std::atomic<uint32_t>;
#else
template <class T>
using FutexAtomic = std::atomic<T>;
#endif

} // namespace co_async











namespace co_async {
// struct TimedConditionVariable {
// private:
//     struct PromiseNode : CustomPromise<void, PromiseNode>,
//                          IntrusiveList<PromiseNode>::NodeType {
//         void doCancel() {
//             mCancelled = true;
//             this->erase_from_parent();
//             co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
//         }
//
//         bool mCancelled = false;
//     };
//
//     IntrusiveList<PromiseNode> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<PromiseNode> coroutine)
//         const {
//             mThat->pushWaiting(coroutine.promise());
//         }
//
//         void await_resume() const noexcept {}
//
//         TimedConditionVariable *mThat;
//     };
//
//     PromiseNode *popWaiting() {
//         return mWaitingList.pop_front();
//     }
//
//     void pushWaiting(PromiseNode &promise) {
//         mWaitingList.push_back(promise);
//     }
//
//     /* struct Canceller { */
//     /*     using OpType = Task<void, PromiseNode>; */
//     /*  */
//     /*     static Task<> doCancel(OpType *op) { */
//     /*         op->get().promise().doCancel(); */
//     /*         co_return; */
//     /*     } */
//     /*  */
//     /*     static void earlyCancelValue(OpType *op) noexcept {} */
//     /* }; */
//
// public:
//     Task<void, PromiseNode> waitNotCancellable() {
//         co_await Awaiter(this);
//     }
//
//     Task<Expected<>> wait() {
//         auto waiter = waitNotCancellable();
//         CancelCallback _(co_await co_cancel, [&] {
//             waiter.promise().doCancel();
//         });
//         co_await waiter;
//         if (waiter.promise().mCancelled) {
//             co_return std::errc::operation_canceled;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
//         auto res =
//             co_await when_any(wait(), co_sleep(expires));
//         if (auto r = std::get_if<0>(&res)) {
//             co_return std::move(*r);
//         }
//         co_return std::errc::stream_timeout;
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
//         return wait(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>> wait_predicate(std::invocable<> auto &&pred) {
//         while (!std::invoke(pred)) {
//             co_await co_await wait();
//         }
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::time_point expires) {
//         while (!std::invoke(pred)) {
//             if (co_await co_cancel) {
//                 co_return std::errc::operation_canceled;
//             }
//             if (std::chrono::steady_clock::now() > expires) {
//                 co_return std::errc::stream_timeout;
//             }
//             co_await co_await wait();
//         }
//         co_return {};
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::duration timeout) {
//         return wait_predicate(std::forward<decltype(pred)>(pred),
//                               std::chrono::steady_clock::now() + timeout);
//     }
//
//     void notify() {
//         while (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     void notify_one() {
//         if (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (auto promise = popWaiting()) {
//             return
//             std::coroutine_handle<PromiseNode>::from_promise(*promise);
//         }
//         return nullptr;
//     }
// };
//
// struct ConditionVariable {
// private:
//     std::deque<std::coroutine_handle<>> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             mThat->mWaitingList.push_back(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         ConditionVariable *mThat;
//     };
//
// public:
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         while (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             co_spawn(coroutine);
//         }
//     }
//
//     void notify_one() {
//         if (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             co_spawn(coroutine);
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (!mWaitingList.empty()) {
//             auto coroutine = mWaitingList.front();
//             mWaitingList.pop_front();
//             return coroutine;
//         }
//         return nullptr;
//     }
// };
//
// struct OneshotConditionVariable {
// private:
//     std::coroutine_handle<> mWaitingCoroutine{nullptr};
//     bool mReady{false};
//
// public:
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return mThat->mReady;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
// #if CO_ASYNC_DEBUG
//             if (mThat->mWaitingCoroutine) [[unlikely]] {
//                 throw std::logic_error(
//                     "please do not co_await on the same "
//                     "OneshotConditionVariable or Future for multiple times");
//             }
// #endif
//             mThat->mWaitingCoroutine = coroutine;
//         }
//
//         void await_resume() const noexcept {
// #if CO_ASYNC_DEBUG
//             if (!mThat->mReady) [[unlikely]] {
//                 throw std::logic_error("OneshotConditionVariable or Future "
//                                        "waked up but not ready");
//             }
// #endif
//             mThat->mReady = false;
//         }
//
//         OneshotConditionVariable *mThat;
//     };
//
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         mReady = true;
//         if (auto coroutine = mWaitingCoroutine) {
//             mWaitingCoroutine = nullptr;
//             co_spawn(coroutine);
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         mReady = true;
//         if (auto coroutine = mWaitingCoroutine) {
//             mWaitingCoroutine = nullptr;
//             return coroutine;
//         }
//         return nullptr;
//     }
// };
//
// struct ConcurrentConditionVariable {
// private:
//     struct WaitEntry {
//         std::coroutine_handle<> coroutine;
//         GenericIOContext *context;
//     };
//
//     std::deque<WaitEntry> mWaitingList;
//     std::mutex mMutex;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             std::lock_guard lock(mThat->mMutex);
//             mThat->mWaitingList.emplace_back(coroutine,
//                                              GenericIOContext::instance);
//         }
//
//         void await_resume() const noexcept {}
//
//         ConcurrentConditionVariable *mThat;
//     };
//
// public:
//     Awaiter operator co_await() noexcept {
//         return Awaiter(this);
//     }
//
//     Task<> wait() {
//         co_await Awaiter(this);
//     }
//
//     void notify() {
//         while (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             waitEntry.context->enqueueJobMT(waitEntry.coroutine);
//         }
//     }
//
//     void notify_one() {
//         if (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             waitEntry.context->enqueueJobMT(waitEntry.coroutine);
//         }
//     }
//
//     std::optional<WaitEntry> notifyPopCoroutine() {
//         std::lock_guard lock(mMutex);
//         if (!mWaitingList.empty()) {
//             auto waitEntry = mWaitingList.front();
//             mWaitingList.pop_front();
//             return waitEntry;
//         }
//         return std::nullopt;
//     }
// };
//
// struct ConcurrentTimedConditionVariable {
// private:
//     struct PromiseNode : CustomPromise<void, PromiseNode>,
//                          ConcurrentIntrusiveList<PromiseNode>::NodeType {
//         void doCancel() {
//             mCancelled = true;
//             this->erase_from_parent();
//             co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
//         }
//
//         bool mCancelled = false;
//     };
//
//     ConcurrentIntrusiveList<PromiseNode> mWaitingList;
//
//     struct Awaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<PromiseNode> coroutine)
//         const {
//             mThat->pushWaiting(coroutine.promise());
//         }
//
//         void await_resume() const noexcept {}
//
//         ConcurrentTimedConditionVariable *mThat;
//     };
//
//     PromiseNode *popWaiting() {
//         return mWaitingList.pop_front();
//     }
//
//     void pushWaiting(PromiseNode &promise) {
//         return mWaitingList.push_back(promise);
//     }
//
//     /* struct Canceller { */
//     /*     using OpType = Task<void, PromiseNode>; */
//     /*  */
//     /*     static Task<> doCancel(OpType *op) { */
//     /*         op->get().promise().doCancel(); */
//     /*         co_return; */
//     /*     } */
//     /*  */
//     /*     static void earlyCancelValue(OpType *op) noexcept {} */
//     /* }; */
//
// public:
//     Task<void, PromiseNode> waitNotCancellable() {
//         co_await Awaiter(this);
//     }
//
//     Task<Expected<>> wait() {
//         auto waiter = waitNotCancellable();
//         CancelCallback _(co_await co_cancel, [&] {
//             waiter.promise().doCancel();
//         });
//         co_await waiter;
//         if (waiter.promise().mCancelled) {
//             co_return std::errc::operation_canceled;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
//         auto res =
//             co_await when_any(wait(), co_sleep(expires));
//         if (res.index() != 0) {
//             co_return std::errc::stream_timeout;
//         }
//         co_return {};
//     }
//
//     Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
//         return wait(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::time_point expires) {
//         while (!std::invoke(pred)) {
//             if (co_await co_cancel) {
//                 co_return std::errc::operation_canceled;
//             }
//             if (std::chrono::steady_clock::now() > expires) {
//                 co_return std::errc::stream_timeout;
//             }
//             co_await co_await wait();
//         }
//         co_return {};
//     }
//
//     Task<Expected<>>
//     wait_predicate(std::invocable<> auto &&pred,
//                    std::chrono::steady_clock::duration timeout) {
//         return wait_predicate(std::forward<decltype(pred)>(pred),
//                               std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<Expected<>> wait_predicate(std::invocable<> auto &&pred) {
//         while (!std::invoke(pred)) {
//             co_await co_await wait();
//         }
//     }
//
//     void notify() {
//         while (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     void notify_one() {
//         if (auto promise = popWaiting()) {
//             co_spawn(
//                 std::coroutine_handle<PromiseNode>::from_promise(*promise));
//         }
//     }
//
//     std::coroutine_handle<> notifyPopCoroutine() {
//         if (auto promise = popWaiting()) {
//             return
//             std::coroutine_handle<PromiseNode>::from_promise(*promise);
//         }
//         return nullptr;
//     }
// };

struct ConditionVariable {
private:
    FutexAtomic<std::uint32_t> mFutex{};

public:
    Task<Expected<>> wait() {
        std::uint32_t old = mFutex.load(std::memory_order_relaxed);
        do {
            co_await co_await futex_wait(&mFutex, old);
        } while (mFutex.load(std::memory_order_acquire) == old);
        co_return {};
    }

    void notify_one() {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, 1);
    }

    void notify_all() {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, kFutexNotifyAll);
    }

    using Mask = std::uint32_t;

    Task<Expected<>> wait(Mask mask) {
        std::uint32_t old = mFutex.load(std::memory_order_relaxed);
        do {
            co_await co_await futex_wait(&mFutex, old, mask);
        } while (mFutex.load(std::memory_order_acquire) == old);
        co_return {};
    }

    void notify_one(Mask mask) {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, 1, mask);
    }

    void notify_all(Mask mask) {
        mFutex.fetch_add(1, std::memory_order_release);
        futex_notify(&mFutex, kFutexNotifyAll, mask);
    }
};

} // namespace co_async








namespace co_async {

struct Semaphore {
private:
    FutexAtomic<std::uint32_t> mCounter;
    std::uint32_t const mMaxCount;

    static constexpr std::uint32_t kAcquireMask = 1;
    static constexpr std::uint32_t kReleaseMask = 2;

public:
    explicit Semaphore(std::uint32_t maxCount, std::uint32_t initialCount)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::uint32_t count() const noexcept {
        return mCounter.load(std::memory_order_relaxed);
    }

    std::uint32_t max_count() const noexcept {
        return mMaxCount;
    }

    Task<Expected<>> acquire() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == 0) {
                co_await co_await futex_wait(&mCounter, count, kAcquireMask);
                count = mCounter.load(std::memory_order_relaxed);
            }
        } while (mCounter.compare_exchange_weak(count, count - 1,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed));
        futex_notify(&mCounter, 1, kReleaseMask);
        co_return {};
    }

    Task<Expected<>> release() {
        std::uint32_t count = mCounter.load(std::memory_order_relaxed);
        do {
            while (count == mMaxCount) {
                co_await co_await futex_wait(&mCounter, count, kReleaseMask);
                count = mCounter.load(std::memory_order_relaxed);
            }
        } while (mCounter.compare_exchange_weak(count, count + 1,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed));
        futex_notify(&mCounter, 1, kAcquireMask);
        co_return {};
    }
};
} // namespace co_async







namespace co_async {
// struct BasicMutex {
//     ConditionVariable mReady;
//     std::atomic_bool mLocked;
//
//     bool try_lock() {
//         bool expect = false;
//         return mLocked.compare_exchange_strong(
//             expect, true, std::memory_order_acquire,
//             std::memory_order_relaxed);
//     }
//
//     Task<> lock() {
//         while (!try_lock()) {
//             co_await mReady.wait();
//         }
//     }
//
//     void unlock() {
//         mLocked.store(false, std::memory_order_release);
//         mReady.notify_one();
//     }
// };
//
// struct BasicConcurrentMutex {
//     ConcurrentConditionVariable mReady;
//     std::atomic_bool mLocked;
//
//     bool try_lock() {
//         bool expect = false;
//         return mLocked.compare_exchange_strong(
//             expect, true, std::memory_order_acquire,
//             std::memory_order_relaxed);
//     }
//
//     Task<> lock() {
//         while (!try_lock()) {
//             co_await mReady.wait();
//         }
//     }
//
//     void unlock() {
//         mLocked.store(false, std::memory_order_release);
//         mReady.notify_one();
//     }
// };
//
// struct BasicTimedMutex {
//     TimedConditionVariable mReady;
//     std::atomic_bool mLocked;
//
//     bool try_lock() {
//         bool expect = false;
//         return mLocked.compare_exchange_strong(
//             expect, true, std::memory_order_acquire,
//             std::memory_order_relaxed);
//     }
//
//     Task<> lock() {
//         while (!try_lock()) {
//             co_await mReady.wait();
//         }
//     }
//
//     Task<bool> try_lock(std::chrono::steady_clock::duration timeout) {
//         return try_lock(std::chrono::steady_clock::now() + timeout);
//     }
//
//     Task<bool> try_lock(std::chrono::steady_clock::time_point expires) {
//         while (!try_lock()) {
//             if (std::chrono::steady_clock::now() > expires ||
//                 !co_await mReady.wait(expires)) {
//                 co_return false;
//             }
//         }
//         co_return true;
//     }
//
//     void unlock() {
//         mLocked.store(false, std::memory_order_release);
//         mReady.notify_one();
//     }
// };

struct BasicMutex {
private:
    FutexAtomic<bool> mFutex;

public:
    bool try_lock() {
        bool old = mFutex.exchange(true, std::memory_order_acquire);
        return old == false;
    }

    Task<Expected<>> lock() {
        while (true) {
            bool old = mFutex.exchange(true, std::memory_order_acquire);
            if (old == false) {
                co_return {};
            }
            co_await co_await futex_wait(&mFutex, old);
        }
    }

    void unlock() {
        mFutex.store(false, std::memory_order_release);
        futex_notify(&mFutex, 1);
    }
};

template <class M, class T>
struct alignas(hardware_destructive_interference_size) MutexImpl {
private:
    M mMutex;
    T mValue;

public:
    struct Locked {
    private:
        explicit Locked(MutexImpl *impl) noexcept : mImpl(impl) {}

        friend MutexImpl;

    public:
        Locked() noexcept : mImpl(nullptr) {}

        T &operator*() const {
            return mImpl->unsafe_access();
        }

        T *operator->() const {
            return std::addressof(mImpl->unsafe_access());
        }

        explicit operator bool() const noexcept {
            return mImpl != nullptr;
        }

        void unlock() {
            if (mImpl) {
                mImpl->mMutex.unlock();
                mImpl = nullptr;
            }
        }

        Locked(Locked &&that) noexcept
            : mImpl(std::exchange(that.mImpl, nullptr)) {}

        Locked &operator=(Locked &&that) noexcept {
            std::swap(mImpl, that.mImpl);
            return *this;
        }

        ~Locked() {
            unlock();
        }

    private:
        MutexImpl *mImpl;
    };

    MutexImpl(MutexImpl &&) = delete;
    MutexImpl(MutexImpl const &) = delete;
    MutexImpl() = default;

    template <class... Args>
        requires(!std::is_void_v<T> && std::constructible_from<T, Args...>)
    explicit MutexImpl(Args &&...args)
        : mMutex(),
          mValue(std::forward<Args>(args)...) {}

    Locked try_lock() {
        if (auto e = mMutex.try_lock()) {
            return Locked(this);
        } else {
            return Locked();
        }
    }

    Task<Expected<Locked>> lock() {
        co_await co_await mMutex.lock();
        co_return Locked(this);
    }

    // Task<Locked> try_lock_for(std::chrono::steady_clock::duration timeout) {
    //     if (!co_await mMutex.try_lock_for(timeout)) {
    //         co_return Locked();
    //     }
    //     co_return Locked(this);
    // }
    //
    // Task<Locked> try_lock_until(std::chrono::steady_clock::time_point
    // expires) {
    //     if (!co_await mMutex.try_lock_for(expires)) {
    //         co_return Locked();
    //     }
    //     co_return Locked(this);
    // }

    T &unsafe_access() {
        return mValue;
    }

    T const &unsafe_access() const {
        return mValue;
    }

    M &unsafe_basic_mutex() {
        return mMutex;
    }

    M const &unsafe_basic_mutex() const {
        return mMutex;
    }
};

template <class M>
struct MutexImpl<M, void> : MutexImpl<M, Void> {
    using MutexImpl<M, Void>::MutexImpl;
};

template <class T = void>
struct Mutex : MutexImpl<BasicMutex, T> {
    using MutexImpl<BasicMutex, T>::MutexImpl;
};

struct CallOnce {
private:
    std::atomic_bool mCalled{false};
    Mutex<> mMutex;

public:
    struct Locked {
    private:
        explicit Locked(Mutex<>::Locked locked, CallOnce *impl) noexcept
            : mLocked(std::move(locked)),
              mImpl(impl) {}

        friend CallOnce;

    public:
        Locked() noexcept : mLocked(), mImpl(nullptr) {}

        explicit operator bool() const noexcept {
            return static_cast<bool>(mLocked);
        }

        void set_ready() const {
            mImpl->mCalled.store(true, std::memory_order_relaxed);
        }

    private:
        Mutex<>::Locked mLocked;
        CallOnce *mImpl;
    };

    Task<Locked> call_once() {
        if (mCalled.load(std::memory_order_relaxed)) {
            co_return Locked();
        }
        while (true) {
            if (auto mtxLock = co_await mMutex.lock()) {
                Locked locked(std::move(*mtxLock), this);
                if (mCalled.load(std::memory_order_relaxed)) {
                    co_return Locked();
                }
                co_return std::move(locked);
            }
        }
    }
};
} // namespace co_async









namespace co_async {

struct ThreadPool {
private:
    struct Thread;

    SpinMutex mWorkingMutex;
    std::list<Thread *> mWorkingThreads;
    SpinMutex mFreeMutex;
    std::list<Thread *> mFreeThreads;
    SpinMutex mThreadsMutex;
    std::list<Thread> mThreads;

    Thread *submitJob(std::function<void()> func);

public:
    Task<Expected<>> rawRun(std::function<void()> func) /* MT-safe */;
    Task<Expected<>> rawRun(std::function<void(std::stop_token)> func,
                            CancelToken cancel) /* MT-safe */;

    auto run(std::invocable auto func) /* MT-safe */
        -> Task<Expected<std::invoke_result_t<decltype(func)>>>
        requires(!std::invocable<decltype(func), std::stop_token>)
    {
        std::optional<Avoid<std::invoke_result_t<decltype(func)>>> res;
        co_await co_await rawRun([&res, func = std::move(func)]() mutable {
            res = (func(), Void());
        });
        if (!res) [[unlikely]] {
            co_return std::errc::operation_canceled;
        }
        co_return std::move(*res);
    }

    auto run(std::invocable<std::stop_token> auto func,
             CancelToken cancel) /* MT-safe */
        -> Task<
            Expected<std::invoke_result_t<decltype(func), std::stop_token>>> {
        std::optional<
            Avoid<std::invoke_result_t<decltype(func), std::stop_token>>>
            res;
        auto e = co_await rawRun(
            [&res, func = std::move(func)](std::stop_token stop) mutable {
                res = (func(stop), Void());
            });
        if (e.has_error()) {
            co_return CO_ASYNC_ERROR_FORWARD(e);
        }
        if (!res) {
            co_return std::errc::operation_canceled;
        }
        co_return std::move(*res);
    }

    auto run(std::invocable<std::stop_token> auto func) /* MT-safe */
        -> Task<
            Expected<std::invoke_result_t<decltype(func), std::stop_token>>> {
        co_return co_await run(func, co_await co_cancel);
    }

    std::size_t threads_count() /* MT-safe */;
    std::size_t working_threads_count() /* MT-safe */;

    ThreadPool();
    ~ThreadPool();
    ThreadPool &operator=(ThreadPool &&) = delete;
};

} // namespace co_async










namespace co_async {
template <class T>
struct Queue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mReady;

    static constexpr ConditionVariable::Mask kNonEmptyMask = 1;
    static constexpr ConditionVariable::Mask kNonFullMask = 2;

public:
    explicit Queue(std::size_t size) : mQueue(size) {}

    std::optional<T> try_pop() {
        bool wasFull = mQueue.full();
        auto value = mQueue.pop();
        if (value && wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        return value;
    }

    bool try_push(T &&value) {
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        if (ok && wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        return ok;
    }

    Task<Expected<>> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await co_await mReady.wait(kNonFullMask);
        }
        mReady.notify_one(kNonEmptyMask);
        co_return {};
    }

    Task<Expected<T>> pop() {
        while (true) {
            if (auto value = mQueue.pop()) {
                mReady.notify_one(kNonFullMask);
                co_return std::move(*value);
            }
            co_await co_await mReady.wait(kNonEmptyMask);
        }
    }
};

template <class T>
struct alignas(hardware_destructive_interference_size) ConcurrentQueue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mReady;
    SpinMutex mMutex;

    static constexpr ConditionVariable::Mask kNonEmptyMask = 1;
    static constexpr ConditionVariable::Mask kNonFullMask = 2;

public:
    explicit ConcurrentQueue(std::size_t maxSize = 0) : mQueue(maxSize) {}

    void set_max_size(std::size_t maxSize) {
        mQueue.set_max_size(maxSize);
    }

    ConcurrentQueue(ConcurrentQueue &&) = delete;

    std::optional<T> try_pop() {
        std::unique_lock lock(mMutex);
        bool wasFull = mQueue.full();
        auto value = mQueue.pop();
        lock.unlock();
        if (value && wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        return value;
    }

    bool try_push(T &&value) {
        std::unique_lock lock(mMutex);
        bool wasEmpty = mQueue.empty();
        bool ok = mQueue.push(std::move(value));
        lock.unlock();
        if (ok && wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        return ok;
    }

    Task<Expected<T>> pop() {
        std::unique_lock lock(mMutex);
        while (mQueue.empty()) {
            lock.unlock();
            co_await co_await mReady.wait(kNonEmptyMask);
            lock.lock();
        }
        bool wasFull = mQueue.full();
        T value = mQueue.pop_unchecked();
        lock.unlock();
        if (wasFull) {
            mReady.notify_one(kNonFullMask);
        }
        co_return std::move(value);
    }

    Task<Expected<>> push(T value) {
        std::unique_lock lock(mMutex);
        while (mQueue.full()) {
            lock.unlock();
            co_await co_await mReady.wait(kNonFullMask);
            lock.lock();
        }
        bool wasEmpty = mQueue.empty();
        mQueue.push_unchecked(std::move(value));
        lock.unlock();
        if (wasEmpty) {
            mReady.notify_one(kNonEmptyMask);
        }
        co_return {};
    }
};
} // namespace co_async








namespace co_async {
struct IOContextMT {
private:
    std::unique_ptr<IOContext[]> mWorkers;
    std::size_t mNumWorkers = 0;

public:
    IOContextMT();
    IOContextMT(IOContext &&) = delete;
    ~IOContextMT();

    static std::size_t get_worker_id(IOContext const &context) noexcept {
        return static_cast<std::size_t>(&context - instance->mWorkers.get());
    }

    static std::size_t this_worker_id() noexcept {
        return get_worker_id(*IOContext::instance);
    }

    static IOContext &nth_worker(std::size_t index) noexcept {
        return instance->mWorkers[index];
    }

    static std::size_t num_workers() noexcept {
        return instance->mNumWorkers;
    }

    static void run(std::size_t numWorkers = 0);

    // static void spawn(std::coroutine_handle<> coroutine,
    //                   std::size_t index = 0) {
    //     instance->mWorkers[index].spawn(coroutine);
    // }
    //
    // template <class T, class P>
    // static T join(Task<T, P> task, std::size_t index = 0) {
    //     return instance->mWorkers[index].join(std::move(task));
    // }

    static IOContextMT *instance;
};
} // namespace co_async





namespace co_async {

struct SpinBarrier {
    explicit SpinBarrier(std::size_t n) noexcept
        : m_top_waiting(static_cast<std::uint32_t>(n) - 1),
          m_num_waiting(0),
          m_sync_flip(0) {}

    bool arrive_and_wait() noexcept {
        bool old_flip = m_sync_flip.load(std::memory_order_relaxed);
        if (m_num_waiting.fetch_add(1, std::memory_order_relaxed) ==
            m_top_waiting) {
            m_num_waiting.store(0, std::memory_order_relaxed);
            m_sync_flip.store(!old_flip, std::memory_order_release);
            return true;
        } else {
            while (m_sync_flip.load(std::memory_order_acquire) == old_flip)
                ;
#if __cpp_lib_atomic_wait
            m_sync_flip.wait(old_flip, std::memory_order_acquire);
#endif
            return false;
        }
    }

    bool arrive_and_drop() noexcept {
        bool old_flip = m_sync_flip.load(std::memory_order_relaxed);
        if (m_num_waiting.fetch_add(1, std::memory_order_relaxed) ==
            m_top_waiting) {
            m_num_waiting.store(0, std::memory_order_relaxed);
            m_sync_flip.store(!old_flip, std::memory_order_release);
            return true;
        } else {
            return false;
        }
    }

private:
    std::uint32_t const m_top_waiting;
    FutexAtomic<std::uint32_t> m_num_waiting;
    FutexAtomic<bool> m_sync_flip;
};

} // namespace co_async



namespace co_async {

template <class ...Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

template <class ...Fs>
overloaded(Fs...) -> overloaded<Fs...>;

}




namespace co_async {
template <class K, class V>
struct SimpleMap {
    SimpleMap() = default;

    SimpleMap(std::initializer_list<std::pair<K, V>> init)
        : mData(init.begin(), init.end()) {}

    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    V *at(Key const &key) noexcept {
        auto it = mData.find(key);
        if (it == mData.end()) {
            return nullptr;
        }
        return std::addressof(it->second);
    }

    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    V const *at(Key const &key) const noexcept {
        auto it = mData.find(key);
        if (it == mData.end()) {
            return nullptr;
        }
        return std::addressof(it->second);
    }

    template <class Key, class F = std::identity>
        requires(requires(F f, V const &v, K const &k, Key const &key) {
            std::invoke(f, v);
            k < key;
            key < k;
        })
    decltype(std::optional(std::declval<std::invoke_result_t<F, V const &>>()))
    get(Key const &key, F &&func = {}) const noexcept {
        auto it = mData.find(key);
        if (it == mData.end()) {
            return std::nullopt;
        }
        return std::invoke(func, it->second);
    }

    template <std::convertible_to<K> Key>
    V &insert_or_assign(Key &&key, V value) {
        return mData
            .insert_or_assign(K(std::forward<Key>(key)), std::move(value))
            .first->second;
    }

    template <std::convertible_to<K> Key>
    V &insert(Key &&key, V value) {
        return mData.emplace(K(std::forward<Key>(key)), std::move(value))
            .first->second;
    }

    template <std::convertible_to<K> Key, class... Args>
        requires std::constructible_from<V, Args...>
    V &emplace(Key &&key, Args &&...args) {
        return mData
            .emplace(K(std::forward<Key>(key)), std::forward<Args>(args)...)
            .first->second;
    }

    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    bool contains(Key &&key) const noexcept {
        return mData.find(std::forward<Key>(key)) != mData.end();
    }

    template <class Key>
        requires(requires(K k, Key key) {
            k < key;
            key < k;
        })
    bool erase(Key &&key) {
        auto it = mData.find(std::forward<Key>(key));
        if (it == mData.end()) {
            return false;
        }
        mData.erase(it);
        return true;
    }

    auto begin() const noexcept {
        return mData.begin();
    }

    auto end() const noexcept {
        return mData.end();
    }

    auto begin() noexcept {
        return mData.begin();
    }

    auto end() noexcept {
        return mData.end();
    }

    bool empty() const noexcept {
        return mData.empty();
    }

    std::size_t size() const noexcept {
        return mData.size();
    }

private:
    std::map<K, V, std::less<>> mData;
};
} // namespace co_async


//
// debug.hpp - prints everything!
//
// Source code available at: https://github.com/archibate/debug-hpp
//
// Usage:
//   debug(), "my variable is", your_variable;
//
// Results in:
//   your_file.cpp:233:  my variable is {1, 2, 3}
//
// (suppose your_variable is an std::vector)
//
// **WARNING**:
//   debug() only works in `Debug` build! It is automatically disabled in
//   `Release` build (we do this by checking whether the NDEBUG macro is
//   defined). Yeah, completely no outputs in `Release` build, this is by
//   design.
//
//   This is a feature for convenience, e.g. you don't have to remove all the
//   debug() sentences after debug done, simply switch to `Release` build and
//   everything debug is gone, no runtime overhead! And when you need debug
//   simply switch back to `Debug` build and everything debug() you written
//   before is back in life.
//
//   If you insist to use debug() even in `Release` build, please `#define
//   DEBUG_LEVEL 1` before including this header file.
//
//
// Assertion check is also supported:
//   debug().check(some_variable) > 0;
//
// Will trigger a 'trap' interrupt (__debugbreak for MSVC and __builtin_trap for
// GCC, configurable, see below) for the debugger to catch when `some_variable >
// 0` is false, as well as printing human readable error message:
//   your_file.cpp:233:  assertion failed: 3 < 0
//
//
// After debugging complete, no need to busy removing all debug() calls! Simply:
//   #define NDEBUG
// would supress all debug() prints and assertion checks, completely no runtime
// overhead. For CMake or Visual Studio users, simply switch to `Release` build
// would supress debug() prints. Since they automatically define `NDEBUG` for
// you in `Release`, `RelWithDebInfo` and `MinSizeRel` build types.
//
//
// TL;DR: This is a useful debugging utility the C++ programmers had all dreamed
// of:
//
//   1. print using the neat comma syntax, easy-to-use
//   2. supports printing STL objects including string, vector, tuple, optional,
//   variant, unique_ptr, type_info, error_code, and so on.
//   3. just add a member method named `repr`, e.g. `std::string repr() const {
//   ... }` to support printing your custom class!
//   4. classes that are not supported to print will be shown in something like
//   `[TypeName@0xdeadbeaf]` where 0xdeadbeaf is it's address.
//   5. highly configurable, customize the behaviour by defining the DEBUG_xxx
//   macros (see below)
//   6. when debug done, supress all debug messages by simply `#define NDEBUG`,
//   the whole library is disabled at compile-time, no runtime overhead
//   7. Thread safe, every line of message is always distinct, no annoying
//   interleaving output rushing into console (typical experience when using
//   cout)
//
//
// Here is a list of configurable macros, define them **before** including this
// header file to take effect:
//
// `#define DEBUG_LEVEL 0` (default when defined NDEBUG) - disable debug output,
// completely no runtime overhead
// `#define DEBUG_LEVEL 1` (default when !defined NDEBUG) - enable debug output,
// prints everything you asked to print
//
// `#define DEBUG_STEPPING 0` (default) - no step debugging
// `#define DEBUG_STEPPING 1` - enable step debugging, stops whenever debug
// output generated, manually press ENTER to continue
// `#define DEBUG_STEPPING 2` - enable step debugging, like 1, but trigger a
// 'trap' interrupt for debugger to catch instead
//
// `#define DEBUG_SHOW_TIMESTAMP 0` - do not print timestamp
// `#define DEBUG_SHOW_TIMESTAMP 1` - enable printing a timestamp for each line
// of debug output (e.g. "09:57:32")
// `#define DEBUG_SHOW_TIMESTAMP 2` (default) - printing timestamp relative to
// program staring time rather than system real time
//
// `#define DEBUG_SHOW_THREAD_ID 0` (default) - do not print the thread id
// `#define DEBUG_SHOW_THREAD_ID 1` - print the current thread id
//
// `#define DEBUG_SHOW_LOCATION 1` (default) - show source location mark before
// each line of the debug output (e.g. "file.cpp:233")
// `#define DEBUG_SHOW_LOCATION 0` - do not show the location mark
//
// `#define DEBUG_OUTPUT std::cerr <<` (default) - controls where to output the
// debug strings (must be callable as DEBUG_OUTPUT(str))
//
// `#define DEBUG_PANIC_METHOD 0` - throws an runtime error with debug string as
// message when assertion failed
// `#define DEBUG_PANIC_METHOD 1` (default) - print the error message when
// assertion failed, then triggers a 'trap' interrupt, useful for debuggers to
// catch, if no debuggers attached, the program would terminate
// `#define DEBUG_PANIC_METHOD 2` - print the error message when assertion
// failed, and then call std::terminate
// `#define DEBUG_PANIC_METHOD 3` - print the error message when assertion
// failed, do not terminate, do not throw any exception
//
// `#define DEBUG_REPR_NAME repr` (default) - if an object x have a member
// function like `x.repr()` or a global function `repr` supporting `repr(x)`,
// then the value of `x.repr()` or `repr(x)` will be printed instead for this
// object
//
// `#define DEBUG_RANGE_BRACE "{}"` (default) - controls format for range-like
// objects (supporting begin(x) and end(x)) in "{1, 2, 3, ...}"
// `#define DEBUG_RANGE_COMMA ", "` (default) - ditto
//
// `#define DEBUG_TUPLE_BRACE "{}"` (default) - controls format for tuple-like
// objects (supporting std::tuple_size<X>) in "{1, 2, 3}"
// `#define DEBUG_TUPLE_COMMA ", "` (default) - ditto
//
// `#define DEBUG_NAMED_MEMBER_MARK ": "` (default) - used in
// debug::named_member and DEBUG_REPR, e.g. '{name: "name", age: 42}'
//
// `#define DEBUG_MAGIC_ENUM magic_enum::enum_name` - enable printing enum in
// their name rather than value, if you have magic_enum.hpp
//
// `#define DEBUG_UNSIGNED_AS_HEXADECIMAL 0` (default) - print unsigned integers
// as decimal
// `#define DEBUG_UNSIGNED_AS_HEXADECIMAL 1` - print unsigned integers as
// hexadecimal
// `#define DEBUG_UNSIGNED_AS_HEXADECIMAL 2` - print unsigned integers as
// full-width hexadecimal
//
// `#define DEBUG_HEXADECIMAL_UPPERCASE 0` (default) - print hexadecimal values
// in lowercase (a-f)
// `#define DEBUG_HEXADECIMAL_UPPERCASE 1` - print hexadecimal values in
// uppercase (A-F)
//
// `#define DEBUG_SUPRESS_NON_ASCII 0` (default) - consider non-ascii characters
// in std::string as printable (e.g. UTF-8 encoded Chinese characters)
// `#define DEBUG_SUPRESS_NON_ASCII 1` - consider non-ascii characters in
// std::string as not printable (print them in e.g. '\xfe' instead)
//
// `#define DEBUG_SHOW_SOURCE_CODE_LINE 1` - enable debug output with detailed
// source code level information (requires readable source file path)
//
// `#define DEBUG_NULLOPT_STRING "nullopt"` (default) - controls how to print
// optional-like objects (supporting *x and (bool)x) when it is nullopt
//
// `#define DEBUG_NULLPTR_STRING "nullptr"` (default) - controls how to print
// pointer-like objects (supporting static_cast<void const volatile *>(x.get()))
// when it is nullptr
//
// `#define DEBUG_SMART_POINTER_MODE 1` (default) - print smart pointer as raw
// pointer address (e.g. 0xdeadbeaf)
// `#define DEBUG_SMART_POINTER_MODE 2` - print smart pointer as content value
// unless nullptr (e.g. 42 for std::shared_ptr<int>)
// `#define DEBUG_SMART_POINTER_MODE 3` - print smart pointer as both content
// value and raw pointer address (e.g. 42@0xdeadbeaf)
//
// `#define DEBUG_NAMESPACE_BEGIN` (default) - expose debug in the global
// namespace
// `#define DEBUG_NAMESPACE_END` (default) - ditto
//
// `#define DEBUG_NAMESPACE_BEGIN namespace mydebugger {` - expose debug in the
// the namespace `mydebugger`, and use it like `mydebugger::debug()`
// `#define DEBUG_NAMESPACE_END }` - ditto
//
// `#define DEBUG_CLASS_NAME debug` (default) - the default name for the debug
// class is `debug()`, you may define your custom name here
//
#ifndef DEBUG_LEVEL
# ifdef NDEBUG
#  define DEBUG_LEVEL 0
# else
#  define DEBUG_LEVEL 1
# endif
#endif
#ifndef DEBUG_SHOW_LOCATION
# define DEBUG_SHOW_LOCATION 1
#endif
#ifndef DEBUG_REPR_NAME
# define DEBUG_REPR_NAME repr
#endif
#ifndef DEBUG_FORMATTER_REPR_NAME
# define DEBUG_FORMATTER_REPR_NAME _debug_formatter_repr
#endif
#ifndef DEBUG_NAMESPACE_BEGIN
# define DEBUG_NAMESPACE_BEGIN
#endif
#ifndef DEBUG_NAMESPACE_END
# define DEBUG_NAMESPACE_END
#endif
#ifndef DEBUG_OUTPUT
# define DEBUG_OUTPUT std::cerr <<
#endif
#ifndef DEBUG_ENABLE_FILES_MATCH
# define DEBUG_ENABLE_FILES_MATCH 0
#endif
#ifndef DEBUG_ERROR_CODE_SHOW_NUMBER
# define DEBUG_ERROR_CODE_SHOW_NUMBER 0
#endif
#ifndef DEBUG_PANIC_METHOD
# define DEBUG_PANIC_METHOD 1
#endif
#ifndef DEBUG_STEPPING
# define DEBUG_STEPPING 0
#endif
#ifndef DEBUG_SUPRESS_NON_ASCII
# define DEBUG_SUPRESS_NON_ASCII 0
#endif
#ifndef DEBUG_SEPARATOR_FILE
# define DEBUG_SEPARATOR_FILE ':'
#endif
#ifndef DEBUG_SEPARATOR_LINE
# define DEBUG_SEPARATOR_LINE ':'
#endif
#ifndef DEBUG_SEPARATOR_TAB
# define DEBUG_SEPARATOR_TAB '\t'
#endif
#ifndef DEBUG_SOURCE_LINE_BRACE
# define DEBUG_SOURCE_LINE_BRACE "[]"
#endif
#ifndef DEBUG_TIMESTAMP_BRACE
# define DEBUG_TIMESTAMP_BRACE "[]"
#endif
#ifndef DEBUG_RANGE_BRACE
# define DEBUG_RANGE_BRACE "{}"
#endif
#ifndef DEBUG_RANGE_COMMA
# define DEBUG_RANGE_COMMA ", "
#endif
#ifndef DEBUG_TUPLE_BRACE
# define DEBUG_TUPLE_BRACE "{}"
#endif
#ifndef DEBUG_TUPLE_COMMA
# define DEBUG_TUPLE_COMMA ", "
#endif
#ifndef DEBUG_ENUM_BRACE
# define DEBUG_ENUM_BRACE "()"
#endif
#ifndef DEBUG_NAMED_MEMBER_MARK
# define DEBUG_NAMED_MEMBER_MARK ": "
#endif
#ifndef DEBUG_NULLOPT_STRING
# define DEBUG_NULLOPT_STRING "nullopt"
#endif
#ifndef DEBUG_NULLPTR_STRING
# define DEBUG_NULLPTR_STRING "nullptr"
#endif
#ifndef DEBUG_UNKNOWN_TYPE_BRACE
# define DEBUG_UNKNOWN_TYPE_BRACE "[]"
#endif
#ifndef DEBUG_ERROR_CODE_BRACE
# define DEBUG_ERROR_CODE_BRACE "[]"
#endif
#ifndef DEBUG_ERROR_CODE_INFIX
# define DEBUG_ERROR_CODE_INFIX ""
#endif
#ifndef DEBUG_ERROR_CODE_POSTFIX
# define DEBUG_ERROR_CODE_POSTFIX ": "
#endif
#ifndef DEBUG_ERROR_CODE_NO_ERROR
# define DEBUG_ERROR_CODE_NO_ERROR std::generic_category().message(0)
#endif
#ifndef DEBUG_UNKNOWN_TYPE_AT
# define DEBUG_UNKNOWN_TYPE_AT '@'
#endif
#ifndef DEBUG_SMART_POINTER_MODE
# define DEBUG_SMART_POINTER_MODE 1
#endif
#ifndef DEBUG_SMART_POINTER_AT
# define DEBUG_SMART_POINTER_AT '@'
#endif
#ifndef DEBUG_POINTER_HEXADECIMAL_PREFIX
# define DEBUG_POINTER_HEXADECIMAL_PREFIX "0x"
#endif
#ifndef DEBUG_UNSIGNED_AS_HEXADECIMAL
# define DEBUG_UNSIGNED_AS_HEXADECIMAL 0
#endif
#ifndef DEBUG_HEXADECIMAL_UPPERCASE
# define DEBUG_HEXADECIMAL_UPPERCASE 0
#endif
#ifndef DEBUG_SHOW_SOURCE_CODE_LINE
# define DEBUG_SHOW_SOURCE_CODE_LINE 0
#endif
#ifndef DEBUG_SHOW_TIMESTAMP
# define DEBUG_SHOW_TIMESTAMP 2
#endif
#ifdef DEBUG_CLASS_NAME
# define debug DEBUG_CLASS_NAME
#endif
#if DEBUG_LEVEL
# include <cstdint>
# include <cstdlib>
# include <iomanip>
# include <iostream>
# include <limits>
# include <system_error>
# if DEBUG_SHOW_SOURCE_CODE_LINE
#  include <fstream>
#  include <unordered_map>
# endif
# ifndef DEBUG_SOURCE_LOCATION
#  if __cplusplus >= 202002L
#   if defined(__has_include)
#    if __has_include(<source_location>)
#     include <source_location>
#     if __cpp_lib_source_location
#      define DEBUG_SOURCE_LOCATION std::source_location
#     endif
#    endif
#   endif
#  endif
# endif
# ifndef DEBUG_SOURCE_LOCATION
#  if __cplusplus >= 201505L
#   if defined(__has_include)
#    if __has_include(<experimental/source_location>)
#     include <experimental/source_location>
#     if __cpp_lib_experimental_source_location
#      define DEBUG_SOURCE_LOCATION std::experimental::source_location
#     endif
#    endif
#   endif
#  endif
# endif
# include <memory>
# include <sstream>
# include <string>
# include <type_traits>
# include <typeinfo>
# include <utility>
# ifndef DEBUG_CUSTOM_DEMANGLE
#  ifndef DEBUG_HAS_CXXABI_H
#   if defined(__has_include)
#    if defined(__unix__) && __has_include(<cxxabi.h>)
#     include <cxxabi.h>
#     define DEBUG_HAS_CXXABI_H
#    endif
#   endif
#  else
#   include <cxxabi.h>
#  endif
# endif
# if DEBUG_SHOW_TIMESTAMP == 1
#  if defined(__has_include)
#   if defined(__unix__) && __has_include(<sys/time.h>)
#    include <sys/time.h>
#    define DEBUG_HAS_SYS_TIME_H
#   endif
#  endif
#  ifndef DEBUG_HAS_SYS_TIME_H
#   include <chrono>
#  endif
# elif DEBUG_SHOW_TIMESTAMP == 2
#  include <chrono>
# endif
# if DEBUG_SHOW_THREAD_ID
#  include <thread>
# endif
# if defined(__has_include)
#  if __has_include(<variant>)
#   include <variant>
#  endif
# endif
# ifndef DEBUG_STRING_VIEW
#  if defined(__has_include)
#   if __cplusplus >= 201703L
#    if __has_include(<string_view>)
#     include <string_view>
#     if __cpp_lib_string_view
#      define DEBUG_STRING_VIEW std::string_view
#     endif
#    endif
#   endif
#  endif
# endif
# ifndef DEBUG_STRING_VIEW
#  include <string>
#  define DEBUG_STRING_VIEW std::string
# endif
# if __cplusplus >= 202002L
#  if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(unlikely)
#    define DEBUG_UNLIKELY [[unlikely]]
#   else
#    define DEBUG_UNLIKELY
#   endif
#   if __has_cpp_attribute(likely)
#    define DEBUG_LIKELY [[likely]]
#   else
#    define DEBUG_LIKELY
#   endif
#   if __has_cpp_attribute(nodiscard)
#    define DEBUG_NODISCARD [[nodiscard]]
#   else
#    define DEBUG_NODISCARD
#   endif
#  else
#   define DEBUG_LIKELY
#   define DEBUG_UNLIKELY
#   define DEBUG_NODISCARD
#  endif
# else
#  define DEBUG_LIKELY
#  define DEBUG_UNLIKELY
#  define DEBUG_NODISCARD
# endif
# if DEBUG_STEPPING == 1
#  include <mutex>
#  if defined(__has_include)
#   if defined(__unix__)
#    if __has_include(<unistd.h>) && __has_include(<termios.h>)
#     include <termios.h>
#     include <unistd.h>
#     define DEBUG_STEPPING_HAS_TERMIOS 1
#    endif
#   endif
#   if defined(_WIN32)
#    if __has_include(<conio.h>)
#     include <conio.h>
#     define DEBUG_STEPPING_HAS_GETCH 1
#    endif
#   endif
#  endif
# endif
DEBUG_NAMESPACE_BEGIN

struct DEBUG_NODISCARD debug {
private:
# ifndef DEBUG_MAGIC_ENUM
#  if (__GNUC__ || __clang__ || _MSC_VER) && __cpp_fold_expressions
#   if __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#   endif

    template <class E, E CrItMaGiC>
    static DEBUG_STRING_VIEW debug_enum_value_name() {
        DEBUG_STRING_VIEW name = __PRETTY_FUNCTION__;
        auto critpos = name.find("CrItMaGiC = ") + 12;
        auto endpos = name.find_first_of(";]");
        auto slice = name.substr(critpos, endpos - critpos);
        return slice;
    }

    template <class E, class I, I... Is>
    static DEBUG_STRING_VIEW
    debug_enum_name_impl(E value, std::integer_sequence<I, Is...>) {
        DEBUG_STRING_VIEW ret = "???";
        ((value == Is
              ? (ret = debug_enum_value_name<E, static_cast<E>(Is)>(), true)
              : false) ||
         ...);
        return ret;
    }

    template <class E>
    static DEBUG_STRING_VIEW debug_enum_name(E value) {
        using I = typename std::underlying_type<E>::type;
        return debug_enum_name_impl(value,
                                    std::make_integer_sequence<I, 128>{});
    }
#   if __clang__
#    pragma clang diagnostic pop
#   endif
#   define DEBUG_MAGIC_ENUM debug_enum_name
#  endif
# endif
# ifndef DEBUG_SOURCE_LOCATION
    struct debug_source_location {
        char const *fn;
        int ln;
        int col;
        char const *fun;

        char const *file_name() const noexcept {
            return fn;
        }

        int line() const noexcept {
            return ln;
        }

        int column() const noexcept {
            return col;
        }

        char const *function_name() const noexcept {
            return fun;
        }

        static debug_source_location current() noexcept {
            return {"???", 0, 0, "?"};
        }
    };
#  ifndef DEBUG_SOURCE_LOCATION_FAKER
#   define DEBUG_SOURCE_LOCATION_FAKER \
       { __FILE__, __LINE__, 0, __func__ }
#  endif
#  define DEBUG_SOURCE_LOCATION debug::debug_source_location
# endif
    template <class T>
    static auto debug_deref_avoid(T const &t) ->
        typename std::enable_if<!std::is_void<decltype(*t)>::value,
                                decltype(*t)>::type {
        return *t;
    }

    struct debug_special_void {
        char const (&repr() const)[5] {
            return "void";
        }
    };

    template <class T>
    static auto debug_deref_avoid(T const &t) ->
        typename std::enable_if<std::is_void<decltype(*t)>::value,
                                debug_special_void>::type {
        return debug_special_void();
    }

    static void debug_quotes(std::ostream &oss, DEBUG_STRING_VIEW sv,
                             char quote) {
        oss << quote;
        for (char c: sv) {
            switch (c) {
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\\': oss << "\\\\"; break;
            case '\0': oss << "\\0"; break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F
# if DEBUG_SUPRESS_NON_ASCII
                    || (static_cast<unsigned char>(c) >= 0x80)
# endif
                ) {
                    auto f = oss.flags();
                    oss << "\\x" << std::hex << std::setfill('0')
                        << std::setw(2)
                        << static_cast<int>(static_cast<unsigned char>(c))
# if DEBUG_HEXADECIMAL_UPPERCASE
                        << std::uppercase
# endif
                        ;
                    oss.flags(f);
                } else {
                    if (c == quote) {
                        oss << '\\';
                    }
                    oss << c;
                }
                break;
            }
        }
        oss << quote;
    }

    template <class T>
    struct debug_is_char_array : std::false_type {};

    template <std::size_t N>
    struct debug_is_char_array<char[N]> : std::true_type {};
# ifdef DEBUG_CUSTOM_DEMANGLE
    static std::string debug_demangle(char const *name) {
        return DEBUG_CUSTOM_DEMANGLE(name);
    }
# else
    static std::string debug_demangle(char const *name) {
#  ifdef DEBUG_HAS_CXXABI_H
        int status;
        char *p = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string s = p ? p : name;
        std::free(p);
#  else
        std::string s = name;
#  endif
        return s;
    }
# endif
public:
    struct debug_formatter {
        std::ostream &os;

        template <class T>
        debug_formatter &operator<<(T const &value) {
            debug_format(os, value);
            return *this;
        }
    };

private:
# if __cpp_if_constexpr && __cpp_concepts && \
     __cpp_lib_type_trait_variable_templates
public:
    template <class T, class U>
        requires std::is_same<T, U>::value
    static void debug_same_as(U &&) {}

private:
    template <class T>
    static void debug_format(std::ostream &oss, T const &t) {
        using std::begin;
        using std::end;
        if constexpr (debug_is_char_array<T>::value) {
            oss << t;
        } else if constexpr ((std::is_convertible<T,
                                                  DEBUG_STRING_VIEW>::value ||
                              std::is_convertible<T, std::string>::value)) {
            if constexpr (!std::is_convertible<T, DEBUG_STRING_VIEW>::value) {
                std::string s = t;
                debug_quotes(oss, s, '"');
            } else {
                debug_quotes(oss, t, '"');
            }
        } else if constexpr (std::is_same<T, bool>::value) {
            auto f = oss.flags();
            oss << std::boolalpha << t;
            oss.flags(f);
        } else if constexpr (std::is_same<T, char>::value ||
                             std::is_same<T, signed char>::value) {
            debug_quotes(oss, {reinterpret_cast<char const *>(&t), 1}, '\'');
        } else if constexpr (
#  if __cpp_char8_t
            std::is_same<T, char8_t>::value ||
#  endif
            std::is_same<T, char16_t>::value ||
            std::is_same<T, char32_t>::value ||
            std::is_same<T, wchar_t>::value) {
            auto f = oss.flags();
            oss << "'\\"
                << " xu U"[sizeof(T)] << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2)
#  if DEBUG_HEXADECIMAL_UPPERCASE
                << std::uppercase
#  endif
                << static_cast<std::uint32_t>(t) << "'";
            oss.flags(f);
#  if DEBUG_UNSIGNED_AS_HEXADECIMAL
        } else if constexpr (std::is_integral<T>::value) {
            if constexpr (std::is_unsigned<T>::value) {
                auto f = oss.flags();
                oss << "0x" << std::hex << std::setfill('0')
#   if DEBUG_UNSIGNED_AS_HEXADECIMAL >= 2
                    << std::setw(sizeof(T) * 2)
#   endif
#   if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#   endif
                    ;
                if constexpr (sizeof(T) == 1) {
                    oss << static_cast<unsigned int>(t);
                } else {
                    oss << t;
                }
                oss.flags(f);
            } else {
                oss << static_cast<std::uint64_t>(
                    static_cast<typename std::make_unsigned<T>::type>(t));
            }
#  else
        } else if constexpr (std::is_integral<T>::value) {
            oss << t; // static_cast<std::uint64_t>(static_cast<typename
                      // std::make_unsigned<T>::type>(t));
#  endif
        } else if constexpr (std::is_floating_point<T>::value) {
            auto f = oss.flags();
            oss << std::fixed
                << std::setprecision(std::numeric_limits<T>::digits10) << t;
            oss.flags(f);
        } else if constexpr (requires(T const &t) {
                                 static_cast<void const volatile *>(t.get());
                             }) {
            auto const *p = t.get();
            if (p != nullptr) {
#  if DEBUG_SMART_POINTER_MODE == 1
                debug_format(oss, *p);
#  elif DEBUG_SMART_POINTER_MODE == 2
                auto f = oss.flags();
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#   if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#   endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    static_cast<void const volatile *>(p));
                oss.flags(f);
#  else
                debug_format(oss, *p);
                oss << DEBUG_SMART_POINTER_AT;
                auto f = oss.flags();
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#   if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#   endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    static_cast<void const volatile *>(p));
                oss.flags(f);
#  endif
            } else {
                oss << DEBUG_NULLPTR_STRING;
            }
        } else if constexpr (std::is_same<T, std::errc>::value) {
            oss << DEBUG_ERROR_CODE_BRACE[0];
            if (t != std::errc()) {
                oss << std::generic_category().name() << DEBUG_ERROR_CODE_INFIX
#  if DEBUG_ERROR_CODE_SHOW_NUMBER
                    << ' ' << static_cast<int>(t)
#  endif
                    << DEBUG_ERROR_CODE_POSTFIX;
                oss << std::generic_category().message(static_cast<int>(t));
            } else {
                oss << DEBUG_ERROR_CODE_NO_ERROR;
            }
            oss << DEBUG_ERROR_CODE_BRACE[1];
        } else if constexpr (std::is_same<T, std::error_code>::value ||
                             std::is_same<T, std::error_condition>::value) {
            oss << DEBUG_ERROR_CODE_BRACE[0];
            if (t) {
                oss << t.category().name() << DEBUG_ERROR_CODE_INFIX
#  if DEBUG_ERROR_CODE_SHOW_NUMBER
                    << ' ' << t.value()
#  endif
                    << DEBUG_ERROR_CODE_POSTFIX << t.message();
            } else {
                oss << DEBUG_ERROR_CODE_NO_ERROR;
            }
            oss << DEBUG_ERROR_CODE_BRACE[1];
        } else if constexpr (requires(T const &t) {
                                 debug_same_as<typename T::type &>(t.get());
                             }) {
            debug_format(oss, t.get());
        } else if constexpr (requires(std::ostream &oss, T const &t) {
                                 oss << t;
                             } && !std::is_enum<T>::value) {
            oss << t;
        } else if constexpr (std::is_pointer<T>::value ||
                             std::is_same<T, std::nullptr_t>::value) {
            if (t == nullptr) {
                auto f = oss.flags();
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#  if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#  endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    reinterpret_cast<void const volatile *>(t));
                oss.flags(f);
            } else {
                oss << DEBUG_NULLPTR_STRING;
            }
        } else if constexpr (requires(T const &t) { begin(t) != end(t); }) {
            oss << DEBUG_RANGE_BRACE[0];
            bool add_comma = false;
            for (auto &&i: t) {
                if (add_comma) {
                    oss << DEBUG_RANGE_COMMA;
                }
                add_comma = true;
                debug_format(oss, std::forward<decltype(i)>(i));
            }
            oss << DEBUG_RANGE_BRACE[1];
        } else if constexpr (requires { std::tuple_size<T>::value; }) {
            oss << DEBUG_TUPLE_BRACE[0];
            bool add_comma = false;
            std::apply(
                [&](auto &&...args) {
                    (([&] {
                         if (add_comma) {
                             oss << DEBUG_TUPLE_COMMA;
                         }
                         add_comma = true;
                         debug_format(oss, std::forward<decltype(args)>(args));
                     }()),
                     ...);
                },
                t);
            oss << DEBUG_TUPLE_BRACE[1];
        } else if constexpr (std::is_enum<T>::value) {
#  ifdef DEBUG_MAGIC_ENUM
            oss << DEBUG_MAGIC_ENUM(t);
#  else
            oss << debug_demangle(typeid(T).name()) << DEBUG_ENUM_BRACE[0];
            oss << static_cast<typename std::underlying_type<T>::type>(t);
            oss << DEBUG_ENUM_BRACE[1];
#  endif
        } else if constexpr (std::is_same<T, std::type_info>::value) {
            oss << debug_demangle(t.name());
        } else if constexpr (requires(T const &t) { t.DEBUG_REPR_NAME(); }) {
            debug_format(oss, raw_repr_if_string(t.DEBUG_REPR_NAME()));
        } else if constexpr (requires(T const &t) { t.DEBUG_REPR_NAME(oss); }) {
            t.DEBUG_REPR_NAME(oss);
        } else if constexpr (requires(T const &t) { DEBUG_REPR_NAME(t); }) {
            debug_format(oss, raw_repr_if_string(DEBUG_REPR_NAME(t)));
        } else if constexpr (requires(debug_formatter const &out, T const &t) {
                                 t.DEBUG_FORMATTER_REPR_NAME(out);
                             }) {
            t.DEBUG_FORMATTER_REPR_NAME(debug_formatter{oss});
        } else if constexpr (requires(debug_formatter const &out, T const &t) {
                                 DEBUG_FORMATTER_REPR_NAME(out, t);
                             }) {
            DEBUG_FORMATTER_REPR_NAME(debug_formatter{oss}, t);
#  if __cpp_lib_variant
        } else if constexpr (requires { std::variant_size<T>::value; }) {
            visit([&oss](auto const &t) { debug_format(oss, t); }, t);
#  endif
        } else if constexpr (requires(T const &t) {
                                 (void)(*t);
                                 (void)static_cast<bool>(t);
                             }) {
            if (static_cast<bool>(t)) {
                debug_format(oss, debug_deref_avoid(t));
            } else {
                oss << DEBUG_NULLOPT_STRING;
            }
        } else {
            oss << DEBUG_UNKNOWN_TYPE_BRACE[0]
                << debug_demangle(typeid(t).name()) << DEBUG_UNKNOWN_TYPE_AT;
            debug_format(oss,
                         reinterpret_cast<void const *>(std::addressof(t)));
            oss << DEBUG_UNKNOWN_TYPE_BRACE[1];
        }
    }
# else
    template <class T>
    struct debug_void {
        using type = void;
    };

    template <bool v>
    struct debug_bool_constant {
        enum {
            value = v
        };
    };

#  define DEBUG_COND(n, ...) \
      template <class T, class = void> \
      struct debug_cond_##n : std::false_type {}; \
      template <class T> \
      struct debug_cond_##n<T, \
                            typename debug_void<decltype(__VA_ARGS__)>::type> \
          : std::true_type {};
    DEBUG_COND(is_ostream_ok2, std::declval<std::ostream &>()
                                   << std::declval<T const &>());

    struct debug_cond_is_ostream_ok
        : std::bool_constant<debug_cond_is_ostream_ok2<T>::value &&
                             !std::is_enum<T>::value> {};

    DEBUG_COND(is_range, begin(std::declval<T const &>()) !=
                             end(std::declval<T const &>()));
    DEBUG_COND(is_tuple, std::tuple_size<T>::value);
    DEBUG_COND(is_member_repr, std::declval<T const &>().DEBUG_REPR_NAME());
    DEBUG_COND(is_member_repr_stream, std::declval<T const &>().DEBUG_REPR_NAME(
                                          std::declval<std::ostream &>()));
    DEBUG_COND(is_adl_repr, DEBUG_REPR_NAME(std::declval<T const &>()));
    DEBUG_COND(is_adl_repr_stream,
               DEBUG_REPR_NAME(std::declval<std::ostream &>(),
                               std::declval<T const &>()));
    DEBUG_COND(is_member_repr_debug,
               std::declval<T const &>().DEBUG_FORMATTER_REPR_NAME(
                   std::declval<debug_formatter const &>()));
    DEBUG_COND(is_adl_repr_debug, DEBUG_FORMATTER_REPR_NAME(
                                      std::declval<debug_formatter const &>(),
                                      std::declval<T const &>()));

    struct variant_test_lambda {
        std::ostream &oss;

        template <class T>
        void operator()(T const &) const {}
    };

#  if __cpp_lib_variant
    DEBUG_COND(is_variant, std::variant_size<T>::value);
#  else
    template <class>
    struct debug_cond_is_variant : std::false_type {};
#  endif
    DEBUG_COND(is_smart_pointer, static_cast<void const volatile *>(
                                     std::declval<T const &>().get()));
    DEBUG_COND(is_optional,
               (((void)*std::declval<T const &>(), (void)0),
                ((void)static_cast<bool>(std::declval<T const &>()), (void)0)));
    DEBUG_COND(
        reference_wrapper,
        (typename std::enable_if<
            std::is_same<typename T::type &,
                         decltype(std::declval<T const &>().get())>::value,
            int>::type)0);
#  define DEBUG_CON(n, ...) \
      template <class T> \
      struct debug_cond_##n : debug_bool_constant<__VA_ARGS__> {};
    DEBUG_CON(string, std::is_convertible<T, DEBUG_STRING_VIEW>::value ||
                          std::is_convertible<T, std::string>::value);
    DEBUG_CON(bool, std::is_same<T, bool>::value);
    DEBUG_CON(char, std::is_same<T, char>::value ||
                        std::is_same<T, signed char>::value);
    DEBUG_CON(error_code, std::is_same<T, std::errc>::value ||
                              std::is_same<T, std::error_code>::value ||
                              std::is_same<T, std::error_condition>::value);
#  if __cpp_char8_t
    DEBUG_CON(unicode_char, std::is_same<T, char8_t>::value ||
                                std::is_same<T, char16_t>::value ||
                                std::is_same<T, char32_t>::value ||
                                std::is_same<T, wchar_t>::value);
#  else
    DEBUG_CON(unicode_char, std::is_same<T, char16_t>::value ||
                                std::is_same<T, char32_t>::value ||
                                std::is_same<T, wchar_t>::value);
#  endif
#  if DEBUG_UNSIGNED_AS_HEXADECIMAL
    DEBUG_CON(integral_unsigned,
              std::is_integral<T>::value &&std::is_unsigned<T>::value);
#  else
    DEBUG_CON(integral_unsigned, false);
#  endif
    DEBUG_CON(integral, std::is_integral<T>::value);
    DEBUG_CON(floating_point, std::is_floating_point<T>::value);
    DEBUG_CON(pointer, std::is_pointer<T>::value ||
                           std::is_same<T, std::nullptr_t>::value);
    DEBUG_CON(enum, std::is_enum<T>::value);
    template <class T, class = void>
    struct debug_format_trait;

    template <class T>
    static void debug_format(std::ostream &oss, T const &t) {
        debug_format_trait<T>()(oss, t);
    }

    template <class T, class>
    struct debug_format_trait {
        void operator()(std::ostream &oss, T const &t) const {
            oss << DEBUG_UNKNOWN_TYPE_BRACE[0]
                << debug_demangle(typeid(t).name()) << DEBUG_UNKNOWN_TYPE_AT;
            debug_format(oss,
                         reinterpret_cast<void const *>(std::addressof(t)));
            oss << DEBUG_UNKNOWN_TYPE_BRACE[1];
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<debug_is_char_array<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            oss << t;
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               debug_cond_string<T>::value &&
               !std::is_convertible<T, DEBUG_STRING_VIEW>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            std::string s = t;
            debug_quotes(oss, s, '"');
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && debug_cond_string<T>::value &&
               std::is_convertible<T, DEBUG_STRING_VIEW>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            debug_quotes(oss, t, '"');
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<!debug_is_char_array<T>::value &&
                                   !debug_cond_string<T>::value &&
                                   debug_cond_bool<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto f = oss.flags();
            oss << std::boolalpha << t;
            oss.flags(f);
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && debug_cond_char<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            debug_quotes(oss, {reinterpret_cast<char const *>(&t), 1}, '\'');
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               debug_cond_unicode_char<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto f = oss.flags();
            oss << "'\\"
                << " xu U"[sizeof(T)] << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2)
#  if DEBUG_HEXADECIMAL_UPPERCASE
                << std::uppercase
#  endif
                << static_cast<std::uint32_t>(t) << "'";
            oss.flags(f);
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               debug_cond_integral_unsigned<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto f = oss.flags();
            oss << "0x" << std::hex << std::setfill('0')
#  if DEBUG_UNSIGNED_AS_HEXADECIMAL >= 2
                << std::setw(sizeof(T) * 2)
#  endif
#  if DEBUG_HEXADECIMAL_UPPERCASE
                << std::uppercase
#  endif
                ;
            oss << static_cast<std::uint64_t>(
                static_cast<typename std::make_unsigned<T>::type>(t));
            oss.flags(f);
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               debug_cond_integral<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            oss << t; // static_cast<std::uint64_t>(static_cast<typename
                      // std::make_unsigned<T>::type>(t));
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               debug_cond_floating_point<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto f = oss.flags();
            oss << std::fixed
                << std::setprecision(std::numeric_limits<T>::digits10) << t;
            oss.flags(f);
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               !debug_cond_floating_point<T>::value &&
               debug_cond_is_smart_pointer<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto const *p = t.get();
            if (p != nullptr) {
#  if DEBUG_SMART_POINTER_MODE == 1
                debug_format(oss, *p);
#  elif DEBUG_SMART_POINTER_MODE == 2
                auto f = oss.flags();
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#   if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#   endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    static_cast<void const volatile *>(p));
                oss.flags(f);
#  else
                debug_format(oss, *p);
                oss << DEBUG_SMART_POINTER_AT;
                auto f = oss.flags();
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#   if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#   endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    static_cast<void const volatile *>(p));
                oss.flags(f);
#  endif
            } else {
                oss << DEBUG_NULLPTR_STRING;
            }
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               !debug_cond_floating_point<T>::value &&
               !debug_cond_is_smart_pointer<T>::value &&
               !debug_cond_error_code<T>::value &&
               debug_cond_reference_wrapper<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            debug_format(oss, t.get());
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               !debug_cond_floating_point<T>::value &&
               !debug_cond_is_smart_pointer<T>::value &&
               !debug_cond_error_code<T>::value &&
               !debug_cond_reference_wrapper<T>::value &&
               debug_cond_is_ostream_ok<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            oss << t;
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               !debug_cond_floating_point<T>::value &&
               !debug_cond_is_smart_pointer<T>::value &&
               !debug_cond_error_code<T>::value &&
               !debug_cond_reference_wrapper<T>::value &&
               !debug_cond_is_ostream_ok<T>::value &&
               debug_cond_pointer<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            auto f = oss.flags();
            if (t == nullptr) {
                oss << DEBUG_POINTER_HEXADECIMAL_PREFIX << std::hex
                    << std::setfill('0')
#  if DEBUG_HEXADECIMAL_UPPERCASE
                    << std::uppercase
#  endif
                    ;
                oss << reinterpret_cast<std::uintptr_t>(
                    reinterpret_cast<void const volatile *>(t));
                oss.flags(f);
            } else {
                oss << DEBUG_NULLPTR_STRING;
            }
        }
    };

    template <class T>
    struct debug_format_trait<
        T, typename std::enable_if<
               !debug_is_char_array<T>::value && !debug_cond_string<T>::value &&
               !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
               !debug_cond_unicode_char<T>::value &&
               !debug_cond_integral_unsigned<T>::value &&
               !debug_cond_integral<T>::value &&
               !debug_cond_floating_point<T>::value &&
               !debug_cond_is_smart_pointer<T>::value &&
               !debug_cond_error_code<T>::value &&
               !debug_cond_reference_wrapper<T>::value &&
               !debug_cond_is_ostream_ok<T>::value &&
               !debug_cond_pointer<T>::value &&
               debug_cond_is_range<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            oss << DEBUG_RANGE_BRACE[0];
            bool add_comma = false;
            auto b = begin(t);
            auto e = end(t);
            for (auto it = b; it != e; ++it) {
                if (add_comma) {
                    oss << DEBUG_RANGE_COMMA;
                }
                add_comma = true;
                debug_format(oss, std::forward<decltype(*it)>(*it));
            }
            oss << DEBUG_RANGE_BRACE[1];
        }
    };
#  if __cpp_lib_integer_sequence
    template <class F, class Tuple, std::size_t... I>
    static void debug_apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {
        std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
    }

    template <class F, class Tuple, std::size_t... I>
    static void debug_apply(F &&f, Tuple &&t) {
        debug_apply_impl(
            std::forward<F>(f), std::forward<Tuple>(t),
            std::make_index_sequence<
                std::tuple_size<typename std::decay<Tuple>::type>::value>{});
    }
#  else
    template <std::size_t... I>
    struct debug_index_sequence {};

    template <std::size_t N, std::size_t... I>
    struct debug_make_index_sequence
        : debug_make_index_sequence<N - 1, I..., N - 1> {};

    template <std::size_t... I>
    struct debug_make_index_sequence<0, I...> : debug_index_sequence<I...> {};

    template <class F, class Tuple, std::size_t... I>
    static void debug_apply_impl(F &&f, Tuple &&t, debug_index_sequence<I...>) {
        return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
    }

    template <class F, class Tuple, std::size_t... I>
    static void debug_apply(F &&f, Tuple &&t) {
        return debug_apply_impl(
            std::forward<F>(f), std::forward<Tuple>(t),
            debug_make_index_sequence<
                std::tuple_size<typename std::decay<Tuple>::type>::value>{});
    }
#  endif
    struct debug_apply_lambda {
        std::ostream &oss;
        bool &add_comma;

        template <class Arg>
        void call(Arg &&arg) const {
            if (add_comma) {
                oss << DEBUG_TUPLE_COMMA;
            }
            add_comma = true;
            debug_format(oss, std::forward<decltype(arg)>(arg));
        }

        template <class... Args>
        void operator()(Args &&...args) const {
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    debug_cond_is_tuple<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    oss << DEBUG_TUPLE_BRACE[0];
                    bool add_comma = false;
                    debug_apply(debug_apply_lambda{oss, add_comma}, t);
                    oss << DEBUG_TUPLE_BRACE[1];
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    debug_cond_enum<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
#  ifdef DEBUG_MAGIC_ENUM
                    oss << DEBUG_MAGIC_ENUM(t);
#  else
                    oss << debug_demangle(typeid(T).name())
                        << DEBUG_ENUM_BRACE[0];
                    oss << static_cast<typename std::underlying_type<T>::type>(
                        t);
                    oss << DEBUG_ENUM_BRACE[1];
#  endif
                }
            };

            template <class V>
            struct debug_format_trait<std::type_info, V> {
                void operator()(std::ostream &oss,
                                std::type_info const &t) const {
                    oss << debug_demangle(t.name());
                }
            };

            template <class V>
            struct debug_format_trait<std::errc, V> {
                void operator()(std::ostream &oss, std::errc const &t) const {
                    oss << DEBUG_ERROR_CODE_BRACE[0];
                    if (t != std::errc()) {
                        oss << std::generic_category().name()
                            << DEBUG_ERROR_CODE_INFIX
#  if DEBUG_ERROR_CODE_SHOW_NUMBER
                            << ' ' << static_cast<int>(t)
#  endif
                            << DEBUG_ERROR_CODE_POSTFIX;
                        oss << std::generic_category().message(
                            static_cast<int>(t));
                    } else {
                        oss << DEBUG_ERROR_CODE_NO_ERROR;
                    }
                    oss << DEBUG_ERROR_CODE_BRACE[1];
                }
            };

            template <class V>
            struct debug_format_trait<std::error_code, V> {
                void operator()(std::ostream &oss,
                                std::error_code const &t) const {
                    oss << DEBUG_ERROR_CODE_BRACE[0];
                    if (t) {
                        oss << t.category().name() << DEBUG_ERROR_CODE_INFIX
#  if DEBUG_ERROR_CODE_SHOW_NUMBER
                            << ' ' << t.value()
#  endif
                            << DEBUG_ERROR_CODE_POSTFIX << t.message();
                    } else {
                        oss << DEBUG_ERROR_CODE_NO_ERROR;
                    }
                    oss << DEBUG_ERROR_CODE_BRACE[1];
                }
            };

            template <class V>
            struct debug_format_trait<std::error_condition, V> {
                void operator()(std::ostream &oss,
                                std::error_condition const &t) const {
                    oss << DEBUG_UNKNOWN_TYPE_BRACE[0];
                    if (t) {
                        oss << t.category().name() << " error " << t.value()
                            << ": " << t.message();
                    } else {
                        oss << "no error";
                    }
                    oss << DEBUG_UNKNOWN_TYPE_BRACE[1];
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    debug_cond_is_member_repr<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    debug_format(oss, raw_repr_if_string(t.DEBUG_REPR_NAME()));
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    debug_cond_is_member_repr_stream<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    t.DEBUG_REPR_NAME(oss);
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    debug_cond_is_adl_repr<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    debug_format(oss, raw_repr_if_string(DEBUG_REPR_NAME(t)));
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    !debug_cond_is_adl_repr<T>::value &&
                    debug_cond_is_adl_repr_stream<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    DEBUG_REPR_NAME(oss, t);
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    !debug_cond_is_adl_repr<T>::value &&
                    !debug_cond_is_adl_repr_stream<T>::value &&
                    debug_cond_is_member_repr_debug<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    t.DEBUG_FORMATTER_REPR_NAME(debug_formatter{oss});
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    !debug_cond_is_adl_repr<T>::value &&
                    !debug_cond_is_adl_repr_stream<T>::value &&
                    !debug_cond_is_member_repr_debug<T>::value &&
                    debug_cond_is_adl_repr_debug<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    DEBUG_FORMATTER_REPR_NAME(debug_formatter{oss}, t);
                }
            };

            struct debug_visit_lambda {
                std::ostream &oss;

                template <class T>
                void operator()(T const &t) const {
                    debug_format(oss, t);
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    !debug_cond_is_adl_repr<T>::value &&
                    !debug_cond_is_adl_repr_stream<T>::value &&
                    debug_cond_is_variant<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    visit(debug_visit_lambda{oss}, t);
                }
            };

            template <class T>
            struct debug_format_trait<
                T,
                typename std::enable_if<
                    !debug_cond_string<T>::value &&
                    !debug_cond_bool<T>::value && !debug_cond_char<T>::value &&
                    !debug_cond_unicode_char<T>::value &&
                    !debug_cond_integral_unsigned<T>::value &&
                    !debug_cond_integral<T>::value &&
                    !debug_cond_floating_point<T>::value &&
                    !debug_cond_is_smart_pointer<T>::value &&
                    !debug_cond_error_code<T>::value &&
                    !debug_cond_reference_wrapper<T>::value &&
                    !debug_cond_is_ostream_ok<T>::value &&
                    !debug_cond_pointer<T>::value &&
                    !debug_cond_is_range<T>::value &&
                    !debug_cond_is_tuple<T>::value &&
                    !debug_cond_enum<T>::value &&
                    !debug_cond_is_member_repr<T>::value &&
                    !debug_cond_is_member_repr_stream<T>::value &&
                    !debug_cond_is_adl_repr<T>::value &&
                    !debug_cond_is_adl_repr_stream<T>::value &&
                    !debug_cond_is_variant<T>::value &&
                    debug_cond_is_optional<T>::value>::type> {
                void operator()(std::ostream &oss, T const &t) const {
                    if (static_cast<bool>(t)) {
                        debug_format(oss, debug_deref_avoid(t));
                    } else {
                        oss << DEBUG_NULLOPT_STRING;
                    }
                }
            };
# endif
    std::ostringstream oss;

    enum {
        silent = 0,
        print = 1,
        panic = 2,
        supress = 3,
    } state;

    DEBUG_SOURCE_LOCATION loc;
# if DEBUG_SHOW_TIMESTAMP == 2
#  if __cpp_inline_variables
    static inline std::chrono::steady_clock::time_point const tp0 =
        std::chrono::steady_clock::now();
#  endif
# endif
    debug &add_location_marks() {
# if DEBUG_SHOW_TIMESTAMP == 1
#  ifdef DEBUG_HAS_SYS_TIME_H
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        std::tm now = *std::localtime(&tv.tv_sec);
        oss << std::put_time(&now, "%H:%M:%S.");
        auto flags = oss.flags();
        oss << std::setw(3) << std::setfill('0');
        oss << (tv.tv_usec / 1000) % 1000;
        oss.flags(flags);
#  else
        auto tp = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm now = *std::gmtime(&t);
        oss << std::put_time(&now, "%H:%M:%S.");
        auto flags = oss.flags();
        oss << std::setw(3) << std::setfill('0');
        oss << std::chrono::duration_cast<std::chrono::milliseconds>(
                   tp.time_since_epoch())
                       .count() %
                   1000;
        oss.flags(flags);
#  endif
        oss << ' ';
# elif DEBUG_SHOW_TIMESTAMP == 2
#  if __cpp_inline_variables
                auto dur = std::chrono::steady_clock::now() - tp0;
#  else
                static std::chrono::steady_clock::time_point const tp0 =
                    std::chrono::steady_clock::now();
                auto dur = std::chrono::steady_clock::now() - tp0;
#  endif
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(dur)
                        .count();
                auto flags = oss.flags();
                oss << std::setw(3) << std::setfill(' ');
                oss << (elapsed / 1000) % 1000;
                oss << '.';
                oss << std::setw(3) << std::setfill('0');
                oss << elapsed % 1000;
                oss.flags(flags);
                oss << ' ';
# endif
# if DEBUG_SHOW_THREAD_ID
        oss << '[' << std::this_thread::get_id() << ']' << ' ';
# endif
        char const *fn = loc.file_name();
        for (char const *fp = fn; *fp; ++fp) {
            if (*fp == '/') {
                fn = fp + 1;
            }
        }
# if DEBUG_SHOW_LOCATION
        oss << fn << DEBUG_SEPARATOR_FILE << loc.line() << DEBUG_SEPARATOR_LINE
            << DEBUG_SEPARATOR_TAB;
# endif
# if DEBUG_SHOW_SOURCE_CODE_LINE
        {
            static thread_local std::unordered_map<std::string, std::string>
                fileCache;
            auto key = std::to_string(loc.line()) + loc.file_name();
            if (auto it = fileCache.find(key);
                it != fileCache.end() && !it->second.empty()) {
                DEBUG_LIKELY {
                    oss << DEBUG_SOURCE_LINE_BRACE[0] << it->second
                        << DEBUG_SOURCE_LINE_BRACE[1];
                }
            } else if (auto file = std::ifstream(loc.file_name());
                       file.is_open()) {
                DEBUG_LIKELY {
                    std::string line;
                    for (std::uint32_t i = 0; i < loc.line(); ++i) {
                        if (!std::getline(file, line)) {
                            DEBUG_UNLIKELY {
                                line.clear();
                                break;
                            }
                        }
                    }
                    if (auto pos = line.find_first_not_of(" \t\r\n");
                        pos != line.npos) {
                        DEBUG_LIKELY {
                            line = line.substr(pos);
                        }
                    }
                    if (!line.empty()) {
                        DEBUG_LIKELY {
                            if (line.back() == ';') {
                                DEBUG_LIKELY {
                                    line.pop_back();
                                }
                            }
                            oss << DEBUG_SOURCE_LINE_BRACE[0] << line
                                << DEBUG_SOURCE_LINE_BRACE[1];
                        }
                    }
                    fileCache.try_emplace(key, std::move(line));
                }
            } else {
                oss << DEBUG_SOURCE_LINE_BRACE[0] << '?'
                    << DEBUG_SOURCE_LINE_BRACE[1];
                fileCache.try_emplace(key);
            }
        }
# endif
        oss << ' ';
        return *this;
    }

    template <class T>
    struct DEBUG_NODISCARD debug_condition {
    private:
        debug &d;
        T const &t;

        template <class U>
        debug &check(bool cond, U const &u, char const *sym) {
            if (!cond) {
                DEBUG_UNLIKELY {
                    d.on_error("assertion failed:") << t << sym << u;
                }
            }
            return d;
        }

    public:
        explicit debug_condition(debug &d, T const &t) noexcept : d(d), t(t) {}

        template <class U>
        debug &operator<(U const &u) {
            return check(t < u, u, "<");
        }

        template <class U>
        debug &operator>(U const &u) {
            return check(t > u, u, ">");
        }

        template <class U>
        debug &operator<=(U const &u) {
            return check(t <= u, u, "<=");
        }

        template <class U>
        debug &operator>=(U const &u) {
            return check(t >= u, u, ">=");
        }

        template <class U>
        debug &operator==(U const &u) {
            return check(t == u, u, "==");
        }

        template <class U>
        debug &operator!=(U const &u) {
            return check(t != u, u, "!=");
        }
    };

    debug &on_error(char const *msg) {
        if (state != supress) {
            state = panic;
            add_location_marks();
        } else {
            oss << ' ';
        }
        oss << msg;
        return *this;
    }

    template <class T>
    debug &on_print(T const &t) {
        if (state == supress) {
            return *this;
        }
        if (state == silent) {
            state = print;
            add_location_marks();
        } else {
            oss << ' ';
        }
        debug_format(oss, t);
        return *this;
    }
# if DEBUG_ENABLE_FILES_MATCH
    static bool file_detected(char const *file) noexcept {
        static auto files = std::getenv("DEBUG_FILES");
        if (!files) {
            return true;
        }
        DEBUG_STRING_VIEW sv = files;
        /* std::size_t pos = 0, nextpos; */
        /* while ((nextpos = sv.find(' ')) != sv.npos) { */
        /*     if (sv.substr(pos, nextpos - pos) == tag) { */
        /*         return true; */
        /*     } */
        /*     pos = nextpos + 1; */
        /* } */
        if (sv.find(file) != sv.npos) {
            return true;
        }
        return false;
    }
# endif
public:
    explicit debug(bool enable = true,
                   DEBUG_SOURCE_LOCATION const &loc =
                       DEBUG_SOURCE_LOCATION::current()) noexcept
        : state(enable
# if DEBUG_ENABLE_FILES_MATCH
                        && file_detected(loc.file_name())
# endif
                    ? silent
                    : supress),
          loc(loc) {
    }

    debug &setloc(DEBUG_SOURCE_LOCATION const &newloc =
                      DEBUG_SOURCE_LOCATION::current()) noexcept {
        loc = newloc;
        return *this;
    }

    debug &noloc() noexcept {
        if (state == silent) {
            state = print;
        }
        return *this;
    }

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug_condition<T> check(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    template <class T>
    debug_condition<T> operator>>(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    debug &fail(bool fail = true) {
        if (fail) {
            DEBUG_UNLIKELY {
                on_error("failed:");
            }
        } else {
            state = supress;
        }
        return *this;
    }

    debug &on(bool enable) {
        if (!enable) {
            DEBUG_LIKELY {
                state = supress;
            }
        }
        return *this;
    }

    template <class T>
    debug &operator<<(T const &t) {
        return on_print(t);
    }

    template <class T>
    debug &operator,(T const &t) {
        return on_print(t);
    }

    ~debug()
# if DEBUG_PANIC_METHOD == 0
        noexcept(false)
# endif
    {
        if (state == panic) {
            DEBUG_UNLIKELY {
# if DEBUG_PANIC_METHOD == 0
                throw std::runtime_error(oss.str());
# elif DEBUG_PANIC_METHOD == 1
                        oss << '\n';
                        DEBUG_OUTPUT(oss.str());
#  if defined(DEBUG_PANIC_CUSTOM_TRAP)
                        DEBUG_PANIC_CUSTOM_TRAP;
                        return;
#  elif defined(_MSC_VER)
                        __debugbreak();
                        return;
#  elif defined(__GNUC__) && defined(__has_builtin)
#   if __has_builtin(__builtin_trap)
                        __builtin_trap();
                        return;
#   else
                        std::terminate();
#   endif
#  else
                        std::terminate();
#  endif
# elif DEBUG_PANIC_METHOD == 2
                oss << '\n';
                DEBUG_OUTPUT(oss.str());
                std::terminate();
# else
                oss << '\n';
                DEBUG_OUTPUT(oss.str());
                return;
# endif
            }
        }
        if (state == print) {
            oss << '\n';
            DEBUG_OUTPUT(oss.str());
        }
# if DEBUG_STEPPING == 1
        static std::mutex mutex;
        std::lock_guard lock(mutex);
#  ifdef DEBUG_CUSTOM_STEPPING
        DEBUG_CUSTOM_STEPPING(msg);
#  elif DEBUG_STEPPING_HAS_TERMIOS
        struct termios tc, oldtc;
        bool tty = isatty(0);
        if (tty) {
            tcgetattr(0, &oldtc);
            tc = oldtc;
            tc.c_lflag &= ~ICANON;
            tc.c_lflag &= ~ECHO;
            tcsetattr(0, TCSANOW, &tc);
        }
        std::cerr << "--More--" << std::flush;
        static char buf[100];
        read(0, buf, sizeof buf);
        std::cerr << "\r        \r";
        if (tty) {
            tcsetattr(0, TCSANOW, &oldtc);
        }
#  elif DEBUG_STEPPING_HAS_GETCH
        std::cerr << "--More--" << std::flush;
        getch();
        std::cerr << "\r        \r";
#  else
        std::cerr << "[Press ENTER to continue]" << std::flush;
        std::string line;
        std::getline(std::cin, line);
#  endif
# elif DEBUG_STEPPING == 2
#  if defined(DEBUG_PANIC_CUSTOM_TRAP)
                DEBUG_PANIC_CUSTOM_TRAP;
                return;
#  elif defined(_MSC_VER)
                __debugbreak();
#  elif defined(__GNUC__) && defined(__has_builtin)
#   if __has_builtin(__builtin_trap)
                __builtin_trap();
#   else
                std::terminate();
#   endif
#  endif
# endif
    }

    operator std::string() {
        std::string ret = oss.str();
        state = supress;
        return ret;
    }

    template <class T>
    struct named_member_t {
        char const *name;
        T const &value;

        void DEBUG_REPR_NAME(std::ostream &os) const {
            os << name << DEBUG_NAMED_MEMBER_MARK;
            debug_format(os, value);
        }
    };

    template <class T>
    static named_member_t<T> named_member(char const *name, T const &value) {
        return {name, value};
    }

    template <class T>
    struct raw_repr_t {
        T const &value;

        void DEBUG_REPR_NAME(std::ostream &os) const {
            os << value;
        }
    };

    template <class T>
    static raw_repr_t<T> raw_repr(T const &value) {
        return {value};
    }

    template <class T, typename std::enable_if<
                           std::is_convertible<T, std::string>::value ||
                               std::is_convertible<T, DEBUG_STRING_VIEW>::value,
                           int>::type = 0>
    static raw_repr_t<T> raw_repr_if_string(T const &value) {
        return {value};
    }

    template <class T, typename std::enable_if<
                           !(std::is_convertible<T, std::string>::value ||
                             std::is_convertible<T, DEBUG_STRING_VIEW>::value),
                           int>::type = 0>
    static T const &raw_repr_if_string(T const &value) {
        return value;
    }
};
# if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#  define DEBUG_REPR(...) \
      __pragma( \
          message("Please turn on /Zc:preprocessor before using DEBUG_REPR!"))
#  define DEBUG_REPR_GLOBAL(...) \
      __pragma( \
          message("Please turn on /Zc:preprocessor before using DEBUG_REPR!"))
#  define DEBUG_REPR_GLOBAL_TEMPLATED(...) \
      __pragma( \
          message("Please turn on /Zc:preprocessor before using DEBUG_REPR!"))
#  define DEBUG_PP_VA_OPT_SUPPORT(...) 0
# else
#  define DEBUG_PP_CONCAT_(a, b)                             a##b
#  define DEBUG_PP_CONCAT(a, b)                              DEBUG_PP_CONCAT_(a, b)
#  define DEBUG_PP_GET_1(a, ...)                             a
#  define DEBUG_PP_GET_2(a, b, ...)                          b
#  define DEBUG_PP_GET_3(a, b, c, ...)                       c
#  define DEBUG_PP_GET_4(a, b, c, d, ...)                    d
#  define DEBUG_PP_GET_5(a, b, c, d, e, ...)                 e
#  define DEBUG_PP_GET_6(a, b, c, d, e, f, ...)              f
#  define DEBUG_PP_GET_7(a, b, c, d, e, f, g, ...)           g
#  define DEBUG_PP_GET_8(a, b, c, d, e, f, g, h, ...)        h
#  define DEBUG_PP_GET_9(a, b, c, d, e, f, g, h, i, ...)     i
#  define DEBUG_PP_GET_10(a, b, c, d, e, f, g, h, i, j, ...) j
#  define DEBUG_PP_VA_EMPTY_(...)                            DEBUG_PP_GET_2(__VA_OPT__(, ) 0, 1, )
#  define DEBUG_PP_VA_OPT_SUPPORT                            !DEBUG_PP_VA_EMPTY_
#  if DEBUG_PP_VA_OPT_SUPPORT(?)
#   define DEBUG_PP_VA_EMPTY(...) DEBUG_PP_VA_EMPTY_(__VA_ARGS__)
#  else
#   define DEBUG_PP_VA_EMPTY(...) 0
#  endif
#  define DEBUG_PP_IF(a, t, f)     DEBUG_PP_IF_(a, t, f)
#  define DEBUG_PP_IF_(a, t, f)    DEBUG_PP_IF__(a, t, f)
#  define DEBUG_PP_IF__(a, t, f)   DEBUG_PP_IF___(DEBUG_PP_VA_EMPTY a, t, f)
#  define DEBUG_PP_IF___(a, t, f)  DEBUG_PP_IF____(a, t, f)
#  define DEBUG_PP_IF____(a, t, f) DEBUG_PP_IF_##a(t, f)
#  define DEBUG_PP_IF_0(t, f)      DEBUG_PP_UNWRAP_BRACE(f)
#  define DEBUG_PP_IF_1(t, f)      DEBUG_PP_UNWRAP_BRACE(t)
#  define DEBUG_PP_NARG(...) \
      DEBUG_PP_IF((__VA_ARGS__), (0), \
                  (DEBUG_PP_NARG_(__VA_ARGS__, 26, 25, 24, 23, 22, 21, 20, 19, \
                                  18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, \
                                  6, 5, 4, 3, 2, 1)))
#  define DEBUG_PP_NARG_(...) DEBUG_PP_NARG__(__VA_ARGS__)
#  define DEBUG_PP_NARG__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                          _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, \
                          _23, _24, _25, _26, N, ...) \
      N
#  define DEBUG_PP_FOREACH(f, ...) \
      DEBUG_PP_FOREACH_(DEBUG_PP_NARG(__VA_ARGS__), f, __VA_ARGS__)
#  define DEBUG_PP_FOREACH_(N, f, ...)  DEBUG_PP_FOREACH__(N, f, __VA_ARGS__)
#  define DEBUG_PP_FOREACH__(N, f, ...) DEBUG_PP_FOREACH_##N(f, __VA_ARGS__)
#  define DEBUG_PP_FOREACH_0(f, ...)
#  define DEBUG_PP_FOREACH_1(f, a)                f(a)
#  define DEBUG_PP_FOREACH_2(f, a, b)             f(a) f(b)
#  define DEBUG_PP_FOREACH_3(f, a, b, c)          f(a) f(b) f(c)
#  define DEBUG_PP_FOREACH_4(f, a, b, c, d)       f(a) f(b) f(c) f(d)
#  define DEBUG_PP_FOREACH_5(f, a, b, c, d, e)    f(a) f(b) f(c) f(d) f(e)
#  define DEBUG_PP_FOREACH_6(f, a, b, c, d, e, g) f(a) f(b) f(c) f(d) f(e) f(g)
#  define DEBUG_PP_FOREACH_7(f, a, b, c, d, e, g, h) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h)
#  define DEBUG_PP_FOREACH_8(f, a, b, c, d, e, g, h, i) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i)
#  define DEBUG_PP_FOREACH_9(f, a, b, c, d, e, g, h, i, j) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j)
#  define DEBUG_PP_FOREACH_10(f, a, b, c, d, e, g, h, i, j, k) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k)
#  define DEBUG_PP_FOREACH_11(f, a, b, c, d, e, g, h, i, j, k, l) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l)
#  define DEBUG_PP_FOREACH_12(f, a, b, c, d, e, g, h, i, j, k, l, m) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m)
#  define DEBUG_PP_FOREACH_13(f, a, b, c, d, e, g, h, i, j, k, l, m, n) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n)
#  define DEBUG_PP_FOREACH_14(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o)
#  define DEBUG_PP_FOREACH_15(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) f(p)
#  define DEBUG_PP_FOREACH_16(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q)
#  define DEBUG_PP_FOREACH_17(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r)
#  define DEBUG_PP_FOREACH_18(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s)
#  define DEBUG_PP_FOREACH_19(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t)
#  define DEBUG_PP_FOREACH_20(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u)
#  define DEBUG_PP_FOREACH_21(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u, v) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u) f(v)
#  define DEBUG_PP_FOREACH_22(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u, v, w) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w)
#  define DEBUG_PP_FOREACH_23(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u, v, w, x) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x)
#  define DEBUG_PP_FOREACH_24(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u, v, w, x, y) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y)
#  define DEBUG_PP_FOREACH_25(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, p, \
                              q, r, s, t, u, v, w, x, y, z) \
      f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
          f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y) f(z)
#  define DEBUG_PP_STRINGIFY(...)     DEBUG_PP_STRINGIFY(__VA_ARGS__)
#  define DEBUG_PP_STRINGIFY_(...)    #__VA_ARGS__
#  define DEBUG_PP_EXPAND(...)        DEBUG_PP_EXPAND_(__VA_ARGS__)
#  define DEBUG_PP_EXPAND_(...)       __VA_ARGS__
#  define DEBUG_PP_UNWRAP_BRACE(...)  DEBUG_PP_UNWRAP_BRACE_ __VA_ARGS__
#  define DEBUG_PP_UNWRAP_BRACE_(...) __VA_ARGS__
#  define DEBUG_REPR_ON_EACH(x) \
      if (add_comma) \
          formatter.os << DEBUG_TUPLE_COMMA; \
      else \
          add_comma = true; \
      formatter.os << #x DEBUG_NAMED_MEMBER_MARK; \
      formatter << x;
#  define DEBUG_REPR(...) \
      template <class debug_formatter> \
      void DEBUG_FORMATTER_REPR_NAME(debug_formatter formatter) const { \
          formatter.os << DEBUG_TUPLE_BRACE[0]; \
          bool add_comma = false; \
          DEBUG_PP_FOREACH(DEBUG_REPR_ON_EACH, __VA_ARGS__) \
          formatter.os << DEBUG_TUPLE_BRACE[1]; \
      }
#  define DEBUG_REPR_GLOBAL_ON_EACH(x) \
      if (add_comma) \
          formatter.os << DEBUG_TUPLE_COMMA; \
      else \
          add_comma = true; \
      formatter.os << #x DEBUG_NAMED_MEMBER_MARK; \
      formatter << object.x;
#  define DEBUG_REPR_GLOBAL(T, ...) \
      template <class debug_formatter> \
      void DEBUG_FORMATTER_REPR_NAME(debug_formatter formatter, \
                                     T const &object) { \
          formatter.os << DEBUG_TUPLE_BRACE[0]; \
          bool add_comma = false; \
          DEBUG_PP_FOREACH(DEBUG_REPR_GLOBAL_ON_EACH, __VA_ARGS__) \
          formatter.os << DEBUG_TUPLE_BRACE[1]; \
      }
#  define DEBUG_REPR_GLOBAL_TEMPLATED(T, Tmpls, TmplsClassed, ...) \
      template <class debug_formatter, DEBUG_PP_UNWRAP_BRACE(TmplsClassed)> \
      void DEBUG_FORMATTER_REPR_NAME( \
          debug_formatter formatter, \
          T<DEBUG_PP_UNWRAP_BRACE(Tmpls)> const &object) { \
          formatter.os << DEBUG_TUPLE_BRACE[0]; \
          bool add_comma = false; \
          DEBUG_PP_FOREACH(DEBUG_REPR_GLOBAL_ON_EACH, __VA_ARGS__) \
          formatter.os << DEBUG_TUPLE_BRACE[1]; \
      }
# endif
DEBUG_NAMESPACE_END
#else
# include <string>
DEBUG_NAMESPACE_BEGIN

struct debug {
    debug(bool = true, char const * = nullptr) noexcept {}

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug &operator,(T const &) {
        return *this;
    }

    template <class T>
    debug &operator<<(T const &) {
        return *this;
    }

    debug &on(bool) {
        return *this;
    }

    debug &fail(bool = true) {
        return *this;
    }

    ~debug() noexcept(false) {}

private:
    struct debug_condition {
        debug &d;

        explicit debug_condition(debug &d) : d(d) {}

        template <class U>
        debug &operator<(U const &) {
            return d;
        }

        template <class U>
        debug &operator>(U const &) {
            return d;
        }

        template <class U>
        debug &operator<=(U const &) {
            return d;
        }

        template <class U>
        debug &operator>=(U const &) {
            return d;
        }

        template <class U>
        debug &operator==(U const &) {
            return d;
        }

        template <class U>
        debug &operator!=(U const &) {
            return d;
        }
    };

public:
    template <class... Ts>
    debug &setloc(Ts &&...ts) noexcept {
        return *this;
    }

    debug &noloc() noexcept {
        return *this;
    }

    template <class T>
    debug_condition check(T const &) noexcept {
        return debug_condition{*this};
    }

    template <class T>
    debug_condition operator>>(T const &) noexcept {
        return debug_condition{*this};
    }

    operator std::string() {
        return {};
    }

    template <class T>
    struct named_member_t {
        char const *name;
        T const &value;
    };

    template <class T>
    static named_member_t<T> named_member(char const *name, T const &value) {
        return {name, value};
    }

    template <class T>
    struct raw_repr_t {
        T const &value;
    };

    template <class T>
    static raw_repr_t<T> raw_repr(T const &value) {
        return {value};
    }

    template <class T>
    static T raw_repr_if_string(T const &value) {
        return value;
    }
};

# define DEBUG_REPR(...)
DEBUG_NAMESPACE_END
#endif
#ifdef DEBUG_CLASS_NAME
# undef debug
# if DEBUG_LEVEL
#  ifdef DEBUG_SOURCE_LOCATION_FAKER
#   define debug() debug(true, DEBUG_SOURCE_LOCATION_FAKER)
#  endif
#  undef debug
# elif DEBUG_LEVEL
#  ifdef DEBUG_SOURCE_LOCATION_FAKER
#   define debug() debug(true, DEBUG_SOURCE_LOCATION_FAKER)
#  endif
# endif
#endif





/* #include <array> */
/* #include <cstddef> */
/* #include <cstdint> */
/* #include <map> */
/* #include <memory> */
/* #include <optional> */
/* #include <string> */
/* #include <string_view> */
/* #include <type_traits> */
/* #include <unordered_map> */
/* #include <variant> */
/* #include <vector> */

namespace co_async {
#if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
# define REFLECT(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT_GLOBAL(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT_GLOBAL_TEMPLATED(...) \
     __pragma(message("Please turn on /Zc:preprocessor before using " \
                      "REFLECT!"))
# define REFLECT__PP_VA_OPT_SUPPORT(...) 0
#else
# define REFLECT__PP_CONCAT_(a, b)          a##b
# define REFLECT__PP_CONCAT(a, b)           REFLECT__PP_CONCAT_(a, b)
# define REFLECT__PP_GET_1(a, ...)          a
# define REFLECT__PP_GET_2(a, b, ...)       b
# define REFLECT__PP_GET_3(a, b, c, ...)    c
# define REFLECT__PP_GET_4(a, b, c, d, ...) d
# define REFLECT__PP_VA_EMPTY_(...)         REFLECT__PP_GET_2(__VA_OPT__(, ) 0, 1, )
# define REFLECT__PP_VA_OPT_SUPPORT         !REFLECT__PP_VA_EMPTY_
# if REFLECT__PP_VA_OPT_SUPPORT(?)
#  define REFLECT__PP_VA_EMPTY(...) REFLECT__PP_VA_EMPTY_(__VA_ARGS__)
# else
#  define REFLECT__PP_VA_EMPTY(...) 0
# endif
# define REFLECT__PP_IF(a, t, f)  REFLECT__PP_IF_(a, t, f)
# define REFLECT__PP_IF_(a, t, f) REFLECT__PP_IF__(a, t, f)
# define REFLECT__PP_IF__(a, t, f) \
     REFLECT__PP_IF___(REFLECT__PP_VA_EMPTY a, t, f)
# define REFLECT__PP_IF___(a, t, f)  REFLECT__PP_IF____(a, t, f)
# define REFLECT__PP_IF____(a, t, f) REFLECT__PP_IF_##a(t, f)
# define REFLECT__PP_IF_0(t, f)      REFLECT__PP_UNWRAP_BRACE(f)
# define REFLECT__PP_IF_1(t, f)      REFLECT__PP_UNWRAP_BRACE(t)
# define REFLECT__PP_NARG(...) \
     REFLECT__PP_IF((__VA_ARGS__), (0), \
                    (REFLECT__PP_NARG_(__VA_ARGS__, 26, 25, 24, 23, 22, 21, \
                                       20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
                                       10, 9, 8, 7, 6, 5, 4, 3, 2, 1)))
# define REFLECT__PP_NARG_(...) REFLECT__PP_NARG__(__VA_ARGS__)
# define REFLECT__PP_NARG__(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                            _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, \
                            _23, _24, _25, _26, N, ...) \
     N
# define REFLECT__PP_FOREACH(f, ...) \
     REFLECT__PP_FOREACH_(REFLECT__PP_NARG(__VA_ARGS__), f, __VA_ARGS__)
# define REFLECT__PP_FOREACH_(N, f, ...) \
     REFLECT__PP_FOREACH__(N, f, __VA_ARGS__)
# define REFLECT__PP_FOREACH__(N, f, ...) \
     REFLECT__PP_FOREACH_##N(f, __VA_ARGS__)
# define REFLECT__PP_FOREACH_0(f, ...)
# define REFLECT__PP_FOREACH_1(f, a)             f(a)
# define REFLECT__PP_FOREACH_2(f, a, b)          f(a) f(b)
# define REFLECT__PP_FOREACH_3(f, a, b, c)       f(a) f(b) f(c)
# define REFLECT__PP_FOREACH_4(f, a, b, c, d)    f(a) f(b) f(c) f(d)
# define REFLECT__PP_FOREACH_5(f, a, b, c, d, e) f(a) f(b) f(c) f(d) f(e)
# define REFLECT__PP_FOREACH_6(f, a, b, c, d, e, g) \
     f(a) f(b) f(c) f(d) f(e) f(g)
# define REFLECT__PP_FOREACH_7(f, a, b, c, d, e, g, h) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h)
# define REFLECT__PP_FOREACH_8(f, a, b, c, d, e, g, h, i) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i)
# define REFLECT__PP_FOREACH_9(f, a, b, c, d, e, g, h, i, j) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j)
# define REFLECT__PP_FOREACH_10(f, a, b, c, d, e, g, h, i, j, k) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k)
# define REFLECT__PP_FOREACH_11(f, a, b, c, d, e, g, h, i, j, k, l) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l)
# define REFLECT__PP_FOREACH_12(f, a, b, c, d, e, g, h, i, j, k, l, m) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m)
# define REFLECT__PP_FOREACH_13(f, a, b, c, d, e, g, h, i, j, k, l, m, n) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n)
# define REFLECT__PP_FOREACH_14(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o)
# define REFLECT__PP_FOREACH_15(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) f(p)
# define REFLECT__PP_FOREACH_16(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q)
# define REFLECT__PP_FOREACH_17(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r)
# define REFLECT__PP_FOREACH_18(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s)
# define REFLECT__PP_FOREACH_19(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t)
# define REFLECT__PP_FOREACH_20(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u)
# define REFLECT__PP_FOREACH_21(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v)
# define REFLECT__PP_FOREACH_22(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w)
# define REFLECT__PP_FOREACH_23(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x)
# define REFLECT__PP_FOREACH_24(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x, y) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y)
# define REFLECT__PP_FOREACH_25(f, a, b, c, d, e, g, h, i, j, k, l, m, n, o, \
                                p, q, r, s, t, u, v, w, x, y, z) \
     f(a) f(b) f(c) f(d) f(e) f(g) f(h) f(i) f(j) f(k) f(l) f(m) f(n) f(o) \
         f(p) f(q) f(r) f(s) f(t) f(u) f(v) f(w) f(x) f(y) f(z)
# define REFLECT__PP_STRINGIFY(...)     REFLECT__PP_STRINGIFY(__VA_ARGS__)
# define REFLECT__PP_STRINGIFY_(...)    #__VA_ARGS__
# define REFLECT__PP_EXPAND(...)        REFLECT__PP_EXPAND_(__VA_ARGS__)
# define REFLECT__PP_EXPAND_(...)       __VA_ARGS__
# define REFLECT__PP_UNWRAP_BRACE(...)  REFLECT__PP_UNWRAP_BRACE_ __VA_ARGS__
# define REFLECT__PP_UNWRAP_BRACE_(...) __VA_ARGS__
# ifdef DEBUG_REPR
#  define REFLECT__EXTRA(...)        DEBUG_REPR(__VA_ARGS__)
#  define REFLECT_GLOBAL__EXTRA(...) DEBUG_REPR_GLOBAL(__VA_ARGS__)
#  define REFLECT_GLOBAL_TEMPLATED__EXTRA(...) \
      DEBUG_REPR_GLOBAL_TEMPLATED(__VA_ARGS__)
# else
#  define REFLECT__EXTRA(...)
#  define REFLECT_GLOBAL__EXTRA(...)
#  define REFLECT_GLOBAL_TEMPLATED__EXTRA(...)
# endif
# define REFLECT__ON_EACH(x) reflector(#x, x);
# define REFLECT(...) \
     template <class ReflectorT> \
     constexpr void REFLECT__MEMBERS(ReflectorT &reflector){ \
         REFLECT__PP_FOREACH(REFLECT__ON_EACH, \
                             __VA_ARGS__)} REFLECT__EXTRA(__VA_ARGS__)
# define REFLECT__GLOBAL_ON_EACH(x) \
     reflector(#x##_REFLECT__static_string, object.x);
# define REFLECT_GLOBAL(T, ...) \
     template <class ReflectorT> \
     constexpr void REFLECT__MEMBERS(ReflectorT &reflector, T &object){ \
         REFLECT__PP_FOREACH(REFLECT__GLOBAL_ON_EACH, \
                             __VA_ARGS__)} REFLECT_GLOBAL__EXTRA(__VA_ARGS__)
# define REFLECT_GLOBAL_TEMPLATED(T, Tmpls, TmplsClassed, ...) \
     template <class ReflectorT, REFLECT__PP_UNWRAP_BRACE(TmplsClassed)> \
     constexpr void REFLECT__MEMBERS( \
         ReflectorT &reflector, \
         T<REFLECT__PP_UNWRAP_BRACE(Tmpls)> &object){REFLECT__PP_FOREACH( \
         REFLECT__GLOBAL_ON_EACH, \
         __VA_ARGS__)} REFLECT_GLOBAL_TEMPLATED__EXTRA(__VA_ARGS__)
#endif
struct JsonValue {
    using Ptr = std::unique_ptr<JsonValue>;
    using Null = std::monostate;
    using String = std::string;
    using Dict = std::map<std::string, JsonValue::Ptr>;
    using Array = std::vector<JsonValue::Ptr>;
    using Integer = std::int64_t;
    using Real = double;
    using Boolean = bool;
    using Union =
        std::variant<Null, String, Dict, Array, Integer, Real, Boolean>;
    Union inner;

    template <class T>
    explicit JsonValue(std::in_place_type_t<T>, T &&value)
        : inner(std::in_place_type<T>, std::move(value)) {}

    template <class T>
    static Ptr make(T value) {
        return std::make_unique<JsonValue>(std::in_place_type<T>,
                                           std::move(value));
    }
};

/* template <class T> */
/* struct NoDefault { */
/* private: */
/*     T value; */
/*  */
/* public: */
/*     NoDefault(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>) */
/*         : value(std::move(value)) {} */
/*  */
/*     NoDefault(T const &value)
 * noexcept(std::is_nothrow_copy_constructible_v<T>) */
/*         : value(value) {} */
/*  */
/*     template <class U, class = std::enable_if_t<std::is_convertible_v<U, T>>>
 */
/*     NoDefault(U &&value) noexcept(std::is_nothrow_convertible_v<U, T>) */
/*         : value(std::forward<U>(value)) {} */
/*  */
/*     template <class = std::enable_if_t<std::is_convertible_v< */
/*                   std::initializer_list<typename T::value_type>, T>>> */
/*     NoDefault(std::initializer_list<typename T::value_type> value) noexcept(
 */
/*         std::is_nothrow_convertible_v< */
/*             std::initializer_list<typename T::value_type>, T>) */
/*         : value(std::move(value)) {} */
/*  */
/*     NoDefault(NoDefault const &) = default; */
/*     NoDefault &operator=(NoDefault const &) = default; */
/*     NoDefault(NoDefault &&) = default; */
/*     NoDefault &operator=(NoDefault &&) = default; */
/*  */
/*     T &operator*() noexcept { */
/*         return value; */
/*     } */
/*  */
/*     T const &operator*() const noexcept { */
/*         return value; */
/*     } */
/*  */
/*     T *operator->() noexcept { */
/*         return std::addressof(value); */
/*     } */
/*  */
/*     T const *operator->() const noexcept { */
/*         return std::addressof(value); */
/*     } */
/*  */
/*     using value_type = T; */
/* }; */

struct JsonEncoder;

template <class T, class = void>
struct JsonTrait {
    static void putValue(JsonEncoder *encoder, T const &value) {
        static_assert(!std::is_same_v<T, T>,
                      "the given type contains members that are not reflected, "
                      "please add REFLECT macro to it");
    }

    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        static_assert(!std::is_same_v<T, T>,
                      "the given type contains members that are not reflected, "
                      "please add REFLECT macro to it");
        return false;
    }
};

struct JsonEncoder {
    std::string json;

    void put(char c) {
        json.push_back(c);
    }

    void put(char const *s, std::size_t len) {
        json.append(s, len);
    }

    void putLiterialString(char const *name) {
        put('"');
        json.append(name);
        put('"');
    }

    void putString(char const *name, std::size_t len) {
        put('"');
        for (char const *it = name, *ep = name + len; it != ep; ++it) {
            char c = *it;
            switch (c) {
            case '\n': put("\\n", 2); break;
            case '\r': put("\\r", 2); break;
            case '\t': put("\\t", 2); break;
            case '\\': put("\\\\", 2); break;
            case '\0': put("\\0", 2); break;
            case '"':  put("\\\"", 2); break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F) {
                    put("\\u00", 4);
                    auto u = static_cast<unsigned char>(c);
                    put("0123456789abcdef"[u >> 4]);
                    put("0123456789abcdef"[u & 0x0F]);
                } else {
                    put(c);
                }
                break;
            }
        }
        put('"');
    }

    template <class T>
    void putArithmetic(T const &value) {
        json.append(std::to_string(value));
    }

    template <class T>
    void putValue(T const &value) {
        JsonTrait<T>::putValue(this, value);
    }
};
enum class JsonError : int {
    Success = 0,
    NullEntry,
    TypeMismatch,
    UnexpectedEnd,
    UnexpectedToken,
    NonTerminatedString,
    InvalidUTF16String,
    DictKeyNotString,
    InvalidNumberFormat,
    InvalidVariantType,
    NotImplemented,
};

inline std::error_category const &jsonCategory() {
    static struct final : std::error_category {
        char const *name() const noexcept override {
            return "json";
        }

        std::string message(int e) const override {
            using namespace std::string_literals;
            switch (static_cast<JsonError>(e)) {
            case JsonError::Success:         return "success"s;
            case JsonError::NullEntry:       return "no such entry"s;
            case JsonError::TypeMismatch:    return "type mismatch"s;
            case JsonError::UnexpectedEnd:   return "unexpected end"s;
            case JsonError::UnexpectedToken: return "unexpected token"s;
            case JsonError::NonTerminatedString:
                return "non-terminated string"s;
            case JsonError::InvalidUTF16String: return "invalid utf-16 string"s;
            case JsonError::DictKeyNotString:   return "dict key must be string"s;
            case JsonError::InvalidNumberFormat:
                return "invalid number format"s;
            case JsonError::InvalidVariantType:
                return "invalid variant type"s;
            case JsonError::NotImplemented: return "not implemented"s;
            default:                        return "unknown error"s;
            }
        }
    } instance;

    return instance;
}

inline std::error_code make_error_code(JsonError e) {
    return std::error_code(static_cast<int>(e), jsonCategory());
}

inline JsonValue::Ptr jsonParse(std::string_view &json, std::error_code &ec) {
    using namespace std::string_view_literals;
    JsonValue::Ptr current;
    auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
    if (nonempty == json.npos) {
        ec = make_error_code(JsonError::UnexpectedEnd);
        return nullptr;
    }
    json.remove_prefix(nonempty);
    char c = json.front();
    if (c == '"') {
        json.remove_prefix(1);
        std::string str;
        unsigned int phase = 0;
        unsigned int lasthex = 0;
        unsigned int hex = 0;
        std::size_t i;
        auto unsignedExtent = [](unsigned int x) {
            return static_cast<char>(static_cast<unsigned char>(x));
        };
        for (i = 0;; ++i) {
            if (i == json.size()) {
                ec = make_error_code(JsonError::NonTerminatedString);
                return nullptr;
            }
            char c = json[i];
            if (phase == 0) {
                if (c == '"') {
                    break;
                } else if (c == '\\') {
                    phase = 1;
                    continue;
                }
            } else if (phase == 1) {
                if (c == 'u') {
                    phase = 2;
                    hex = 0;
                    lasthex = false;
                    continue;
                } else if (c == 'n') {
                    c = '\n';
                } else if (c == 't') {
                    c = '\t';
                } else if (c == '\\') {
                    c = '\\';
                } else if (c == '0') {
                    c = '\0';
                } else if (c == 'r') {
                    c = '\r';
                } else if (c == 'v') {
                    c = '\v';
                } else if (c == 'f') {
                    c = '\f';
                } else if (c == 'b') {
                    c = '\b';
                } else if (c == 'a') {
                    c = '\a';
                }
                phase = 0;
            } else {
                hex <<= 4;
                if ('0' <= c && c <= '9') {
                    hex |= static_cast<unsigned int>(c - '0');
                } else if ('a' <= c && c <= 'f') {
                    hex |= static_cast<unsigned int>(c - 'a' + 10);
                } else if ('A' <= c && c <= 'F') {
                    hex |= static_cast<unsigned int>(c - 'A' + 10);
                }
                if (phase == 5) {
                    if (0xD800 <= hex && hex < 0xDC00) {
                        if (!lasthex) {
                            phase = 2;
                            lasthex = hex;
                            hex = 0;
                            continue;
                        } else {
                            ec = make_error_code(JsonError::InvalidUTF16String);
                            return nullptr;
                        }
                    } else if (0xDC00 <= hex && hex < 0xE000) {
                        if (lasthex) {
                            hex = 0x10000 + (lasthex - 0xD800) * 0x400 +
                                  (hex - 0xDC00);
                            lasthex = false;
                            phase = 0;
                        } else {
                            ec = make_error_code(JsonError::InvalidUTF16String);
                            return nullptr;
                        }
                    }
                    if (hex <= 0x7F) {
                        str.push_back(unsignedExtent(hex));
                    } else if (hex <= 0x7FF) {
                        str.push_back(unsignedExtent(0xC0 | (hex >> 6)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0xFFFF) {
                        str.push_back(unsignedExtent(0xE0 | (hex >> 12)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0x10FFFF) {
                        str.push_back(unsignedExtent(0xF0 | (hex >> 18)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 12) & 0x3F)));
                        str.push_back(
                            unsignedExtent(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(unsignedExtent(0x80 | (hex & 0x3F)));
                    } else {
                        ec = make_error_code(JsonError::InvalidUTF16String);
                        return nullptr;
                    }
                    phase = 0;
                } else {
                    ++phase;
                }
                continue;
            }
            str.push_back(c);
        }
        json.remove_prefix(i + 1);
        current = JsonValue::make<JsonValue::String>(std::move(str));
    } else if (c == '{') {
        json.remove_prefix(1);
        std::map<std::string, JsonValue::Ptr> dict;
        for (;;) {
            nonempty = json.find_first_not_of(" \t\n\r\0"sv);
            if (nonempty == json.npos) {
                ec = make_error_code(JsonError::UnexpectedEnd);
                return nullptr;
            }
            json.remove_prefix(nonempty);
            if (json.front() == '}') {
                json.remove_prefix(1);
                break;
            } else if (json.front() == ',') {
                json.remove_prefix(1);
                continue;
            } else {
                auto key = jsonParse(json, ec);
                if (!key) {
                    return nullptr;
                }
                std::string keyString;
                if (auto p = std::get_if<JsonValue::String>(&key->inner)) {
                    keyString = std::move(*p);
                } else {
                    ec = make_error_code(JsonError::DictKeyNotString);
                    return nullptr;
                }
                auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
                if (nonempty == json.npos) {
                    ec = make_error_code(JsonError::UnexpectedEnd);
                    return nullptr;
                }
                json.remove_prefix(nonempty);
                if (json.front() != ':') {
                    ec = make_error_code(JsonError::UnexpectedToken);
                    return nullptr;
                }
                json.remove_prefix(1);
                auto value = jsonParse(json, ec);
                if (!value) {
                    return nullptr;
                }
                dict.emplace(std::move(keyString), std::move(value));
            }
        }
        current = JsonValue::make<JsonValue::Dict>(std::move(dict));
    } else if (c == '[') {
        json.remove_prefix(1);
        std::vector<JsonValue::Ptr> array;
        for (;;) {
            auto nonempty = json.find_first_not_of(" \t\n\r\0"sv);
            if (nonempty == json.npos) {
                ec = make_error_code(JsonError::UnexpectedEnd);
                return nullptr;
            }
            json.remove_prefix(nonempty);
            if (json.front() == ']') {
                json.remove_prefix(1);
                break;
            } else if (json.front() == ',') {
                json.remove_prefix(1);
                continue;
            } else {
                auto value = jsonParse(json, ec);
                if (!value) {
                    return nullptr;
                }
                array.emplace_back(std::move(value));
            }
        }
        current = JsonValue::make<JsonValue::Array>(std::move(array));
    } else if (('0' <= c && c <= '9') || c == '.' || c == '-' || c == '+') {
        auto end = json.find_first_of(",]}"sv);
        if (end == json.npos) {
            end = json.size();
        }
        auto str = std::string(json.data(), end);
        if (str.find('.') != str.npos) {
            double value;
            try {
                value = std::stod(str);
            } catch (std::exception const &) {
                ec = make_error_code(JsonError::InvalidNumberFormat);
                return nullptr;
            }
            current = JsonValue::make<JsonValue::Real>(value);
        } else {
            std::int64_t value;
            try {
                value = std::stoll(str);
            } catch (std::exception const &) {
                ec = make_error_code(JsonError::InvalidNumberFormat);
                return nullptr;
            }
            current = JsonValue::make<JsonValue::Integer>(value);
        }
        json.remove_prefix(end);
    } else if (c == 't') {
        if (!json.starts_with("true"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Boolean>(true);
        json.remove_prefix(4);
    } else if (c == 'f') {
        if (!json.starts_with("false"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Boolean>(false);
        json.remove_prefix(5);
    } else if (c == 'n') {
        if (!json.starts_with("null"sv)) {
            ec = make_error_code(JsonError::UnexpectedToken);
            return nullptr;
        }
        current = JsonValue::make<JsonValue::Null>(JsonValue::Null());
        json.remove_prefix(4);
    } else {
        ec = make_error_code(JsonError::UnexpectedToken);
        return nullptr;
    }
    return current;
}

struct ReflectorJsonEncode {
    JsonEncoder *encoder;
    bool comma = false;

    template <class T>
    void operator()(char const *name, T &value) {
        if (!comma) {
            comma = true;
        } else {
            encoder->put(',');
        }
        encoder->putLiterialString(name);
        encoder->put(':');
        encoder->putValue(value);
    }
};

struct ReflectorJsonDecode {
    JsonValue::Dict *currentDict;
    std::error_code ec{};
    bool failed = false;

    template <class T>
    void operator()(char const *name, T &value) {
        if (failed) {
            return;
        }
        auto it = currentDict->find(name);
        if (it == currentDict->end()) {
            JsonValue::Union nullData;
            failed = !JsonTrait<T>::getValue(nullData, value, ec);
        } else {
            failed = !JsonTrait<T>::getValue(it->second->inner, value, ec);
        }
    }

    static void typeMismatch(char const *expect, JsonValue::Union const &inner,
                             std::error_code &ec) {
        if (std::holds_alternative<JsonValue::Null>(inner)) {
#if DEBUG_LEVEL
            std::cerr << std::string("json_decode no such entry (expect ") +
                             expect + ", got null)\n";
#endif
            ec = make_error_code(JsonError::NullEntry);
        } else {
            char const *got = "???";
            std::visit(
                [&](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, JsonValue::Null>) {
                        got = "null";
                    } else if constexpr (std::is_same_v<T, JsonValue::String>) {
                        got = "string";
                    } else if constexpr (std::is_same_v<T, JsonValue::Dict>) {
                        got = "dict";
                    } else if constexpr (std::is_same_v<T, JsonValue::Array>) {
                        got = "array";
                    } else if constexpr (std::is_same_v<T,
                                                        JsonValue::Integer>) {
                        got = "integer";
                    } else if constexpr (std::is_same_v<T, JsonValue::Real>) {
                        got = "real";
                    } else if constexpr (std::is_same_v<T,
                                                        JsonValue::Boolean>) {
                        got = "boolean";
                    }
                },
                inner);
#if DEBUG_LEVEL
            std::cerr << std::string("json_decode type mismatch (expect ") +
                             expect + ", got " + got + ")\n";
#endif
            ec = make_error_code(JsonError::TypeMismatch);
        }
    }
};

struct JsonTraitPointerLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value == nullptr) {
            encoder->put("null", 4);
        } else {
            JsonTrait<typename std::pointer_traits<T>::element_type>::putValue(
                encoder, *value);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union const &inner, T &value,
                         std::error_code &ec) {
        JsonTrait<typename std::pointer_traits<T>::element_type>::getValue(
            inner, *value, ec);
        return !ec;
    }
};

struct JsonTraitArrayLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        auto bit = value.begin();
        auto eit = value.end();
        encoder->put('[');
        bool comma = false;
        for (auto it = bit; it != eit; ++it) {
            if (!comma) {
                comma = true;
            } else {
                encoder->put(',');
            }
            JsonTrait<typename T::value_type>::putValue(encoder, *it);
        }
        encoder->put(']');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Array>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.emplace_back();
                if (!JsonTrait<typename T::value_type>::getValue((*it)->inner,
                                                                 element, ec)) {
                    return false;
                }
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("array", data, ec);
            return false;
        }
    }
};

struct JsonTraitDictLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        auto bit = value.begin();
        auto eit = value.end();
        encoder->put('{');
        bool comma = false;
        for (auto it = bit; it != eit; ++it) {
            if (!comma) {
                comma = true;
            } else {
                encoder->put(',');
            }
            encoder->putString(it->first);
            encoder->put(':');
            JsonTrait<typename T::mapped_type>::putValue(encoder, it->second);
        }
        encoder->put('}');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            auto bit = p->begin();
            auto eit = p->end();
            for (auto it = bit; it != eit; ++it) {
                auto &element = value.try_emplace(it->first).first->second;
                if (!JsonTrait<typename T::mapped_type>::getValue(
                        it->second->inner, element, ec)) {
                    return false;
                }
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("dict", data, ec);
            return false;
        }
    }
};

struct JsonTraitStringLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putString(value.data(), value.size());
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::String>(&data)) {
            value = std::move(*p);
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("string", data, ec);
            return false;
        }
    }
};

struct JsonTraitNullLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->put("null", 4);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (std::get_if<JsonValue::Null>(&data)) {
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("null", data, ec);
            return false;
        }
    }
};

struct JsonTraitOptionalLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value) {
            encoder->putValue(*value);
        } else {
            encoder->put("null", 4);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (std::get_if<JsonValue::Null>(&data)) {
            value = std::nullopt;
            return true;
        } else {
            return JsonTrait<typename T::value_type>::getValue(
                data, value.emplace(), ec);
        }
    }
};

struct JsonTraitBooleanLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        if (value) {
            encoder->put("true", 4);
        } else {
            encoder->put("false", 5);
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Boolean>(&data)) {
            value = *p;
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("boolean", data, ec);
            return false;
        }
    }
};

struct JsonTraitArithmeticLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putArithmetic(value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Integer>(&data)) {
            value = static_cast<T>(*p);
            return true;
        } else if (auto p = std::get_if<JsonValue::Real>(&data)) {
            value = static_cast<T>(*p);
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("integer or real", data, ec);
            return false;
        }
    }
};

struct JsonTraitVariantLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        std::visit([&](auto const &arg) {
            using Arg = std::decay_t<decltype(arg)>;
            encoder->put('{');
            encoder->putLiterialString("type");
            encoder->put(':');
            encoder->putLiterialString(Arg::name);
            encoder->put(',');
            encoder->putLiterialString("object");
            encoder->put(':');
            encoder->putValue(arg);
            encoder->put('}');
        }, value);
    }

    template <class T, std::size_t ...Is>
    static bool getValueImpl(JsonValue::String const &name,
                             JsonValue::Union &object, T &value,
                             std::error_code &ec, std::index_sequence<Is...>) {
        int ret = 0;
        (void)((name == std::variant_alternative_t<Is, T>::name ?
            (ret = (JsonTrait<std::variant_alternative_t<Is, T>>
             ::getValue(object, value.template emplace<Is>(), ec) ? 2 : 1), true) :
            false) || ...);
        switch (ret) {
            case 2: return true;
            case 1: return false;
            default:
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
        }
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            if (auto type = p->find("type"); type == p->end()) {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            } else if (auto object = p->find("object"); object == p->end()) {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            } else if (auto p = std::get_if<JsonValue::String>(&type->second->inner)) {
                if (!getValueImpl(*p, object->second->inner, value, ec,
                                  std::make_index_sequence<std::variant_size_v<T>>())) {
                    return false;
                }
                return true;
            } else {
                ec = make_error_code(JsonError::InvalidVariantType);
                return false;
            }
        } else {
            ReflectorJsonDecode::typeMismatch("object", data, ec);
            return false;
        }
    }
};

struct JsonTraitJsonValueLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        encoder->putValue(value.inner);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, JsonValue &value,
                         std::error_code &ec) {
        value.inner = std::move(data);
        return true;
    }
};

struct JsonTraitObjectLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        ReflectorJsonEncode reflector(encoder);
        encoder->put('{');
        reflect_members(reflector, const_cast<T &>(value));
        encoder->put('}');
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        if (auto p = std::get_if<JsonValue::Dict>(&data)) {
            ReflectorJsonDecode reflector(p);
            reflect_members(reflector, value);
            if (reflector.failed) {
                ec = reflector.ec;
                return false;
            }
            return true;
        } else {
            ReflectorJsonDecode::typeMismatch("object", data, ec);
            return false;
        }
    }
};

struct JsonTraitWrapperLike {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        return JsonTrait<typename T::value_type>::putValue(encoder, *value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        return JsonTrait<typename T::value_type>::getValue(data, *value, ec);
    }
};

template <>
struct JsonTrait<JsonValue> {
    template <class T>
    static void putValue(JsonEncoder *encoder, T const &value) {
        std::visit([&]<class U>(
                       U const &arg) { JsonTrait<U>::getValue(encoder, arg); },
                   value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, JsonValue &value,
                         std::error_code &ec) {
        value.inner = std::move(data);
        return true;
    }
};

template <class T>
struct JsonTrait<T *> : JsonTraitPointerLike {};

template <class T, class Deleter>
struct JsonTrait<std::unique_ptr<T, Deleter>> : JsonTraitPointerLike {};

template <class T>
struct JsonTrait<std::shared_ptr<T>> : JsonTraitPointerLike {};

template <class T, std::size_t N>
struct JsonTrait<std::array<T, N>> : JsonTraitArrayLike {};

template <class T, class Alloc>
struct JsonTrait<std::vector<T, Alloc>> : JsonTraitArrayLike {};

template <class K, class V, class Cmp, class Alloc>
struct JsonTrait<std::map<K, V, Cmp, Alloc>> : JsonTraitDictLike {};

template <class K, class V, class Hash, class Eq, class Alloc>
struct JsonTrait<std::unordered_map<K, V, Hash, Eq, Alloc>>
    : JsonTraitDictLike {};

template <class Traits, class Alloc>
struct JsonTrait<std::basic_string<char, Traits, Alloc>> : JsonTraitStringLike {
};

template <class Traits>
struct JsonTrait<std::basic_string_view<char, Traits>> : JsonTraitStringLike {};

template <class... Ts>
struct JsonTrait<std::variant<Ts...>> : JsonTraitVariantLike {};

template <class T>
struct JsonTrait<std::optional<T>> : JsonTraitOptionalLike {};

template <>
struct JsonTrait<std::nullptr_t> : JsonTraitNullLike {};

template <>
struct JsonTrait<std::nullopt_t> : JsonTraitNullLike {};

template <>
struct JsonTrait<std::monostate> : JsonTraitNullLike {};

template <>
struct JsonTrait<bool> : JsonTraitBooleanLike {};

/* template <class T> */
/* struct JsonTrait<NoDefault<T>> : JsonTraitWrapperLike {}; */

template <class T>
struct JsonTrait<T, std::enable_if_t<std::is_arithmetic_v<T>>>
    : JsonTraitArithmeticLike {};

template <class Reflector, class T>
inline std::void_t<
    decltype(std::declval<T &>().REFLECT__MEMBERS(std::declval<Reflector &>()))>
reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(reflector);
}

template <class Reflector, class T,
          class = std::void_t<decltype(REFLECT__MEMBERS(
              std::declval<T &>(), std::declval<Reflector &>()))>>
inline void reflect_members(Reflector &reflector, T &value) {
    value.REFLECT__MEMBERS(value, reflector);
}

template <class T>
struct JsonTrait<T, JsonValue> : JsonTraitJsonValueLike {};

template <class T>
struct JsonTrait<
    T, std::void_t<decltype(reflect_members(
           std::declval<ReflectorJsonEncode &>(), std::declval<T &>()))>>
    : JsonTraitObjectLike {};

template <class T>
inline std::string json_encode(T const &value) {
    JsonEncoder encoder;
    encoder.putValue(value);
    return encoder.json;
}

template <class T>
inline bool json_decode(JsonValue &root, T &value, std::error_code &ec) {
    return JsonTrait<T>::getValue(root.inner, value, ec);
}

template <class T>
inline bool json_decode(std::string_view json, T &value, std::error_code &ec) {
    auto root = jsonParse(json, ec);
    if (!root) {
        return false;
    }
    return json_decode(*root, value, ec);
}

template <class T>
inline Expected<T> json_decode(JsonValue &root) {
    T value{};
    std::error_code ec;
    if (!json_decode(root, value, ec)) [[unlikely]] {
        return ec;
    }
    return std::move(value);
}

template <class T>
inline Expected<T> json_decode(std::string_view json) {
    T value{};
    std::error_code ec;
    if (!json_decode(json, value, ec)) [[unlikely]] {
        return ec;
    }
    return std::move(value);
}

} // namespace co_async




namespace co_async {

template <class T>
constexpr T bruteForceByteSwap(T value) {
    if constexpr (sizeof(T) > 1) {
        char* ptr = reinterpret_cast<char*>(&value);
        for (size_t i = 0; i < sizeof(T) / 2; ++i) {
            std::swap(ptr[i], ptr[sizeof(T) - 1 - i]);
        }
    }
    return value;
}

template <class T>
    requires (std::is_trivial_v<T> && !std::is_integral_v<T>)
constexpr T byteswap(T value) {
    return bruteForceByteSwap(value);
}

template <class T>
    requires std::is_integral_v<T>
constexpr T byteswap(T value) {
#if __cpp_lib_byteswap
    return std::byteswap(value);
#elif defined(__GNUC__) && defined(__has_builtin)
#if __has_builtin(__builtin_bswap)
    return __builtin_bswap(value);
#else
    return bruteForceByteSwap(value);
#endif
#else
    return brute_force_byteswap(value);
#endif
}

#if __cpp_lib_endian // C++20 支持的 <bit> 头文件中可以方便地判断本地硬件的大小端
inline constexpr bool is_little_endian = std::endian::native == std::endian::little;
#else
#if _MSC_VER
#include <endian.h>
#if defined(__BYTE_ORDER) && __BYTE_ORDER != 0 && __BYTE_ORDER == __BIG_ENDIAN
inline constexpr bool is_little_endian = false;
#else
inline constexpr bool is_little_endian = true;
#endif
#else
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ != 0
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline constexpr bool is_little_endian = false;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline constexpr bool is_little_endian = true;
#else
inline constexpr bool is_little_endian = true;
#endif
#else
inline constexpr bool is_little_endian = true;
#endif
#endif
#endif

template <class T>
    requires std::is_trivial_v<T>
constexpr T byteswap_if_little(T value) {
    if constexpr (is_little_endian) {
        return byteswap(value);
    } else {
        return value;
    }
}

}




namespace co_async {
template <class T>
struct PImplMethod {};

template <class T>
struct PImplConstruct {
    static std::shared_ptr<T> construct();
};

#define StructPImpl(T) \
    struct T; \
    template <> \
    struct PImplMethod<T>
#define DefinePImpl(T) \
    template <> \
    std::shared_ptr<T> PImplConstruct<T>::construct() { \
        return std::make_shared<T>(); \
    }
#define ForwardPImplMethod(T, func, args, ...) \
    PImplMethod<T>::func args { \
        return pimpl(this).func(__VA_ARGS__); \
    }

template <class T>
struct PImpl : PImplMethod<T> {
    PImpl()
        requires(requires {
            {
                PImplConstruct<T>::construct()
            } -> std::convertible_to<std::shared_ptr<T>>;
        })
        : mImpl(PImplConstruct<T>::construct()) {}

    template <class... Args>
        requires(sizeof...(Args) != 0 &&
                 requires(Args &&...args) {
                     {
                         PImplConstruct<T>::construct(
                             std::forward<Args>(args)...)
                     } -> std::convertible_to<std::shared_ptr<T>>;
                 })
    explicit PImpl(Args &&...args)
        : mImpl(PImplConstruct<T>::construct(std::forward<Args>(args)...)) {}

    PImpl(std::nullptr_t) : mImpl(nullptr) {}

    T &operator*() const noexcept {
        return *mImpl;
    }

    T *operator->() const noexcept {
        return mImpl.get();
    }

    T *impl() const noexcept {
        return mImpl.get();
    }

    T *operator&() const noexcept {
        return mImpl.get();
    }

    operator T &() const noexcept {
        return *mImpl;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(mImpl);
    }

private:
    std::shared_ptr<T> mImpl;
};

template <class T>
T &pimpl(PImplMethod<T> const *that) {
    return **static_cast<PImpl<T> const *>(that);
}
} // namespace co_async





namespace co_async {

template <class F>
struct Finally {
private:
    F func;
    bool enable;

public:
    Finally(std::nullptr_t = nullptr) : enable(false) {}

    Finally(std::convertible_to<F> auto &&func)
        : func(std::forward<decltype(func)>(func)),
          enable(true) {}

    Finally(Finally &&that) : func(std::move(that.func)), enable(that.enable) {
        that.enable = false;
    }

    Finally &operator=(Finally &&that) {
        if (this != &that) {
            if (enable) {
                func();
            }
            func = std::move(that.func);
            enable = that.enable;
            that.enable = false;
        }
        return *this;
    }

    void reset() {
        if (enable) {
            func();
        }
        enable = false;
    }

    void release() {
        enable = false;
    }

    ~Finally() {
        if (enable) {
            func();
        }
    }
};

template <class F>
Finally(F &&) -> Finally<std::decay_t<F>>;

} // namespace co_async





namespace co_async {
template <class T>
struct from_string_t;

template <class Traits, class Alloc>
struct from_string_t<std::basic_string<char, Traits, Alloc>> {
    std::basic_string<char, Traits, Alloc>
    operator()(std::string_view s) const {
        return std::basic_string<char, Traits, Alloc>(s);
    }
};

template <>
struct from_string_t<std::string_view> {
    std::string_view operator()(std::string_view s) const {
        return s;
    }
};

template <std::integral T>
struct from_string_t<T> {
    std::optional<T> operator()(std::string_view s, int base = 10) const {
        T result;
        auto [p, ec] =
            std::from_chars(s.data(), s.data() + s.size(), result, base);
        if (ec != std::errc()) [[unlikely]] {
            return std::nullopt;
        }
        if (p != s.data() + s.size()) [[unlikely]] {
            return std::nullopt;
        }
        return result;
    }
};

template <std::floating_point T>
struct from_string_t<T> {
    std::optional<T>
    operator()(std::string_view s,
               std::chars_format fmt = std::chars_format::general) const {
        T result;
        auto [p, ec] =
            std::from_chars(s.data(), s.data() + s.size(), result, fmt);
        if (ec != std::errc()) [[unlikely]] {
            return std::nullopt;
        }
        if (p != s.data() + s.size()) [[unlikely]] {
            return std::nullopt;
        }
        return result;
    }
};
template <class T>
inline constexpr from_string_t<T> from_string;
template <class T = void>
struct to_string_t;

template <>
struct to_string_t<void> {
    template <class U>
    void operator()(String &result, U &&value) const {
        to_string_t<std::decay_t<U>>()(result, std::forward<U>(value));
    }

    template <class U>
    String operator()(U &&value) const {
        String result;
        operator()(result, std::forward<U>(value));
        return result;
    }
};

template <>
struct to_string_t<String> {
    template <class Traits, class Alloc>
    void operator()(String &result,
                    std::basic_string<char, Traits, Alloc> const &value) const {
        result.assign(value);
    }
};

template <>
struct to_string_t<std::string_view> {
    void operator()(String &result, std::string_view value) const {
        result.assign(value);
    }
};

template <std::integral T>
struct to_string_t<T> {
    void operator()(String &result, T value) const {
        result.resize(std::numeric_limits<T>::digits10 + 2, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(std::size_t(p - result.data()));
    }
};

template <std::floating_point T>
struct to_string_t<T> {
    void operator()(String &result, T value) const {
        result.resize(std::numeric_limits<T>::max_digits10 + 2, '\0');
        auto [p, ec] =
            std::to_chars(result.data(), result.data() + result.size(), value);
        if (ec != std::errc()) [[unlikely]] {
            throw std::system_error(std::make_error_code(ec), "to_chars");
        }
        result.resize(p - result.data());
    }
};

inline constexpr to_string_t<> to_string;

inline String lower_string(std::string_view s) {
    String ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'A' && c <= 'Z') {
            c += 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline String upper_string(std::string_view s) {
    String ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline String trim_string(std::string_view s,
                          std::string_view trims = {" \t\r\n", 4}) {
    auto pos = s.find_first_not_of(trims);
    if (pos == std::string_view::npos) {
        return {};
    }
    auto end = s.find_last_not_of(trims);
    return String(s.substr(pos, end - pos + 1));
}

template <class Delim>
struct SplitString {
    SplitString(std::string_view s, Delim delimiter)
        : s(s),
          delimiter(delimiter) {}

    struct sentinel {
        explicit sentinel() = default;
    };

    struct iterator {
        explicit iterator(std::string_view s, Delim delimiter) noexcept
            : s(s),
              delimiter(delimiter),
              ended(false),
              toBeEnded(false) {
            find_next();
        }

        std::string_view operator*() const noexcept {
            return current;
        }

        std::string_view rest() const noexcept {
            return std::string_view{current.data(), current.size() + s.size()};
        }

        iterator &operator++() {
            find_next();
            return *this;
        }

        bool operator!=(sentinel) const noexcept {
            return !ended;
        }

        bool operator==(sentinel) const noexcept {
            return ended;
        }

        friend bool operator==(sentinel const &lhs,
                               iterator const &rhs) noexcept {
            return rhs == lhs;
        }

        friend bool operator!=(sentinel const &lhs,
                               iterator const &rhs) noexcept {
            return rhs != lhs;
        }

    private:
        void find_next() {
            auto pos = s.find(delimiter);
            if (pos == std::string_view::npos) {
                current = s;
                s = {};
                ended = toBeEnded;
                toBeEnded = true;
            } else {
                current = s.substr(0, pos);
                if constexpr (std::is_same_v<Delim, std::string_view>) {
                    s = s.substr(pos + delimiter.size());
                } else if constexpr (std::is_same_v<Delim, char>) {
                    s = s.substr(pos + 1);
                } else {
                    static_assert(!std::is_void_v<std::void_t<Delim>>);
                }
            }
        }

        std::string_view s;
        Delim delimiter;
        std::string_view current;
        bool ended;
        bool toBeEnded;
    };

    iterator begin() const noexcept {
        return iterator(s, delimiter);
    }

    sentinel end() const noexcept {
        return sentinel();
    }

    std::vector<String> collect() const {
        std::vector<String> result;
        for (auto &&part: *this) {
            result.emplace_back(part);
        }
        return result;
    }

    template <std::size_t N>
        requires(N > 0)
    std::array<String, N> collect() const {
        std::array<String, N> result;
        std::size_t i = 0;
        for (auto it = begin(); it != end(); ++it, ++i) {
            if (i + 1 >= N) {
                result[i] = String(it.rest());
                break;
            }
            result[i] = String(*it);
        }
        return result;
    }

private:
    std::string_view s;
    Delim delimiter;
};

inline SplitString<std::string_view> split_string(std::string_view s,
                                                  std::string_view delimiter) {
    return {s, delimiter};
}

inline SplitString<char> split_string(std::string_view s, char delimiter) {
    return {s, delimiter};
}
} // namespace co_async




namespace co_async {

inline uint32_t getSeedByTime() {
    return static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
}

inline uint32_t getSecureSeed() {
    return std::random_device{}();
}

inline uint32_t wangsHash(uint32_t x) noexcept {
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
}

struct WangsHash {
    using result_type = uint32_t;

    result_type mSeed;

    WangsHash(result_type seed) noexcept : mSeed(seed) {
    }

    void seed(result_type seed) noexcept {
        mSeed = seed;
    }

    result_type operator()() noexcept {
        mSeed = wangsHash(mSeed);
        return mSeed;
        std::mt19937 mt;
        mt.discard(1);
    }

    static result_type max() noexcept {
        return std::numeric_limits<result_type>::max();
    }

    static result_type min() noexcept {
        return std::numeric_limits<result_type>::min();
    }
};

}






#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace co_async {
struct [[nodiscard]] FileHandle {
    FileHandle() noexcept : mFileNo(-1) {}

    explicit FileHandle(int fileNo) noexcept : mFileNo(fileNo) {}

    int fileNo() const noexcept {
        return mFileNo;
    }

    int releaseFile() noexcept {
        int ret = mFileNo;
        mFileNo = -1;
        return ret;
    }

    explicit operator bool() noexcept {
        return mFileNo != -1;
    }

    FileHandle(FileHandle &&that) noexcept : mFileNo(that.releaseFile()) {}

    FileHandle &operator=(FileHandle &&that) noexcept {
        std::swap(mFileNo, that.mFileNo);
        return *this;
    }

    ~FileHandle() {
        if (mFileNo != -1) {
            close(mFileNo);
        }
    }

protected:
    int mFileNo;
};

struct FileStat {
    struct statx *getNativeStatx() {
        return &mStatx;
    }

    std::uint64_t size() const noexcept {
        return mStatx.stx_size;
    }

    std::uint64_t num_blocks() const noexcept {
        return mStatx.stx_blocks;
    }

    mode_t mode() const noexcept {
        return mStatx.stx_mode;
    }

    unsigned int uid() const noexcept {
        return mStatx.stx_uid;
    }

    unsigned int gid() const noexcept {
        return mStatx.stx_gid;
    }

    bool is_directory() const noexcept {
        return (mStatx.stx_mode & S_IFDIR) != 0;
    }

    bool is_regular_file() const noexcept {
        return (mStatx.stx_mode & S_IFREG) != 0;
    }

    bool is_symlink() const noexcept {
        return (mStatx.stx_mode & S_IFLNK) != 0;
    }

    bool is_readable() const noexcept {
        return (mStatx.stx_mode & S_IRUSR) != 0;
    }

    bool is_writable() const noexcept {
        return (mStatx.stx_mode & S_IWUSR) != 0;
    }

    bool is_executable() const noexcept {
        return (mStatx.stx_mode & S_IXUSR) != 0;
    }

    std::chrono::system_clock::time_point accessed_time() const {
        return statTimestampToTimePoint(mStatx.stx_atime);
    }

    std::chrono::system_clock::time_point attribute_changed_time() const {
        return statTimestampToTimePoint(mStatx.stx_ctime);
    }

    std::chrono::system_clock::time_point created_time() const {
        return statTimestampToTimePoint(mStatx.stx_btime);
    }

    std::chrono::system_clock::time_point modified_time() const {
        return statTimestampToTimePoint(mStatx.stx_mtime);
    }

private:
    struct statx mStatx;

    static std::chrono::system_clock::time_point
    statTimestampToTimePoint(struct statx_timestamp const &time) {
        return std::chrono::system_clock::time_point(
            std::chrono::seconds(time.tv_sec) +
            std::chrono::nanoseconds(time.tv_nsec));
    }
};

#if CO_ASYNC_DIRECT
static constexpr size_t kOpenModeDefaultFlags =
    O_LARGEFILE | O_CLOEXEC | O_DIRECT;
#else
static constexpr size_t kOpenModeDefaultFlags = O_LARGEFILE | O_CLOEXEC;
#endif

enum class OpenMode : int {
    Read = O_RDONLY | kOpenModeDefaultFlags,
    Write = O_WRONLY | O_TRUNC | O_CREAT | kOpenModeDefaultFlags,
    ReadWrite = O_RDWR | O_CREAT | kOpenModeDefaultFlags,
    Append = O_WRONLY | O_APPEND | O_CREAT | kOpenModeDefaultFlags,
    Directory = O_RDONLY | O_DIRECTORY | kOpenModeDefaultFlags,
};

inline std::filesystem::path make_path(std::string_view path) {
    return std::filesystem::path(
        reinterpret_cast<char8_t const *>(std::string(path).c_str()));
}

template <std::convertible_to<std::string_view>... Ts>
    requires(sizeof...(Ts) >= 2)
inline std::filesystem::path make_path(Ts &&...chunks) {
    return (make_path(chunks) / ...);
}

inline Task<Expected<FileHandle>> fs_open(std::filesystem::path path, OpenMode mode,
                                          mode_t access = 0644) {
    int oflags = static_cast<int>(mode);
    int fd = co_await expectError(co_await UringOp().prep_openat(
        AT_FDCWD, path.c_str(), oflags, access))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] { return expectError(open(path.c_str(), oflags, access)); })
#endif
        ;
    FileHandle file(fd);
    co_return file;
}

inline Task<Expected<FileHandle>> fs_openat(FileHandle dir,
                                            std::filesystem::path path,
                                            OpenMode mode,
                                            mode_t access = 0644) {
    int oflags = static_cast<int>(mode);
    int fd = co_await expectError(co_await UringOp().prep_openat(
        dir.fileNo(), path.c_str(), oflags, access))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::bad_file_descriptor, [&] { return expectError(openat(dir.fileNo(), path.c_str(), oflags, access)); })
#endif
        ;
    FileHandle file(fd);
    co_return file;
}

inline Task<Expected<>> fs_close(FileHandle file) {
    co_await expectError(co_await UringOp().prep_close(file.fileNo()));
    file.releaseFile();
    co_return {};
}

inline Task<Expected<>> fs_mkdir(std::filesystem::path path, mode_t access = 0755) {
    co_await expectError(
        co_await UringOp().prep_mkdirat(AT_FDCWD, path.c_str(), access));
    co_return {};
}

inline Task<Expected<>> fs_link(std::filesystem::path oldpath, std::filesystem::path newpath) {
    co_await expectError(
        co_await UringOp().prep_linkat(AT_FDCWD, oldpath.c_str(),
                                       AT_FDCWD, newpath.c_str(), 0));
    co_return {};
}

inline Task<Expected<>> fs_symlink(std::filesystem::path target, std::filesystem::path linkpath) {
    co_await expectError(co_await UringOp().prep_symlinkat(
        target.c_str(), AT_FDCWD, linkpath.c_str()));
    co_return {};
}

inline Task<Expected<>> fs_unlink(std::filesystem::path path) {
    co_await expectError(
        co_await UringOp().prep_unlinkat(AT_FDCWD, path.c_str(), 0));
    co_return {};
}

inline Task<Expected<>> fs_rmdir(std::filesystem::path path) {
    co_await expectError(co_await UringOp().prep_unlinkat(
        AT_FDCWD, path.c_str(), AT_REMOVEDIR));
    co_return {};
}

inline Task<Expected<FileStat>>
fs_stat(std::filesystem::path path, unsigned int mask = STATX_BASIC_STATS | STATX_BTIME, int flags = 0) {
    FileStat ret;
    co_await expectError(co_await UringOp().prep_statx(
        AT_FDCWD, path.c_str(), flags, mask, ret.getNativeStatx()))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::bad_file_descriptor, [&] { return expectError(statx(AT_FDCWD, path.c_str(), flags, mask, ret.getNativeStatx())); })
#endif
            ;
    co_return ret;
}

inline Task<Expected<std::size_t>>
fs_read(FileHandle &file, std::span<char> buffer,
        std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(
            co_await UringOp().prep_read(file.fileNo(), buffer, offset))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(read(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pread64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_write(FileHandle &file, std::span<char const> buffer,
         std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(
            co_await UringOp().prep_write(file.fileNo(), buffer, offset))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(write(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pwrite64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_read(FileHandle &file, std::span<char> buffer, CancelToken cancel,
        std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_read(file.fileNo(), buffer, offset)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(read(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pread64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<std::size_t>>
fs_write(FileHandle &file, std::span<char const> buffer, CancelToken cancel,
         std::uint64_t offset = static_cast<std::uint64_t>(-1)) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_write(file.fileNo(), buffer, offset)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
            .or_else(std::errc::invalid_argument,
                      [&] {
                          if (offset == static_cast<std::uint64_t>(-1)) {
                              return expectError(static_cast<int>(write(
                                  file.fileNo(), buffer.data(), buffer.size())));
                          } else {
                              return expectError(static_cast<int>(pwrite64(
                                  file.fileNo(), buffer.data(), buffer.size(),
                                  static_cast<__off64_t>(offset))));
                          }
                      })
#endif
    );
}

inline Task<Expected<>> fs_truncate(FileHandle &file, std::uint64_t size = 0) {
    co_await expectError(co_await UringOp().prep_ftruncate(
        file.fileNo(), static_cast<loff_t>(size)));
    co_return {};
}

inline Task<Expected<std::size_t>>
fs_splice(FileHandle &fileIn, FileHandle &fileOut, std::size_t size,
          std::int64_t offsetIn = -1, std::int64_t offsetOut = -1) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp().prep_splice(
            fileIn.fileNo(), offsetIn, fileOut.fileNo(), offsetOut, size, 0)));
}

inline Task<Expected<std::size_t>> fs_getdents(FileHandle &dirFile,
                                               std::span<char> buffer) {
    int res = static_cast<int>(
        getdents64(dirFile.fileNo(), buffer.data(), buffer.size()));
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_return static_cast<std::size_t>(co_await expectError(res));
}

inline Task<int> fs_nop() {
    co_return co_await UringOp().prep_nop();
}

inline Task<Expected<>> fs_cancel_fd(FileHandle &file) {
    co_await expectError(co_await UringOp().prep_cancel_fd(
        file.fileNo(), IORING_ASYNC_CANCEL_FD | IORING_ASYNC_CANCEL_ALL));
    co_return {};
}
} // namespace co_async






#if CO_ASYNC_ALLOC
struct BytesBuffer {
private:
    struct Deleter {
        std::size_t mSize;
        std::pmr::memory_resource *mResource;

        void operator()(char *p) noexcept {
            mResource->deallocate(p, mSize);
        }
    };

    std::unique_ptr<char[], Deleter> mData;

public:
    BytesBuffer() noexcept = default;

    explicit BytesBuffer(std::size_t size,
                         std::pmr::polymorphic_allocator<> alloc = {})
        : mData(reinterpret_cast<char *>(alloc.resource()->allocate(size)),
                Deleter{size, alloc.resource()}) {}

    void allocate(std::size_t size,
                  std::pmr::polymorphic_allocator<> alloc = {}) {
        std::pmr::memory_resource *resource = alloc.resource();
        mData.reset(reinterpret_cast<char *>(resource->allocate(size)));
        mData.get_deleter() = {size, resource};
    }

    char *data() const noexcept {
        return mData.get();
    }

    std::size_t size() const noexcept {
        return mData.get_deleter().mSize;
    }

    explicit operator bool() const noexcept {
        return (bool)mData;
    }

    char &operator[](std::size_t index) const noexcept {
        return mData[index];
    }

    operator std::span<char>() const noexcept {
        return {data(), size()};
    }
};
#else
// struct BytesBuffer {
// private:
//     std::unique_ptr<char[]> mData;
//     std::size_t mSize = 0;
//
// public:
//     BytesBuffer() noexcept = default;
//
//     explicit BytesBuffer(std::size_t size) :
//     mData(std::make_unique<char[]>(size)), mSize(size) {}
//
//     void allocate(std::size_t size) {
//         mData = std::make_unique<char[]>(size);
//         mSize = size;
//     }
//
//     char *data() const noexcept {
//         return mData.get();
//     }
//
//     std::size_t size() const noexcept {
//         return mSize;
//     }
//
//     explicit operator bool() const noexcept {
//         return static_cast<bool>(mData);
//     }
//
//     char &operator[](std::size_t index) const noexcept {
//         return mData[index];
//     }
//
//     operator std::span<char>() const noexcept {
//         return {data(), size()};
//     }
// };
struct BytesBuffer {
private:
    char *mData;
    std::size_t mSize;

# if __unix__
    void *pageAlignedAlloc(size_t n) {
        return valloc(n);
    }

    void pageAlignedFree(void *p, size_t) {
        free(p);
    }
# elif _WIN32
    __ void *pageAlignedAlloc(size_t n) {
        return _aligned_malloc(n, 4096);
    }

    void pageAlignedFree(void *p, size_t) {
        _aligned_free(p);
    }
# else
    void *pageAlignedAlloc(size_t n) {
        return malloc(n);
    }

    void pageAlignedFree(void *p, size_t) {
        free(p);
    }
# endif

public:
    BytesBuffer() noexcept : mData(nullptr), mSize(0) {}

    explicit BytesBuffer(std::size_t size)
        : mData(static_cast<char *>(pageAlignedAlloc(size))),
          mSize(size) {}

    BytesBuffer(BytesBuffer &&that) noexcept
        : mData(that.mData),
          mSize(that.mSize) {
        that.mData = nullptr;
        that.mSize = 0;
    }

    BytesBuffer &operator=(BytesBuffer &&that) noexcept {
        if (this != &that) {
            pageAlignedFree(mData, mSize);
            mData = that.mData;
            mSize = that.mSize;
            that.mData = nullptr;
            that.mSize = 0;
        }
        return *this;
    }

    ~BytesBuffer() noexcept {
        pageAlignedFree(mData, mSize);
    }

    void allocate(std::size_t size) {
        mData = static_cast<char *>(pageAlignedAlloc(size));
        mSize = size;
    }

    char *data() const noexcept {
        return mData;
    }

    std::size_t size() const noexcept {
        return mSize;
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(mData);
    }

    char &operator[](std::size_t index) const noexcept {
        return mData[index];
    }

    operator std::span<char>() const noexcept {
        return {data(), size()};
    }
};
#endif








namespace co_async {
inline constexpr std::size_t kStreamBufferSize = 8192;

inline std::error_code eofError() {
    static struct final : public std::error_category {
        const char *name() const noexcept override {
            return "eof";
        }
        std::string message(int) const override {
            return "End of file";
        }
    } category;
    return std::error_code(1, category);
}

struct Stream {
    virtual void raw_timeout(std::chrono::steady_clock::duration timeout) {}

    virtual Task<Expected<>> raw_seek(std::uint64_t pos) {
        co_return std::errc::invalid_seek;
    }

    virtual Task<Expected<>> raw_flush() {
        co_return {};
    }

    virtual Task<> raw_close() {
        co_return;
    }

    virtual Task<Expected<std::size_t>> raw_read(std::span<char> buffer) {
        co_return std::errc::not_supported;
    }

    virtual Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) {
        co_return std::errc::not_supported;
    }

    Stream &operator=(Stream &&) = delete;
    virtual ~Stream() = default;
};

struct BorrowedStream {
    BorrowedStream() : mRaw() {}

    explicit BorrowedStream(Stream *raw) : mRaw(raw) {}

    virtual ~BorrowedStream() = default;
    BorrowedStream(BorrowedStream &&) = default;
    BorrowedStream &operator=(BorrowedStream &&) = default;

    Task<Expected<char>> getchar() {
        if (bufempty()) {
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        char c = mInBuffer[mInIndex];
        ++mInIndex;
        co_return c;
    }

    Task<Expected<>> getline(String &s, char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    s.append(mInBuffer.data() + start, i - start);
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            s.append(mInBuffer.data() + start, mInEnd - start);
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropline(char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getline(String &s, std::string_view eol) {
    again:
        co_await co_await getline(s, eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInEnd = mInIndex = 0;
                co_await co_await fillbuf();
            }
            char c = mInBuffer[mInIndex];
            if (eol[i] == c) [[likely]] {
                ++mInIndex;
            } else {
                s.append(eol.data(), i);
                goto again;
            }
        }
        co_return {};
    }

    Task<Expected<>> dropline(std::string_view eol) {
    again:
        co_await co_await dropline(eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInEnd = mInIndex = 0;
                co_await co_await fillbuf();
            }
            char c = mInBuffer[mInIndex];
            if (eol[i] == c) [[likely]] {
                ++mInIndex;
            } else {
                goto again;
            }
        }
        co_return {};
    }

    Task<Expected<String>> getline(char eol) {
        String s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<String>> getline(std::string_view eol) {
        String s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<>> getspan(std::span<char> s) {
        auto p = s.data();
        auto n = s.size();
        std::size_t start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                p = std::copy(mInBuffer.data() + start, mInBuffer.data() + end,
                              p);
                mInIndex = end;
                co_return {};
            }
            p = std::copy(mInBuffer.data() + start, mInBuffer.data() + mInEnd,
                          p);
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> dropn(std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getn(String &s, std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                s.append(mInBuffer.data() + mInIndex, n);
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            s.append(mInBuffer.data() + mInIndex, m);
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<String>> getn(std::size_t n) {
        String s;
        s.reserve(n);
        co_await co_await getn(s, n);
        co_return s;
    }

    Task<Expected<>> dropall() {
        do {
            mInEnd = mInIndex = 0;
        } while (co_await (co_await fillbuf()).transform([] { return true; }).or_else(eofError(), [] { return false; }));
        co_return {};
    }

    Task<Expected<>> getall(String &s) {
        std::size_t start = mInIndex;
        do {
            s.append(mInBuffer.data() + start, mInEnd - start);
            start = 0;
            mInEnd = mInIndex = 0;
        } while (co_await (co_await fillbuf()).transform([] { return true; }).or_else(eofError(), [] { return false; }));
        co_return {};
    }

    Task<Expected<String>> getall() {
        String s;
        co_await co_await getall(s);
        co_return s;
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<>> getstruct(T &ret) {
        return getspan(
            std::span<char>(reinterpret_cast<char *>(&ret), sizeof(T)));
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<T>> getstruct() {
        T ret;
        co_await co_await getstruct(ret);
        co_return ret;
    }

    std::span<char const> peekbuf() const noexcept {
        return {mInBuffer.data() + mInIndex, mInEnd - mInIndex};
    }

    void seenbuf(std::size_t n) noexcept {
        mInIndex += n;
    }

    Task<Expected<String>> getchunk() noexcept {
        if (bufempty()) {
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        auto buf = peekbuf();
        String ret(buf.data(), buf.size());
        seenbuf(buf.size());
        co_return std::move(ret);
    }

    std::size_t tryread(std::span<char> buffer) {
        auto peekBuf = peekbuf();
        std::size_t n = std::min(buffer.size(), peekBuf.size());
        std::memcpy(buffer.data(), peekBuf.data(), n);
        seenbuf(n);
        return n;
    }

    Task<Expected<char>> peekchar() {
        if (bufempty()) {
            mInEnd = mInIndex = 0;
            co_await co_await fillbuf();
        }
        co_return mInBuffer[mInIndex];
    }

    Task<Expected<>> peekn(String &s, std::size_t n) {
        if (mInBuffer.size() - mInIndex < n) {
            if (mInBuffer.size() < n) [[unlikely]] {
                co_return std::errc::value_too_large;
            }
            std::memmove(mInBuffer.data(), mInBuffer.data() + mInIndex,
                         mInEnd - mInIndex);
            mInEnd -= mInIndex;
            mInIndex = 0;
        }
        while (mInEnd - mInIndex < n) {
            co_await co_await fillbuf();
        }
        s.append(mInBuffer.data() + mInIndex, n);
        co_return {};
    }

    Task<Expected<String>> peekn(std::size_t n) {
        String s;
        co_await co_await peekn(s, n);
        co_return s;
    }

    void allocinbuf(std::size_t size) {
        if (!mInBuffer) [[likely]] {
            mInBuffer.allocate(size);
            mInIndex = 0;
            mInEnd = 0;
        }
    }

    Task<Expected<>> fillbuf() {
        if (!mInBuffer) {
            allocinbuf(kStreamBufferSize);
        }
        // #if CO_ASYNC_DEBUG
        //         if (!bufempty()) [[unlikely]] {
        //             throw std::logic_error("buf must be empty before
        //             fillbuf");
        //         }
        // #endif
        auto n = co_await co_await mRaw->raw_read(std::span(
            mInBuffer.data() + mInIndex, mInBuffer.size() - mInIndex));
        // auto n = co_await co_await mRaw->raw_read(mInBuffer);
        if (n == 0) [[unlikely]] {
            co_return eofError();
        }
        mInEnd = mInIndex + n;
        co_return {};
    }

    bool bufempty() const noexcept {
        return mInIndex == mInEnd;
    }

    Task<Expected<>> putchar(char c) {
        if (buffull()) {
            co_await co_await flush();
        }
        mOutBuffer[mOutIndex] = c;
        ++mOutIndex;
        co_return {};
    }

    Task<Expected<>> putspan(std::span<char const> s) {
        auto p = s.data();
        auto const pe = s.data() + s.size();
    again:
        if (std::size_t(pe - p) <= mOutBuffer.size() - mOutIndex) {
            auto b = mOutBuffer.data() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
        } else {
            auto b = mOutBuffer.data() + mOutIndex;
            auto const be = mOutBuffer.data() + mOutBuffer.size();
            mOutIndex = mOutBuffer.size();
            while (b < be) {
                *b++ = *p++;
            }
            co_await co_await flush();
            mOutIndex = 0;
            goto again;
        }
        co_return {};
    }

    std::size_t trywrite(std::span<char const> s) {
        if (!mOutBuffer) {
            allocoutbuf(kStreamBufferSize);
        }
        auto p = s.data();
        auto const pe = s.data() + s.size();
        auto nMax = mOutBuffer.size() - mOutIndex;
        auto n = std::size_t(pe - p);
        if (n <= nMax) {
            auto b = mOutBuffer.data() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
            return n;
        } else {
            auto b = mOutBuffer.data() + mOutIndex;
            auto const be = mOutBuffer.data() + mOutBuffer.size();
            mOutIndex = mOutBuffer.size();
            while (b < be) {
                *b++ = *p++;
            }
            return nMax;
        }
    }

    Task<Expected<>> puts(std::string_view s) {
        return putspan(std::span<char const>(s.data(), s.size()));
    }

    template <class T>
    Task<Expected<>> putstruct(T const &s) {
        return putspan(std::span<char const>(
            reinterpret_cast<char const *>(std::addressof(s)), sizeof(T)));
    }

    Task<Expected<>> putchunk(std::string_view s) {
        co_await co_await puts(s);
        co_return co_await flush();
    }

    Task<Expected<>> putline(std::string_view s) {
        co_await co_await puts(s);
        co_await co_await putchar('\n');
        co_return co_await flush();
    }

    void allocoutbuf(std::size_t size) {
        if (!mOutBuffer) [[likely]] {
            mOutBuffer.allocate(size);
            mOutIndex = 0;
        }
    }

    Task<Expected<>> flush() {
        if (!mOutBuffer) {
            allocoutbuf(kStreamBufferSize);
            co_return {};
        }
        if (mOutIndex) [[likely]] {
            auto buf = std::span(mOutBuffer.data(), mOutIndex);
            auto len = co_await mRaw->raw_write(buf);
            while (len.has_value() && *len > 0 && *len != buf.size()) {
                buf = buf.subspan(*len);
                len = co_await mRaw->raw_write(buf);
            }
            if (len.has_error()) [[unlikely]] {
#if CO_ASYNC_DEBUG
                co_return {len.error(), len.mErrorLocation};
#else
                co_return len.error();
#endif
            }
            if (*len == 0) [[unlikely]] {
                co_return eofError();
            }
            mOutIndex = 0;
            co_await co_await mRaw->raw_flush();
        }
        co_return {};
    }

    bool buffull() const noexcept {
        return mOutIndex == mOutBuffer.size();
    }

    Stream &raw() const noexcept {
        return *mRaw;
    }

    template <std::derived_from<Stream> Derived>
    Derived &raw() const {
        return dynamic_cast<Derived &>(*mRaw);
    }

    Task<> close() {
#if CO_ASYNC_DEBUG
        if (mOutIndex) [[unlikely]] {
            std::cerr << "WARNING: stream closed with buffer not flushed\n";
        }
#endif
        return mRaw->raw_close();
    }

    Task<Expected<std::size_t>> read(std::span<char> buffer) {
        if (!bufempty()) {
            auto n = std::min(mInEnd - mInIndex, buffer.size());
            std::memcpy(buffer.data(), mInBuffer.data() + mInIndex, n);
            mInIndex += n;
            co_return n;
        }
        co_return co_await mRaw->raw_read(buffer);
    }

    Task<Expected<std::size_t>> read(void *buffer, std::size_t len) {
        return read(std::span<char>(static_cast<char *>(buffer), len));
    }

    std::size_t tryread(void *buffer, std::size_t len) {
        return tryread(std::span<char>(static_cast<char *>(buffer), len));
    }

    Task<Expected<std::size_t>> write(std::span<char const> buffer) {
        if (!buffull()) {
            auto n = std::min(mInBuffer.size() - mInIndex, buffer.size());
            co_await co_await putspan(buffer.subspan(0, n));
            co_return n;
        }
        co_return co_await mRaw->raw_write(buffer);
    }

    Task<Expected<std::size_t>> write(void const *buffer, std::size_t len) {
        return write(
            std::span<char const>(static_cast<char const *>(buffer), len));
    }

    Task<Expected<>> putspan(void const *buffer, std::size_t len) {
        return putspan(
            std::span<char const>(static_cast<char const *>(buffer), len));
    }

    std::size_t trywrite(void const *buffer, std::size_t len) {
        return trywrite(
            std::span<char const>(static_cast<char const *>(buffer), len));
    }

    void timeout(std::chrono::steady_clock::duration timeout) {
        mRaw->raw_timeout(timeout);
    }

    Task<Expected<>> seek(std::uint64_t pos) {
        co_await co_await mRaw->raw_seek(pos);
        mInIndex = 0;
        mInEnd = 0;
        mOutIndex = 0;
        co_return {};
    }

private:
    BytesBuffer mInBuffer;
    std::size_t mInIndex = 0;
    std::size_t mInEnd = 0;
    BytesBuffer mOutBuffer;
    std::size_t mOutIndex = 0;
    Stream *mRaw;
};

struct OwningStream : BorrowedStream {
    explicit OwningStream() : BorrowedStream(), mRawUnique() {}

    explicit OwningStream(std::unique_ptr<Stream> raw)
        : BorrowedStream(raw.get()),
          mRawUnique(std::move(raw)) {}

    std::unique_ptr<Stream> releaseraw() noexcept {
        return std::move(mRawUnique);
    }

private:
    std::unique_ptr<Stream> mRawUnique;
};

template <std::derived_from<Stream> Stream, class... Args>
OwningStream make_stream(Args &&...args) {
    return OwningStream(std::make_unique<Stream>(std::forward<Args>(args)...));
}
} // namespace co_async








namespace co_async {
Task<Expected<OwningStream>> file_open(std::filesystem::path path,
                                       OpenMode mode);
OwningStream file_from_handle(FileHandle handle);
Task<Expected<String>> file_read(std::filesystem::path path);
Task<Expected<>> file_write(std::filesystem::path path,
                            std::string_view content);
Task<Expected<>> file_append(std::filesystem::path path,
                             std::string_view content);
} // namespace co_async


#ifdef __linux__
# include <unistd.h>
#endif


#ifdef __linux__





namespace co_async {
struct FSPipeHandlePair {
    FileHandle mReader;
    FileHandle mWriter;

    std::array<OwningStream, 2> stream() {
        return {file_from_handle(reader()), file_from_handle(writer())};
    }

    FileHandle reader() {
# if CO_ASYNC_DEBUG
        if (!mReader) [[unlikely]] {
            throw std::logic_error(
                "PipeHandlePair::reader() can only be called once");
        }
# endif
        return std::move(mReader);
    }

    FileHandle writer() {
# if CO_ASYNC_DEBUG
        if (!mWriter) [[unlikely]] {
            throw std::logic_error(
                "PipeHandlePair::writer() can only be called once");
        }
# endif
        return std::move(mWriter);
    }
};

inline Task<Expected<FSPipeHandlePair>> fs_pipe() {
    int p[2];
    int res = pipe2(p, 0);
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_await expectError(res);
    co_return FSPipeHandlePair{FileHandle(p[0]), FileHandle(p[1])};
}

inline Task<Expected<>> send_file(FileHandle &sock, FileHandle &&file) {
    auto [readPipe, writePipe] = co_await co_await fs_pipe();
    while (auto n = co_await co_await fs_splice(file, writePipe, 65536)) {
        std::size_t m;
        while ((m = co_await co_await fs_splice(readPipe, sock, n)) < n)
            [[unlikely]] {
            n -= m;
        }
    }
    co_await co_await fs_close(std::move(file));
    co_await co_await fs_close(std::move(readPipe));
    co_await co_await fs_close(std::move(writePipe));
    co_return {};
}

inline Task<Expected<>> recv_file(FileHandle &sock, FileHandle &&file) {
    auto [readPipe, writePipe] = co_await co_await fs_pipe();
    while (auto n = co_await co_await fs_splice(sock, writePipe, 65536)) {
        std::size_t m;
        while ((m = co_await co_await fs_splice(readPipe, file, n)) < n)
            [[unlikely]] {
            n -= m;
        }
    }
    co_await co_await fs_close(std::move(file));
    co_await co_await fs_close(std::move(readPipe));
    co_await co_await fs_close(std::move(writePipe));
    co_return {};
}
} // namespace co_async
#endif











#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
using Pid = pid_t;

struct WaitProcessResult {
    Pid pid;
    int status;

    enum ExitType : int {
        Continued = CLD_CONTINUED,
        Stopped = CLD_STOPPED,
        Trapped = CLD_TRAPPED,
        Dumped = CLD_DUMPED,
        Killed = CLD_KILLED,
        Exited = CLD_EXITED,
        Timeout = -1,
    } exitType;
};

inline Task<Expected<>> kill_process(Pid pid, int sig = SIGKILL) {
    co_await expectError(kill(pid, sig));
    co_return {};
}

inline Task<Expected<WaitProcessResult>> wait_process(Pid pid,
                                                      int options = WEXITED) {
    siginfo_t info{};
    co_await expectError(co_await UringOp().prep_waitid(
        P_PID, static_cast<id_t>(pid), &info, options, 0));
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = static_cast<WaitProcessResult::ExitType>(info.si_code),
    };
}

inline Task<Expected<WaitProcessResult>>
wait_process(Pid pid, std::chrono::steady_clock::duration timeout,
             int options = WEXITED) {
    siginfo_t info{};
    auto ts = durationToKernelTimespec(timeout);
    auto ret = expectError(co_await UringOp::link_ops(
        UringOp().prep_waitid(P_PID, static_cast<id_t>(pid), &info, options, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    if (ret == std::make_error_code(std::errc::operation_canceled)) {
        co_return std::errc::stream_timeout;
    }
    co_await std::move(ret);
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = static_cast<WaitProcessResult::ExitType>(info.si_code),
    };
}

struct ProcessBuilder {
    ProcessBuilder() {
        mAbsolutePath = false;
        mEnvInherited = false;
        throwingErrorErrno(posix_spawnattr_init(&mAttr));
        throwingErrorErrno(posix_spawn_file_actions_init(&mFileActions));
    }

    ProcessBuilder(ProcessBuilder &&) = delete;

    ~ProcessBuilder() {
        posix_spawnattr_destroy(&mAttr);
        posix_spawn_file_actions_destroy(&mFileActions);
    }

    ProcessBuilder &chdir(std::filesystem::path path) {
        throwingErrorErrno(
            posix_spawn_file_actions_addchdir_np(&mFileActions, path.c_str()));
        return *this;
    }

    ProcessBuilder &open(int fd, FileHandle &&file) {
        open(fd, file.fileNo());
        mFileStore.push_back(std::move(file));
        return *this;
    }

    ProcessBuilder &open(int fd, FileHandle const &file) {
        return open(fd, file.fileNo());
    }

    ProcessBuilder &open(int fd, int ourFd) {
        if (fd != ourFd) {
            throwingErrorErrno(
                posix_spawn_file_actions_adddup2(&mFileActions, ourFd, fd));
        }
        return *this;
    }

    ProcessBuilder &pipe_out(int fd, OwningStream &stream) {
        int p[2];
        throwingErrorErrno(pipe2(p, 0));
        open(fd, FileHandle(p[1]));
        stream = file_from_handle(FileHandle(p[0]));
        close(p[0]);
        close(p[1]);
        return *this;
    }

    ProcessBuilder &pipe_in(int fd, OwningStream &stream) {
        int p[2];
        throwingErrorErrno(pipe2(p, 0));
        open(fd, FileHandle(p[0]));
        stream = file_from_handle(FileHandle(p[1]));
        close(p[0]);
        close(p[1]);
        return *this;
    }

    ProcessBuilder &close(int fd) {
        throwingErrorErrno(
            posix_spawn_file_actions_addclose(&mFileActions, fd));
        return *this;
    }

    ProcessBuilder &path(std::filesystem::path path, bool isAbsolute = false) {
        mPath = path.string();
        mAbsolutePath = isAbsolute;
        return *this;
    }

    ProcessBuilder &arg(std::string_view arg) {
        mArgvStore.emplace_back(arg);
        return *this;
    }

    ProcessBuilder &inherit_env(bool inherit = true) {
        if (inherit) {
            for (char *const *e = environ; *e; ++e) {
                mEnvpStore.emplace_back(*e);
            }
        }
        mEnvInherited = true;
        return *this;
    }

    ProcessBuilder &env(std::string_view key, std::string_view val) {
        if (!mEnvInherited) {
            inherit_env();
        }
        std::string env(key);
        env.push_back('=');
        env.append(val);
        mEnvpStore.emplace_back(std::move(env));
        return *this;
    }

    Task<Expected<Pid>> spawn() {
        Pid pid;
        std::vector<char *> argv;
        std::vector<char *> envp;
        if (!mArgvStore.empty()) {
            argv.reserve(mArgvStore.size() + 2);
            argv.push_back(mPath.data());
            for (auto &s: mArgvStore) {
                argv.push_back(s.data());
            }
            argv.push_back(nullptr);
        } else {
            argv = {mPath.data(), nullptr};
        }
        if (!mEnvpStore.empty()) {
            envp.reserve(mEnvpStore.size() + 1);
            for (auto &s: mEnvpStore) {
                envp.push_back(s.data());
            }
            envp.push_back(nullptr);
        }
        int status = (mAbsolutePath ? posix_spawn : posix_spawnp)(
            &pid, mPath.c_str(), &mFileActions, &mAttr, argv.data(),
            mEnvpStore.empty() ? environ : envp.data());
        if (status != 0) [[unlikely]] {
            co_return std::errc(errno);
        }
        mPath.clear();
        mArgvStore.clear();
        mEnvpStore.clear();
        mFileStore.clear();
        co_return pid;
    }

private:
    posix_spawn_file_actions_t mFileActions;
    posix_spawnattr_t mAttr;
    bool mAbsolutePath;
    bool mEnvInherited;
    std::string mPath;
    std::vector<std::string> mArgvStore;
    std::vector<std::string> mEnvpStore;
    std::vector<FileHandle> mFileStore;
};
} // namespace co_async









#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace co_async {
struct FileWatch {
    enum FileEvent : std::uint32_t {
        OnAccessed = IN_ACCESS,
        OnOpened = IN_OPEN,
        OnAttributeChanged = IN_ATTRIB,
        OnModified = IN_MODIFY,
        OnDeleted = IN_DELETE_SELF,
        OnMoved = IN_MOVE_SELF,
        OnChildCreated = IN_CREATE,
        OnChildDeleted = IN_DELETE,
        OnChildMovedAway = IN_MOVED_FROM,
        OnChildMovedInto = IN_MOVED_TO,
        OnWriteFinished = IN_CLOSE_WRITE,
        OnReadFinished = IN_CLOSE_NOWRITE,
    };

    FileWatch()
        : mFile(throwingErrorErrno(inotify_init1(0))),
          mStream(file_from_handle(FileHandle(mFile))) {}

    int add(std::filesystem::path const &path, FileEvent event) {
        int wd =
            throwingErrorErrno(inotify_add_watch(mFile, path.c_str(), event));
        mWatches.emplace(wd, path);
        return wd;
    }

    FileWatch &watch(std::filesystem::path const &path, FileEvent event,
                     bool recursive = false) {
        add(path, event);
        if (recursive && std::filesystem::is_directory(path)) {
            for (auto const &entry:
                 std::filesystem::recursive_directory_iterator(path)) {
                add(entry.path(), event);
            }
        }
        return *this;
    }

    FileWatch &remove(int wd) {
        throwingErrorErrno(inotify_rm_watch(mFile, wd));
        mWatches.erase(wd);
        return *this;
    }

    struct WaitFileResult {
        std::filesystem::path path;
        FileEvent event;
    };

    Task<Expected<WaitFileResult>> wait() {
        if (!co_await mStream.getstruct(*mEventBuffer)) [[unlikely]] {
            throw std::runtime_error("EOF while reading struct");
        }
        String name;
        name.reserve(mEventBuffer->len);
        co_await co_await mStream.getn(name, mEventBuffer->len);
        name = name.c_str();
        auto path = mWatches.at(mEventBuffer->wd);
        if (!name.empty()) {
            path /= make_path(name);
        }
        co_return WaitFileResult{
            .path = std::move(path),
            .event = static_cast<FileEvent>(mEventBuffer->mask),
        };
    }

private:
    int mFile;
    OwningStream mStream;
    std::unique_ptr<struct inotify_event> mEventBuffer =
        std::make_unique<struct inotify_event>();
    std::map<int, std::filesystem::path> mWatches;
};
} // namespace co_async

// 
// 
// 
// 
// 
// #include <signal.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
//
// namespace co_async {
// struct SignalingContextMT {
//     static void startMain(std::stop_token stop) {
//         while (!stop.stop_requested()) [[likely]] {
//             sigset_t s;
//             sigemptyset(&s);
//             std::unique_lock lock(instance->mMutex);
//             for (auto [signo, waiters]: instance->mWaitingSignals) {
//                 sigaddset(&s, signo);
//             }
//             lock.unlock();
//             int signo;
//             throwingError(-sigwait(&s, &signo));
//             lock.lock();
//             std::deque<std::coroutine_handle<>> waiters;
//             waiters.swap(instance->mWaitingSignals.at(signo));
//             lock.unlock();
//             for (auto coroutine: waiters) {
//                 IOContextMT::spawn(coroutine);
//             }
//         }
//     }
//
//     struct SignalAwaiter {
//         bool await_ready() const noexcept {
//             return false;
//         }
//
//         void await_suspend(std::coroutine_handle<> coroutine) const {
//             std::lock_guard lock(instance->mMutex);
//             instance->mWaitingSignals[mSigno].push_back(coroutine);
//         }
//
//         void await_resume() const noexcept {}
//
//         int mSigno;
//     };
//
//     static SignalAwaiter waitSignal(int signo) {
//         return SignalAwaiter(signo);
//     }
//
//     static void start() {
//         instance->mWorker =
//             std::jthread([](std::stop_token stop) { startMain(stop); });
//     }
//
//     static inline SignalingContextMT *instance;
//
//     SignalingContextMT() {
//         if (instance) {
//             throw std::logic_error(
//                 "each process may contain only one SignalingContextMT");
//         }
//         instance = this;
//         start();
//     }
//
//     SignalingContextMT(SignalingContextMT &&) = delete;
//
//     ~SignalingContextMT() {
//         instance = nullptr;
//     }
//
// private:
//     std::map<int, std::deque<std::coroutine_handle<>>> mWaitingSignals;
//     std::mutex mMutex;
//     std::jthread mWorker;
// };
// } // namespace co_async



#include <arpa/inet.h>







#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace co_async {
std::error_category const &getAddrInfoCategory();

// struct IpAddress {
//     explicit IpAddress(struct in_addr const &addr) noexcept : mAddr(addr) {}
//
//     explicit IpAddress(struct in6_addr const &addr6) noexcept : mAddr(addr6)
//     {}
//
//     static Expected<IpAddress> fromString(char const *host);
//
//     String toString() const;
//
//     auto repr() const {
//         return toString();
//     }
//
//     std::variant<struct in_addr, struct in6_addr> mAddr;
// };

struct SocketAddress {
    SocketAddress() = default;

    explicit SocketAddress(struct sockaddr const *addr, socklen_t addrLen,
                           sa_family_t family, int sockType, int protocol);

    struct sockaddr_storage mAddr;
    socklen_t mAddrLen;
    int mSockType;
    int mProtocol;

    sa_family_t family() const noexcept {
        return mAddr.ss_family;
    }

    int socktype() const noexcept {
        return mSockType;
    }

    int protocol() const noexcept {
        return mProtocol;
    }

    std::string host() const;

    int port() const;

    void trySetPort(int port);

    String toString() const;

    auto repr() const {
        return toString();
    }

private:
    void initFromHostPort(struct in_addr const &host, int port);
    void initFromHostPort(struct in6_addr const &host, int port);
};

struct AddressResolver {
private:
    std::string m_host;
    int m_port = -1;
    std::string m_service;
    struct addrinfo m_hints = {};

public:
    AddressResolver &host(std::string_view host) {
        if (auto i = host.find("://"); i != host.npos) {
            if (auto service = host.substr(0, i); !service.empty()) {
                m_service = service;
            }
            host.remove_prefix(i + 3);
        }
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1)))
                [[likely]] {
                m_port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        m_host = host;
        return *this;
    }

    AddressResolver &port(int port) {
        m_port = port;
        return *this;
    }

    AddressResolver &service(std::string_view service) {
        m_service = service;
        return *this;
    }

    AddressResolver &family(int family) {
        m_hints.ai_family = family;
        return *this;
    }

    AddressResolver &socktype(int socktype) {
        m_hints.ai_socktype = socktype;
        return *this;
    }

    struct ResolveResult {
        std::vector<SocketAddress> addrs;
        std::string service;
    };

    Expected<ResolveResult> resolve_all();
    Expected<SocketAddress> resolve_one();
    Expected<SocketAddress> resolve_one(std::string &service);
};

struct [[nodiscard]] SocketHandle : FileHandle {
    using FileHandle::FileHandle;
};

struct [[nodiscard]] SocketListener : SocketHandle {
    using SocketHandle::SocketHandle;
};

SocketAddress get_socket_address(SocketHandle &sock);
SocketAddress get_socket_peer_address(SocketHandle &sock);

template <class T>
Expected<T> socketGetOption(SocketHandle &sock, int level, int optId) {
    T val;
    socklen_t len = sizeof(val);
    if (auto e =
            expectError(getsockopt(sock.fileNo(), level, optId, &val, &len))) {
        return e.error();
    }
    return val;
}

template <class T>
Expected<> socketSetOption(SocketHandle &sock, int level, int opt,
                           T const &optVal) {
    return expectError(
        setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

Task<Expected<SocketHandle>> createSocket(int family, int type, int protocol);
Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr);
Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout);

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel);
Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog = SOMAXCONN);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr);
Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel);
Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf);
Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf, CancelToken cancel);
Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf,
                                            CancelToken cancel);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout, CancelToken cancel);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout, CancelToken cancel);
Task<Expected<>> socket_shutdown(SocketHandle &sock, int how = SHUT_RDWR);
} // namespace co_async







namespace co_async {
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest);
Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest);
} // namespace co_async








namespace co_async {
std::error_category const &bearSSLCategory();

StructPImpl(SSLClientTrustAnchor) {
    Expected<> add(std::string_view content);
};

StructPImpl(SSLServerPrivateKey){};

StructPImpl(SSLServerCertificate) {
    Expected<> add(std::string_view content);
};

StructPImpl(SSLServerSessionCache){};
Task<Expected<OwningStream>>
ssl_connect(char const *host, int port, SSLClientTrustAnchor const &ta,
            std::span<char const *const> protocols, std::string_view proxy,
            std::chrono::steady_clock::duration timeout);
OwningStream ssl_accept(SocketHandle file, SSLServerCertificate const &cert,
                        SSLServerPrivateKey const &pkey,
                        std::span<char const *const> protocols,
                        SSLServerSessionCache *cache = nullptr);
} // namespace co_async






namespace co_async {
struct CachedStream : Stream {
    explicit CachedStream(BorrowedStream &stream) : mStream(stream) {}

    BorrowedStream &base() const noexcept {
        return mStream;
    }

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        if (mPos != mCache.size()) {
            auto n = std::min(mCache.size() - mPos, buffer.size());
            std::memcpy(buffer.data(), mCache.data() + mPos, n);
            mPos += n;
            co_return n;
        }
        auto n = co_await co_await mStream.read(buffer);
        mCache.append(buffer.data(), n);
        co_return n;
    }

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        mStream.timeout(timeout);
    }

    Task<> raw_close() override {
        return mStream.close();
    }

    Task<Expected<>> raw_flush() override {
        return mStream.flush();
    }

    Task<Expected<>> raw_seek(std::uint64_t pos) override {
        if (pos <= mCache.size()) {
            mPos = pos;
            co_return {};
        } else {
            co_return std::errc::invalid_seek;
        }
    }

private:
    BorrowedStream &mStream;
    std::string mCache;
    std::size_t mPos = 0;
};
} // namespace co_async








namespace co_async {
struct IStringStream : Stream {
    IStringStream() noexcept : mPosition(0) {}

    IStringStream(std::string_view strView)
        : mStringView(strView),
          mPosition(0) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        std::size_t size =
            std::min(buffer.size(), mStringView.size() - mPosition);
        std::copy_n(mStringView.begin() + mPosition, size, buffer.begin());
        mPosition += size;
        co_return size;
    }

    std::string_view str() const noexcept {
        return mStringView;
    }

    std::string_view unread_str() const noexcept {
        return mStringView.substr(mPosition);
    }

private:
    std::string_view mStringView;
    std::size_t mPosition;
};

struct OStringStream : Stream {
    OStringStream(String &output) noexcept : mOutput(output) {}

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        mOutput.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }

    String &str() const noexcept {
        return mOutput;
    }

    String release() noexcept {
        return std::move(mOutput);
    }

private:
    String &mOutput;
};
} // namespace co_async






namespace co_async {

std::array<OwningStream, 2> pipe_stream();
Task<Expected<>> pipe_forward(BorrowedStream &in, BorrowedStream &out);

template <class Func, class... Args>
    requires std::invocable<Func, Args..., OwningStream &>
inline Task<Expected<>> pipe_bind(OwningStream w, Func &&func, Args &&...args) {
    return co_bind(
        [func = std::forward<decltype(func)>(func),
         w = std::move(w)](auto &&...args) mutable -> Task<Expected<>> {
            auto e1 =
                co_await std::invoke(std::forward<decltype(func)>(func),
                                     std::forward<decltype(args)>(args)..., w);
            auto e2 = co_await w.flush();
            co_await w.close();
            co_await e1;
            co_await e2;
            co_return {};
        },
        std::forward<decltype(args)>(args)...);
}
} // namespace co_async





namespace co_async {
OwningStream &stdio();
OwningStream &raw_stdio();
} // namespace co_async







#include <dirent.h>

namespace co_async {
struct DirectoryWalker {
    explicit DirectoryWalker(FileHandle file);
    DirectoryWalker(DirectoryWalker &&) = default;
    DirectoryWalker &operator=(DirectoryWalker &&) = default;
    ~DirectoryWalker();
    Task<Expected<String>> next();

private:
    OwningStream mStream;
};

Task<Expected<DirectoryWalker>> dir_open(std::filesystem::path path);
} // namespace co_async







namespace co_async {
Task<Expected<SocketHandle>>
socket_proxy_connect(char const *host, int port, std::string_view proxy,
                     std::chrono::steady_clock::duration timeout);
} // namespace co_async









namespace co_async {
struct SocketStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        auto ret =
            co_await socket_read(mFile, buffer, mTimeout, co_await co_cancel);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return std::errc::stream_timeout;
        }
        co_return ret;
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        auto ret =
            co_await socket_write(mFile, buffer, mTimeout, co_await co_cancel);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return std::errc::stream_timeout;
        }
        co_return ret;
    }

    SocketHandle release() noexcept {
        return std::move(mFile);
    }

    SocketHandle &get() noexcept {
        return mFile;
    }

    explicit SocketStream(SocketHandle file) : mFile(std::move(file)) {}

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        mTimeout = timeout;
    }

private:
    SocketHandle mFile;
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(30);
};

inline Task<Expected<OwningStream>>
tcp_connect(char const *host, int port, std::string_view proxy,
            std::chrono::steady_clock::duration timeout) {
    auto handle =
        co_await co_await socket_proxy_connect(host, port, proxy, timeout);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(timeout);
    co_return sock;
}

inline Task<Expected<OwningStream>> tcp_accept(SocketListener &listener) {
    auto handle = co_await co_await listener_accept(listener);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    co_return sock;
}
} // namespace co_async






namespace co_async {
struct URIParams : SimpleMap<String, String> {
    using SimpleMap<String, String>::SimpleMap;
};

struct URI {
    String path;
    URIParams params;

public:
    static void url_decode(String &r, std::string_view s);
    static String url_decode(std::string_view s);
    static void url_encode(String &r, std::string_view s);
    static String url_encode(std::string_view s);
    static void url_encode_path(String &r, std::string_view s);
    static String url_encode_path(std::string_view s);
    static URI parse(std::string_view uri);
    void dump(String &r) const;
    String dump() const;

    String repr() const {
        return dump();
    }
};
} // namespace co_async






namespace co_async {
String timePointToHTTPDate(std::chrono::system_clock::time_point tp);
Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(String const &date);
String httpDateNow();
std::string_view getHTTPStatusName(int status);
String guessContentTypeByExtension(
    std::string_view ext, char const *defaultType = "text/plain;charset=utf-8");
String capitalizeHTTPHeader(std::string_view key);
} // namespace co_async










namespace co_async {
struct HTTPHeaders : SimpleMap<String, String> {
    using SimpleMap<String, String>::SimpleMap;
    // NOTE: user-specified http headers must not contain the following keys:
    // - connection
    // - acecpt-encoding
    // - transfer-encoding
    // - content-encoding
    // - content-length
    // they should only be used internally in our http protocol implementation
};
enum class HTTPContentEncoding {
    Identity = 0,
    Gzip,
    Deflate,
};

struct HTTPRequest {
    String method{"GET", 3};
    URI uri{String{"/", 1}, {}};
    HTTPHeaders headers{};

    auto repr() const {
        return std::make_tuple(method, uri, headers);
    }
};

struct HTTPResponse {
    int status{0};
    HTTPHeaders headers{};

    auto repr() const {
        return std::make_tuple(status, headers);
    }
};

struct HTTPProtocol {
public:
    OwningStream sock;

    explicit HTTPProtocol(OwningStream sock) : sock(std::move(sock)) {}

    HTTPProtocol(HTTPProtocol &&) = delete;
    virtual ~HTTPProtocol() = default;
    virtual void initServerState() = 0;
    virtual void initClientState() = 0;
    virtual Task<Expected<>> writeBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<>> readBodyStream(BorrowedStream &body) = 0;
    virtual Task<Expected<>> writeBody(std::string_view body) = 0;
    virtual Task<Expected<>> readBody(String &body) = 0;
    virtual Task<Expected<>> writeRequest(HTTPRequest const &req) = 0;
    virtual Task<Expected<>> readRequest(HTTPRequest &req) = 0;
    virtual Task<Expected<>> writeResponse(HTTPResponse const &res) = 0;
    virtual Task<Expected<>> readResponse(HTTPResponse &res) = 0;
};

struct HTTPProtocolVersion11 : HTTPProtocol {
    using HTTPProtocol::HTTPProtocol;

protected:
    HTTPContentEncoding mContentEncoding;
    String mAcceptEncoding;
    std::optional<std::size_t> mContentLength;
    HTTPContentEncoding httpContentEncodingByName(std::string_view name);
    Task<Expected<>> parseHeaders(HTTPHeaders &headers);
    Task<Expected<>> dumpHeaders(HTTPHeaders const &headers);
    void handleContentEncoding(HTTPHeaders &headers);
    void handleAcceptEncoding(HTTPHeaders &headers);
    void
    negotiateAcceptEncoding(HTTPHeaders &headers,
                            std::span<HTTPContentEncoding const> encodings);
    Task<Expected<>> writeChunked(BorrowedStream &body);
    Task<Expected<>> writeChunkedString(std::string_view body);
    Task<Expected<>> readChunked(BorrowedStream &body);
    Task<Expected<>> readChunkedString(String &body);
    Task<Expected<>> writeEncoded(BorrowedStream &body);
    Task<Expected<>> writeEncodedString(std::string_view body);
    Task<Expected<>> readEncoded(BorrowedStream &body);
    Task<Expected<>> readEncodedString(String &body);
#if CO_ASYNC_DEBUG
    void checkPhase(int from, int to);

private:
    int mPhase = 0;
#else
    void checkPhase(int from, int to);
#endif
public:
    Task<Expected<>> writeBodyStream(BorrowedStream &body) override;
    Task<Expected<>> writeBody(std::string_view body) override;
    Task<Expected<>> readBodyStream(BorrowedStream &body) override;
    Task<Expected<>> readBody(String &body) override;
    Task<Expected<>> writeRequest(HTTPRequest const &req) override;
    void initServerState() override;
    void initClientState() override;
    Task<Expected<>> readRequest(HTTPRequest &req) override;
    Task<Expected<>> writeResponse(HTTPResponse const &res) override;
    Task<Expected<>> readResponse(HTTPResponse &res) override;
    explicit HTTPProtocolVersion11(OwningStream sock);
    ~HTTPProtocolVersion11() override;
};

struct HTTPProtocolVersion2 : HTTPProtocolVersion11 {
    using HTTPProtocolVersion11::HTTPProtocolVersion11;
};
} // namespace co_async















namespace co_async {
enum class HTTPRouteMode {
    SuffixAny = 0, // "/a-9\\*g./.."
    SuffixName,    // "/a"
    SuffixPath,    // "/a/b/c"
};

struct SSLServerState {
    PImpl<SSLServerCertificate> cert;
    PImpl<SSLServerPrivateKey> skey;
    PImpl<SSLServerSessionCache> cache;
};

struct HTTPServer {
    struct IO {
        explicit IO(HTTPProtocol *http) noexcept : mHttp(http) {}

        HTTPRequest request;
        Task<Expected<bool>> readRequestHeader();
        Task<Expected<String>> request_body();
        Task<Expected<>> request_body_stream(OwningStream &out);
        Task<Expected<>> response(HTTPResponse resp, std::string_view content);
        Task<Expected<>> response(HTTPResponse resp, OwningStream &body);

        BorrowedStream &extractSocket() const noexcept {
            return mHttp->sock;
        }

    private:
        HTTPProtocol *mHttp;
        bool mBodyRead = false;
#if CO_ASYNC_DEBUG
        HTTPResponse mResponseSavedForDebug{};
        friend HTTPServer;
#endif
        void builtinHeaders(HTTPResponse &res);
    };

    using HTTPHandler = std::function<Task<Expected<>>(IO &)>;
    using HTTPPrefixHandler =
        std::function<Task<Expected<>>(IO &, std::string_view)>;
    /* using HTTPHandler = Task<Expected<>>(*)(IO &); */
    /* using HTTPPrefixHandler = Task<Expected<>>(*)(IO &, std::string_view); */
    HTTPServer();
    ~HTTPServer();
    HTTPServer(HTTPServer &&) = delete;
#if CO_ASYNC_DEBUG
    void enableLogRequests();
#endif
    void timeout(std::chrono::steady_clock::duration timeout);
    void route(std::string_view methods, std::string_view path,
               HTTPHandler handler);
    void route(std::string_view methods, std::string_view prefix,
               HTTPRouteMode mode, HTTPPrefixHandler handler);
    void route(HTTPHandler handler);
    Task<std::unique_ptr<HTTPProtocol>>
    prepareHTTPS(SocketHandle handle, SSLServerState &https) const;
    Task<std::unique_ptr<HTTPProtocol>> prepareHTTP(SocketHandle handle) const;
    Task<Expected<>> handle_http(SocketHandle handle) const;
    Task<Expected<>> handle_http_redirect_to_https(SocketHandle handle) const;
    Task<Expected<>> handle_https(SocketHandle handle,
                                  SSLServerState &https) const;
    Task<Expected<>>
    doHandleConnection(std::unique_ptr<HTTPProtocol> http) const;
    static Task<Expected<>> make_error_response(IO &io, int status);

private:
    struct Impl;
    std::unique_ptr<Impl> const mImpl;
};
} // namespace co_async







namespace co_async {
struct HTTPServerUtils {
    static String html_encode(std::string_view str);
    static Task<Expected<>>
    make_ok_response(HTTPServer::IO &io, std::string_view body,
                     String contentType = "text/html;charset=utf-8");
    static Task<Expected<>>
    make_response_from_directory(HTTPServer::IO &io,
                                 std::filesystem::path path);
    static Task<Expected<>> make_error_response(HTTPServer::IO &io, int status);
    static Task<Expected<>>
    make_response_from_file_or_directory(HTTPServer::IO &io,
                                         std::filesystem::path path);
    static Task<Expected<>> make_response_from_path(HTTPServer::IO &io,
                                                    std::filesystem::path path);
    static Task<Expected<>> make_response_from_file(HTTPServer::IO &io,
                                                    std::filesystem::path path);
};
} // namespace co_async



















namespace co_async {
struct HTTPConnection {
private:
    struct HTTPProtocolFactory {
    protected:
        std::string mHost;
        int mPort;
        std::string mHostName;
        std::string mProxy;
        std::chrono::steady_clock::duration mTimeout;

    public:
        HTTPProtocolFactory(std::string host, int port,
                            std::string_view hostName, std::string proxy,
                            std::chrono::steady_clock::duration timeout)
            : mHost(std::move(host)),
              mPort(port),
              mHostName(hostName),
              mProxy(std::move(proxy)),
              mTimeout(timeout) {}

        virtual Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() = 0;
        virtual ~HTTPProtocolFactory() = default;

        std::string const &hostName() const noexcept {
            return mHostName;
        }
    };

    struct HTTPProtocolFactoryHTTPS : HTTPProtocolFactory {
        using HTTPProtocolFactory::HTTPProtocolFactory;

        static PImpl<SSLClientTrustAnchor> &trustAnchors() {
            static PImpl<SSLClientTrustAnchor> instance;
            return instance;
        }

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            static CallOnce taInitializeOnce;
            static char const *const protocols[] = {
                /* "h2", */
                "http/1.1",
            };
            if (auto locked = co_await taInitializeOnce.call_once()) {
                auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
                auto content = co_await co_await file_read(path);
                co_await trustAnchors().add(content);
                locked.set_ready();
            }
            auto sock = co_await co_await ssl_connect(mHost.c_str(), mPort,
                                                      trustAnchors(), protocols,
                                                      mProxy, mTimeout);
            co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        }
    };

    struct HTTPProtocolFactoryHTTP : HTTPProtocolFactory {
        using HTTPProtocolFactory::HTTPProtocolFactory;

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            auto sock = co_await co_await tcp_connect(mHost.c_str(), mPort,
                                                      mProxy, mTimeout);
            co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        }
    };

    std::unique_ptr<HTTPProtocol> mHttp;
    std::unique_ptr<HTTPProtocolFactory> mHttpFactory;
    friend struct HTTPConnectionPool;

    void terminateLifetime() {
        mHttp = nullptr;
        mHttpFactory = nullptr;
    }

    struct RAIIPointerResetter {
        std::unique_ptr<HTTPProtocol> *mHttp;
        RAIIPointerResetter &operator=(RAIIPointerResetter &&) = delete;

        void neverMind() {
            mHttp = nullptr;
        }

        ~RAIIPointerResetter() {
            if (mHttp) [[unlikely]] {
                *mHttp = nullptr;
            }
        }
    };

    void builtinHeaders(HTTPRequest &req) {
        using namespace std::string_literals;
#if CO_ASYNC_DEBUG
        if (!mHttpFactory) [[unlikely]] {
            throw std::logic_error("http factory not initialized");
        }
#endif
        req.headers.insert("host"_s, String{mHttpFactory->hostName()});
        req.headers.insert("user-agent"_s, "co_async/0.0.1"_s);
        req.headers.insert("accept"_s, "*/*"_s);
#if CO_ASYNC_ZLIB
        req.headers.insert("accept-encoding"_s, "deflate, gzip"_s);
#else
        req.headers.insert("accept-encoding"_s, "gzip"_s);
#endif
    }

    Task<Expected<>> tryWriteRequestAndBody(HTTPRequest const &request,
                                            std::string_view body) {
        if (!mHttp)
            mHttp = co_await co_await mHttpFactory->createConnection();
        co_await co_await mHttp->writeRequest(request);
        co_await co_await mHttp->writeBody(body);
        co_return {};
#if 0
        std::error_code ec;
        for (std::size_t n = 0; n < 3; ++n) {
            if (!mHttp) {
                if (auto e = co_await mHttpFactory->createConnection())
                    [[likely]] {
                    mHttp = std::move(*e);
                    mHttp->initClientState();
                } else {
                    ec = e.error();
                    continue;
                }
            }
            if (co_await mHttp->writeRequest(request) &&
                co_await mHttp->writeBody(body) &&
                co_await mHttp->sock.peekchar()) [[likely]] {
                co_return {};
            }
            mHttp = nullptr;
        }
        co_return ec;
#endif
    }

    /* Task<Expected<>> */
    /* tryWriteRequestAndBodyStream(HTTPRequest const &request, */
    /*                              BorrowedStream &bodyStream) { */
    /*     auto cachedStream = make_stream<CachedStream>(bodyStream); */
    /*     for (std::size_t n = 0; n < 3; ++n) { */
    /*         if (!mHttp) { */
    /*             if (auto e = co_await mHttpFactory->createConnection()) */
    /*                 [[likely]] { */
    /*                 mHttp = std::move(*e); */
    /*                 mHttp->initClientState(); */
    /*             } else { */
    /*                 continue; */
    /*             } */
    /*         } */
    /*         if (co_await mHttp->writeRequest(request) && */
    /*             co_await mHttp->writeBodyStream(cachedStream) && */
    /*             co_await mHttp->sock.peekchar()) [[likely]] { */
    /*             co_return {}; */
    /*         } */
    /*         (void)cachedStream.seek(0); */
    /*         mHttp = nullptr; */
    /*     } */
    /*     co_return std::errc::connection_aborted; */
    /* } */
    HTTPConnection(std::unique_ptr<HTTPProtocolFactory> httpFactory)
        : mHttp(nullptr),
          mHttpFactory(std::move(httpFactory)) {}

private:
    static std::tuple<std::string, int>
    parseHostAndPort(std::string_view hostName, int defaultPort) {
        int port = defaultPort;
        auto host = hostName;
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1)))
                [[likely]] {
                port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        return {std::string(host), port};
    }

public:
    BorrowedStream &extractSocket() const noexcept {
        return mHttp->sock;
    }

    Expected<> doConnect(std::string_view host,
                         std::chrono::steady_clock::duration timeout,
                         bool followProxy) {
        terminateLifetime();
        if (host.starts_with("https://")) {
            host.remove_prefix(8);
            std::string proxy;
            if (followProxy) [[likely]] {
                if (auto p = std::getenv("https_proxy")) {
                    proxy = p;
                }
            }
            auto [h, p] = parseHostAndPort(host, 443);
            mHttpFactory = std::make_unique<HTTPProtocolFactoryHTTPS>(
                std::move(h), p, host, std::move(proxy), timeout);
            return {};
        } else if (host.starts_with("http://")) {
            host.remove_prefix(7);
            std::string proxy;
            if (followProxy) {
                if (auto p = std::getenv("http_proxy")) {
                    proxy = p;
                }
            }
            auto [h, p] = parseHostAndPort(host, 80);
            mHttpFactory = std::make_unique<HTTPProtocolFactoryHTTP>(
                std::move(h), p, host, std::move(proxy), timeout);
            return {};
        } else [[unlikely]] {
            return std::errc::protocol_not_supported;
        }
    }

    HTTPConnection() = default;

    Task<Expected<std::tuple<HTTPResponse, String>>>
    request(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        String body;
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBody(body);
        reset.neverMind();
        co_return std::tuple{std::move(res), std::move(body)};
    }

    Task<Expected<std::tuple<HTTPResponse, OwningStream>>>
    request_streamed(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        std::string body;
        co_await co_await mHttp->readResponse(res);
        auto [r, w] = pipe_stream();
        co_spawn(pipe_bind(std::move(w),
                           [this](OwningStream &w) -> Task<Expected<>> {
                               co_await co_await mHttp->readBodyStream(w);
                               co_return {};
                           }));
        reset.neverMind();
        co_return std::tuple{res, std::move(r)};
    }
};

struct HTTPConnectionPool {
private:
    struct alignas(hardware_destructive_interference_size) PoolEntry {
        HTTPConnection mHttp;
        std::atomic_bool mInuse{false};
        bool mValid{false};
        std::chrono::steady_clock::time_point mLastAccess;
    };

    struct HostPool {
        std::vector<PoolEntry> mPool;
        ConditionVariable mFreeSlot;

        explicit HostPool(std::size_t size) : mPool(size) {}
    };

    std::shared_mutex mMutex;
    SimpleMap<std::string, HostPool> mPools;
    std::chrono::steady_clock::duration mTimeout;
    std::chrono::steady_clock::duration mKeepAlive;
    std::chrono::steady_clock::time_point mLastGC;
    std::size_t mConnPerHost;
    bool mFollowProxy;

public:
    struct HTTPConnectionPtr {
    private:
        PoolEntry *mEntry;
        HostPool *mPool;

        explicit HTTPConnectionPtr(PoolEntry *entry, HostPool *pool) noexcept
            : mEntry(entry),
              mPool(pool) {}

        friend HTTPConnectionPool;

    public:
        HTTPConnectionPtr() noexcept : mEntry(nullptr) {}

        HTTPConnectionPtr(HTTPConnectionPtr &&that) noexcept
            : mEntry(std::exchange(that.mEntry, nullptr)),
              mPool(std::exchange(that.mPool, nullptr)) {}

        HTTPConnectionPtr &operator=(HTTPConnectionPtr &&that) noexcept {
            std::swap(mEntry, that.mEntry);
            std::swap(mPool, that.mPool);
            return *this;
        }

        ~HTTPConnectionPtr() {
            if (mEntry) {
                mEntry->mLastAccess = std::chrono::steady_clock::now();
                mEntry->mInuse.store(false, std::memory_order_release);
                mPool->mFreeSlot.notify_one();
            }
        }

        HTTPConnection &operator*() const noexcept {
            return mEntry->mHttp;
        }

        HTTPConnection *operator->() const noexcept {
            return &mEntry->mHttp;
        }
    };

    explicit HTTPConnectionPool(
        std::size_t connPerHost = 8,
        std::chrono::steady_clock::duration timeout = std::chrono::seconds(20),
        std::chrono::steady_clock::duration keepAlive = std::chrono::minutes(3),
        bool followProxy = true)
        : mTimeout(timeout),
          mKeepAlive(keepAlive),
          mConnPerHost(connPerHost),
          mFollowProxy(followProxy) {}

private:
    std::optional<Expected<HTTPConnectionPtr>>
    lookForFreeSlot(std::string_view host) /* MT-safe */ {
        std::shared_lock lock(mMutex);
        auto *pool = mPools.at(host);
        lock.unlock();
        if (!pool) {
            std::lock_guard wlock(mMutex);
            pool = mPools.at(host);
            if (!pool) [[likely]] {
                pool = &mPools.emplace(std::string(host), mConnPerHost);
            }
        }
        for (auto &entry: pool->mPool) {
            bool expected = false;
            if (entry.mInuse.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel)) {
                if (entry.mValid) {
                    entry.mLastAccess = std::chrono::steady_clock::now();
                } else {
                    if (auto e =
                            entry.mHttp.doConnect(host, mTimeout, mFollowProxy);
                        e.has_error()) [[unlikely]] {
                        return std::move(e).error();
                    }
                    entry.mLastAccess = std::chrono::steady_clock::now();
                    entry.mValid = true;
                }
                return HTTPConnectionPtr(&entry, pool);
            }
        }
        return std::nullopt;
    }

    Task<> waitForFreeSlot(std::string_view host) /* MT-safe */ {
        std::shared_lock lock(mMutex);
        auto *pool = mPools.at(host);
        lock.unlock();
        if (pool) [[likely]] {
            (void)co_await co_timeout(pool->mFreeSlot.wait(),
                                      std::chrono::milliseconds(100));
        }
        co_return;
    }

    void garbageCollect() /* MT-safe */ {
        auto now = std::chrono::steady_clock::now();
        if ((now - mLastGC) * 2 > mKeepAlive) {
            std::shared_lock lock(mMutex);
            for (auto &[_, pool]: mPools) {
                for (auto &entry: pool.mPool) {
                    bool expected = false;
                    if (entry.mInuse.compare_exchange_strong(
                            expected, true, std::memory_order_acq_rel)) {
                        if (entry.mValid &&
                            now - entry.mLastAccess > mKeepAlive) {
                            entry.mHttp.terminateLifetime();
                            entry.mValid = false;
                        }
                        entry.mInuse.store(false, std::memory_order_release);
                    }
                }
                mLastGC = now;
            }
        }
    }

public:
    Task<Expected<HTTPConnectionPtr>>
    connect(std::string_view host) /* MT-safe */ {
    again:
        garbageCollect();
        if (auto conn = lookForFreeSlot(host)) {
            co_return std::move(*conn);
        }
        co_await waitForFreeSlot(host);
        goto again;
    }
};
} // namespace co_async












#include <hashlib/hashlib.hpp>

namespace co_async {

// 小彭老师带你用 C++ 实现 WebSocket 协议
// WebSocket 是为了解决 HTTP 协议的一些缺陷而提出的
// 过去，HTTP 只能客户端单向地往服务端发出请求，服务端被动地返回响应
// 要获取服务端的动态数据，只能通过轮询或长轮询的形式（参见上一期视频）
// 在聊天室这种少量人员的场景中，轮询还扛得住，但在实时音视频，直播领域，HTTP 轮询的延迟就太高了
// 而 WebSocket 是一种双向通信协议，建立连接后，服务端和客户端都能主动向对方发送或接收数据
// WebSocket 面向二进制字节流，类似于 TCP，比基于文本传输数据的 HTTP 协议更高效
// 主流浏览器都提供 WebSocket 的 API，可以实现浏览器和服务器的双向实时通信
// 我们这一期视频主要来实现 C++ 的服务器端，要求能够与浏览器中的 JS 建立 WebSocket 连接
// 如果时间来得及，我们希望利用这个 WebSocket 服务器实现实时语音通话

inline std::string websocketGenerateNonce() {
    uint32_t seed = getSeedByTime();
    uint8_t buf[16];
    for (size_t i = 0; i != 16; ++i) {
        seed = wangsHash(seed);
        buf[i] = static_cast<uint8_t>(seed & 0xFF);
    }
    return base64::encode_into<std::string>(buf, buf + 16);
}

inline std::string websocketSecretHash(std::string userKey) {
    // websocket 官方要求的神秘仪式
    SHA1 sha1;
    std::string inKey = userKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1.add(inKey.data(), inKey.size());
    uint8_t buf[SHA1::HashBytes];
    sha1.getHash(buf);
    return base64::encode_into<std::string>(buf, buf + SHA1::HashBytes);
}

inline Task<Expected<bool>> httpUpgradeToWebSocket(HTTPServer::IO &io) {
    if (io.request.headers.get("upgrade") != "websocket") {
        co_return false;
    }
    // 是 ws:// 请求
    auto wsKey = io.request.headers.get("sec-websocket-key");
    if (!wsKey) {
        co_await co_await HTTPServerUtils::make_error_response(io, 400);
        co_return true;
    }
    auto wsNewKey = websocketSecretHash(*wsKey);
    HTTPResponse res{
        .status = 101,
        .headers =
        {
            {"connection", "Upgrade"},
            {"upgrade", "websocket"},
            {"sec-websocket-accept", wsNewKey},
        },
    };
    co_await co_await io.response(res, "");
    co_return true;
}

struct WebSocketPacket {
    enum Opcode : uint8_t {
        kOpcodeText = 1,
        kOpcodeBinary = 2,
        kOpcodeClose = 8,
        kOpcodePing = 9,
        kOpcodePong = 10,
    } opcode;
    std::string content;

    REFLECT(opcode, content);
};

inline Task<Expected<WebSocketPacket>> wsRecvPacket(BorrowedStream &ws) {
    WebSocketPacket packet;
    auto head = co_await co_await ws.getn(2);
    bool fin;
    do {
        uint8_t head0 = static_cast<uint8_t>(head[0]);
        uint8_t head1 = static_cast<uint8_t>(head[1]);
        fin = (head0 & 0x80) != 0;
        packet.opcode = static_cast<WebSocketPacket::Opcode>(head0 & 0x0F);
        bool masked = (head1 & 0x80) != 0;
        uint8_t payloadLen8 = head1 & 0x7F;
        size_t payloadLen;
        if (packet.opcode >= 8 && packet.opcode <= 10 && payloadLen8 >= 0x7E) [[unlikely]] {
            co_return std::errc::protocol_error;
        }
        if (payloadLen8 == 0x7E) {
            auto payloadLen16 = byteswap_if_little(co_await co_await ws.getstruct<uint16_t>());
            payloadLen = static_cast<size_t>(payloadLen16);
        } else if (payloadLen8 == 0x7F) {
            auto payloadLen64 = byteswap_if_little(co_await co_await ws.getstruct<uint64_t>());
            if constexpr (sizeof(uint64_t) > sizeof(size_t)) {
                if (payloadLen64 > std::numeric_limits<size_t>::max()) {
                    co_return std::errc::not_enough_memory;
                }
            }
            payloadLen = static_cast<size_t>(payloadLen64);
        } else {
            payloadLen = static_cast<size_t>(payloadLen8);
        }
        std::string mask;
        if (masked) {
            mask = co_await co_await ws.getn(4);
        }
        auto data = co_await co_await ws.getn(payloadLen);
        if (masked) {
            for (size_t i = 0; i != data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }
        }
        packet.content += data;
    } while (!fin);
    co_return std::move(packet);
}

inline Task<Expected<>> wsSendPacket(BorrowedStream &ws, WebSocketPacket packet, uint32_t mask = 0) {
    const bool fin = true;
    bool masked = mask != 0;
    uint8_t payloadLen8 = 0;
    if (packet.content.size() < 0x7E) {
        payloadLen8 = static_cast<uint8_t>(packet.content.size());
    } else if (packet.content.size() <= 0xFFFF) {
        payloadLen8 = 0x7E;
    } else {
        payloadLen8 = 0x7F;
    }
    uint8_t head0 = (fin ? 1 : 0) << 7 | static_cast<uint8_t>(packet.opcode);
    uint8_t head1 = (masked ? 1 : 0) << 7 | payloadLen8;
    char head[2];
    head[0] = static_cast<uint8_t>(head0);
    head[1] = static_cast<uint8_t>(head1);
    co_await co_await ws.write(head);
    if (packet.content.size() > 0x7E) {
        if (packet.content.size() <= 0xFFFF) {
            auto payloadLen16 = static_cast<uint16_t>(packet.content.size());
            co_await co_await ws.putstruct(byteswap_if_little(payloadLen16));
        } else {
            auto payloadLen64 = static_cast<uint64_t>(packet.content.size());
            co_await co_await ws.putstruct(byteswap_if_little(payloadLen64));
        }
    }
    if (masked) {
        char mask_buf[4];
        mask_buf[0] = mask >> 24;
        mask_buf[1] = (mask >> 16) & 0xFF;
        mask_buf[2] = (mask >> 8) & 0xFF;
        mask_buf[3] = mask & 0xFF;
        co_await co_await ws.write(mask_buf);
        for (size_t i = 0; i != packet.content.size(); ++i) {
            packet.content[i] ^= mask_buf[i % 4];
        }
    }
    co_await co_await ws.write(packet.content);
    co_await co_await ws.flush();
    co_return {};
}

struct WebSocket {
    BorrowedStream &sock;
    std::function<Task<Expected<>>(std::string const &)> mOnMessage;
    std::function<Task<Expected<>>()> mOnClose;
    std::function<Task<Expected<>>(std::chrono::steady_clock::duration)> mOnPong;
    bool mHalfClosed = false;
    bool mWaitingPong = true;
    std::chrono::steady_clock::time_point mLastPingTime{};

    WebSocket(WebSocket &&) = default;

    explicit WebSocket(BorrowedStream &sock) : sock(sock) {
    }

    bool is_closing() const noexcept {
        return mHalfClosed;
    }

    void on_message(std::function<Task<Expected<>>(std::string const &)> onMessage) {
        mOnMessage = std::move(onMessage);
    }

    void on_close(std::function<Task<Expected<>>()> onClose) {
        mOnClose = std::move(onClose);
    }

    void on_pong(std::function<Task<Expected<>>(std::chrono::steady_clock::duration)> onPong) {
        mOnPong = std::move(onPong);
    }

    Task<Expected<>> send(std::string text) {
        if (mHalfClosed) [[unlikely]] {
            co_return std::errc::broken_pipe;
        }
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodeText,
            .content = text,
        });
    }

    Task<Expected<>> close(uint16_t code = 1000) {
        std::string content;
        code = byteswap_if_little(code);
        content.resize(sizeof(code));
        std::memcpy(content.data(), &code, sizeof(code));
        mHalfClosed = true;
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodeClose,
            .content = content,
        });
    }

    Task<Expected<>> sendPing() {
        mLastPingTime = std::chrono::steady_clock::now();
        // debug(), "主动ping";
        co_return co_await wsSendPacket(sock, WebSocketPacket{
            .opcode = WebSocketPacket::kOpcodePing,
            .content = {},
        });
    }

    Task<Expected<>> start(std::chrono::steady_clock::duration pingPongTimeout = std::chrono::seconds(5)) {
        while (true) {
            auto maybePacket = co_await co_timeout(wsRecvPacket(sock), pingPongTimeout);
            if (maybePacket == std::errc::stream_timeout) {
                if (mWaitingPong) {
                    break;
                }
                co_await co_await sendPing();
                mWaitingPong = true;
                continue;
            }
            mWaitingPong = false;
            if (maybePacket == eofError()) {
                break;
            }
            auto packet = co_await std::move(maybePacket);
            if (packet.opcode == packet.kOpcodeText || packet.opcode == packet.kOpcodeBinary) {
                if (mOnMessage) {
                    co_await co_await mOnMessage(packet.content);
                }
            } else if (packet.opcode == packet.kOpcodePing) {
                // debug(), "收到ping";
                packet.opcode = packet.kOpcodePong;
                co_await co_await wsSendPacket(sock, packet);
            } else if (packet.opcode == packet.kOpcodePong) {
                auto now = std::chrono::steady_clock::now();
                if (mOnPong && mLastPingTime.time_since_epoch().count() != 0) {
                    auto dt = now - mLastPingTime;
                    co_await co_await mOnPong(dt);
                    // debug(), "网络延迟:", dt;
                }
                // debug(), "收到pong";
            } else if (packet.opcode == packet.kOpcodeClose) {
                // debug(), "收到关闭请求";
                if (mOnClose) {
                    co_await co_await mOnClose();
                }
                if (!mHalfClosed) {
                    co_await co_await wsSendPacket(sock, packet);
                    mHalfClosed = true;
                } else {
                    break;
                }
            }
        }
        co_await sock.close();
        co_return {};
    }
};

inline Task<Expected<WebSocket>> websocket_server(HTTPServer::IO &io) {
    if (co_await co_await httpUpgradeToWebSocket(io)) {
        co_return WebSocket(io.extractSocket());
    }
    co_return std::errc::protocol_error;
}

inline Task<Expected<WebSocket>> websocket_client(HTTPConnection &conn, URI uri) {
    std::string nonceKey;
    using namespace std::string_literals;
    nonceKey = websocketGenerateNonce();
    HTTPRequest request = {
        .method = "GET"s,
        .uri = uri,
        .headers = {
            {"sec-websocket-key"s, nonceKey},
            {"connection"s, "Upgrade"s},
            {"upgrade"s, "websocket"s},
            {"sec-websocket-version"s, "13"s},
        },
    };
    auto [response, _] = co_await co_await conn.request(request);
    if (response.headers.get("sec-websocket-accept") != websocketSecretHash(nonceKey)) {
        co_return std::errc::protocol_error;
    }
    co_return WebSocket(conn.extractSocket());
}

}












































































#ifdef CO_ASYNC_IMPLEMENTATION



namespace co_async {
thread_local std::pmr::memory_resource *currentAllocator =
    std::pmr::new_delete_resource();

#if CO_ASYNC_ALLOC
namespace {
inline struct DefaultResource : std::pmr::memory_resource {
    void *do_allocate(size_t size, size_t align) override {
        return currentAllocator->allocate(size, align);
    }

    void do_deallocate(void *p, size_t size, size_t align) override {
        return currentAllocator->deallocate(p, size, align);
    }

    bool do_is_equal(
        std::pmr::memory_resource const &other) const noexcept override {
        return currentAllocator->is_equal(other);
    }

    DefaultResource() noexcept {
        std::pmr::set_default_resource(this);
    }

    DefaultResource(DefaultResource &&) = delete;
} defaultResource;
} // namespace
#endif
} // namespace co_async





namespace co_async {

struct ThreadPool::Thread {
    std::function<void()> mTask;
    std::condition_variable mCV;
    std::mutex mMutex;
    ThreadPool *mPool;
    std::jthread mThread;

    ~Thread() {
        mThread.request_stop();
        mCV.notify_one();
        mThread.join();
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            std::unique_lock lock(mMutex);
            mCV.wait(lock,
                     [&] { return mTask != nullptr || stop.stop_requested(); });
            if (stop.stop_requested()) [[unlikely]] {
                return;
            }
            auto task = std::exchange(mTask, nullptr);
            lock.unlock();
            task();
            lock.lock();
            mTask = nullptr;
            std::unique_lock workingLock(mPool->mWorkingMutex);
            mPool->mWorkingThreads.remove(this);
            workingLock.unlock();
            std::lock_guard freeLock(mPool->mFreeMutex);
            mPool->mFreeThreads.push_back(this);
        }
    }

    explicit Thread(ThreadPool *pool) {
        mPool = pool;
        mThread =
            std::jthread([this](std::stop_token stop) { startMain(stop); });
    }

    Thread(Thread &&) = delete;

    void workOn(std::function<void()> func) {
        std::lock_guard lock(mMutex);
        mTask = std::move(func);
        mCV.notify_one();
    }
};

ThreadPool::Thread *ThreadPool::submitJob(std::function<void()> func) {
    std::unique_lock freeLock(mFreeMutex);
    if (mFreeThreads.empty()) {
        freeLock.unlock();
        std::unique_lock threadsLock(mThreadsMutex);
        // list 保证插入后元素的指针和引用永远不会失效
        // 所以在这里获取地址并存入其他链表是安全的：
        Thread *newThread = &mThreads.emplace_back(this);
        threadsLock.unlock();
        newThread->workOn(std::move(func));
        std::lock_guard workingLock(mWorkingMutex);
        mWorkingThreads.push_back(newThread);
        return newThread;
    } else {
        Thread *freeThread = std::move(mFreeThreads.front());
        mFreeThreads.pop_front();
        freeLock.unlock();
        freeThread->workOn(std::move(func));
        std::lock_guard workingLock(mWorkingMutex);
        mWorkingThreads.push_back(freeThread);
        return freeThread;
    }
}

Task<Expected<>> ThreadPool::rawRun(std::function<void()> func) {
    auto ready = std::make_shared<FutexAtomic<bool>>(false);
    std::exception_ptr ep;
    submitJob([ready, func = std::move(func), &ep]() mutable {
        try {
            func();
        } catch (...) {
            ep = std::current_exception();
        }
        ready->store(true, std::memory_order_release);
        (void)futex_notify_sync(ready.get());
    });
    while (ready->load(std::memory_order_acquire) == false) {
        (void)co_await futex_wait(ready.get(), false);
    }
    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    co_return {};
}

Task<Expected<>> ThreadPool::rawRun(std::function<void(std::stop_token)> func,
                                    CancelToken cancel) {
    auto ready = std::make_shared<FutexAtomic<bool>>(false);
    std::stop_source stop;
    bool stopped = false;
    std::exception_ptr ep;
    submitJob([ready, func = std::move(func), stop = stop.get_token(),
               &ep]() mutable {
        try {
            func(stop);
        } catch (...) {
            ep = std::current_exception();
        }
        ready->store(true, std::memory_order_release);
        (void)futex_notify_sync(ready.get());
    });

    {
        CancelCallback _(cancel, [&] {
            stopped = true;
            stop.request_stop();
        });
        while (ready->load(std::memory_order_acquire) == false) {
            (void)co_await futex_wait(ready.get(), false);
        }
    }

    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    if (stopped) {
        co_return std::errc::operation_canceled;
    }
    co_return {};
}

std::size_t ThreadPool::threads_count() {
    std::lock_guard lock(mThreadsMutex);
    return mThreads.size();
}

std::size_t ThreadPool::working_threads_count() {
    std::lock_guard lock(mWorkingMutex);
    return mWorkingThreads.size();
}

ThreadPool::ThreadPool() = default;
ThreadPool::~ThreadPool() = default;

} // namespace co_async









namespace co_async {

IOContext::IOContext(IOContextOptions options) {
    if (instance) {
        throw std::logic_error("each thread may create only one IOContext");
    }
    instance = this;
    GenericIOContext::instance = &mGenericIO;
    PlatformIOContext::instance = &mPlatformIO;
    if (options.threadAffinity) {
        PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
    }
    mPlatformIO.setup(options.queueEntries);
    mMaxSleep = options.maxSleep;
}

IOContext::~IOContext() {
    IOContext::instance = nullptr;
    GenericIOContext::instance = nullptr;
    PlatformIOContext::instance = nullptr;
}

void IOContext::run() {
    while (runOnce())
        ;
}

bool IOContext::runOnce() {
    auto duration = mGenericIO.runDuration();
    if (!duration && !mPlatformIO.hasPendingEvents()) [[unlikely]] {
        return false;
    }
    if (!duration || *duration > mMaxSleep) {
        duration = mMaxSleep;
    }
    mPlatformIO.waitEventsFor(duration);
    return true;
}

thread_local IOContext *IOContext::instance;

// void IOContext::wakeUp() {
//     if (mWake.fetch_add(1, std::memory_order_relaxed) == 0)
//         futex_notify_sync(&mWake, 1);
// }
//
// Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
// IOContext::watchDogTask() {
//     // helps wake up main loop when IOContext::spawn called
//     while (true) {
//         while (mWake.load(std::memory_order_relaxed) == 0)
//             (void)co_await futex_wait(&mWake, 0);
//         mWake.store(0, std::memory_order_relaxed);
//     }
// }

} // namespace co_async








namespace co_async {
IOContextMT::IOContextMT() {
    if (IOContextMT::instance) [[unlikely]] {
        throw std::logic_error("each process may contain only one IOContextMT");
    }
    IOContextMT::instance = this;
}

IOContextMT::~IOContextMT() {
    IOContextMT::instance = nullptr;
}

void IOContextMT::run(std::size_t numWorkers) {
    // if (numWorkers == 0) {
    //     setAffinity = true;
    //     numWorkers = std::thread::hardware_concurrency();
    //     if (!numWorkers) [[unlikely]] {
    //         throw std::logic_error(
    //             "failed to detect number of hardware threads");
    //     }
    // } else {
    //     setAffinity = false;
    // }
    instance->mWorkers = std::make_unique<IOContext[]>(numWorkers);
    instance->mNumWorkers = numWorkers;
    for (std::size_t i = 0; i < instance->mNumWorkers; ++i) {
        instance->mWorkers[i].run();
    }
}

IOContextMT *IOContextMT::instance;
} // namespace co_async




namespace co_async {

// bool GenericIOContext::runComputeOnly() {
//     if (auto coroutine = mQueue.pop()) {
//         coroutine->resume();
//         return true;
//     }
//     return false;
// }
//
GenericIOContext::GenericIOContext() = default;
GenericIOContext::~GenericIOContext() = default;

std::optional<std::chrono::steady_clock::duration>
GenericIOContext::runDuration() {
    while (true) {
        if (!mTimers.empty()) {
            auto &promise = mTimers.front();
            std::chrono::steady_clock::time_point now =
                std::chrono::steady_clock::now();
            if (promise.mExpires <= now) {
                promise.mCancelled = false;
                promise.erase_from_parent();
                std::coroutine_handle<TimerNode>::from_promise(promise).resume();
                continue;
            } else {
                return promise.mExpires - now;
            }
        } else {
            return std::nullopt;
        }
    }
}

//
// void GenericIOContext::startMain(std::stop_token stop) {
//     while (!stop.stop_requested()) [[likely]] {
//         auto duration = runDuration();
//         if (duration) {
//             std::this_thread::sleep_for(*duration);
//         } else {
//             break;
//         }
//     }
// }
//
} // namespace co_async





#include <fcntl.h>
#include <liburing.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
void PlatformIOContext::schedSetThreadAffinity(size_t cpu) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    throwingErrorErrno(
        sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpu_set));
}

PlatformIOContext::IOUringProbe::IOUringProbe() {
    mRing = nullptr;
    // mProbe = io_uring_get_probe();
    mProbe = nullptr;
    if (!mProbe) {
        mRing = new struct io_uring;
        throwingError(io_uring_queue_init(8, mRing, 0));
    }
}

PlatformIOContext::IOUringProbe::~IOUringProbe() {
    if (mProbe) {
        io_uring_free_probe(mProbe);
    }
    if (mRing) {
        io_uring_queue_exit(mRing);
        delete mRing;
    }
}

bool PlatformIOContext::IOUringProbe::isSupported(int op) noexcept {
    if (mProbe) {
        return io_uring_opcode_supported(mProbe, op);
    }
    if (mRing) {
        struct io_uring_sqe *sqe = io_uring_get_sqe(mRing);
        io_uring_prep_rw(op, sqe, -1, nullptr, 0, 0);
        struct io_uring_cqe *cqe;
        throwingError(io_uring_submit(mRing));
        throwingError(io_uring_wait_cqe(mRing, &cqe));
        int res = cqe->res;
        io_uring_cqe_seen(mRing, cqe);
        return res != ENOSYS;
    }
    return false;
}

void PlatformIOContext::IOUringProbe::dumpDiagnostics() {
    static char const *ops[IORING_OP_LAST + 1] = {
        "IORING_OP_NOP",
        "IORING_OP_READV",
        "IORING_OP_WRITEV",
        "IORING_OP_FSYNC",
        "IORING_OP_READ_FIXED",
        "IORING_OP_WRITE_FIXED",
        "IORING_OP_POLL_ADD",
        "IORING_OP_POLL_REMOVE",
        "IORING_OP_SYNC_FILE_RANGE",
        "IORING_OP_SENDMSG",
        "IORING_OP_RECVMSG",
        "IORING_OP_TIMEOUT",
        "IORING_OP_TIMEOUT_REMOVE",
        "IORING_OP_ACCEPT",
        "IORING_OP_ASYNC_CANCEL",
        "IORING_OP_LINK_TIMEOUT",
        "IORING_OP_CONNECT",
        "IORING_OP_FALLOCATE",
        "IORING_OP_OPENAT",
        "IORING_OP_CLOSE",
        "IORING_OP_FILES_UPDATE",
        "IORING_OP_STATX",
        "IORING_OP_READ",
        "IORING_OP_WRITE",
        "IORING_OP_FADVISE",
        "IORING_OP_MADVISE",
        "IORING_OP_SEND",
        "IORING_OP_RECV",
        "IORING_OP_OPENAT2",
        "IORING_OP_EPOLL_CTL",
        "IORING_OP_SPLICE",
        "IORING_OP_PROVIDE_BUFFERS",
        "IORING_OP_REMOVE_BUFFERS",
        "IORING_OP_TEE",
        "IORING_OP_SHUTDOWN",
        "IORING_OP_RENAMEAT",
        "IORING_OP_UNLINKAT",
        "IORING_OP_MKDIRAT",
        "IORING_OP_SYMLINKAT",
        "IORING_OP_LINKAT",
        "IORING_OP_MSG_RING",
        "IORING_OP_FSETXATTR",
        "IORING_OP_SETXATTR",
        "IORING_OP_FGETXATTR",
        "IORING_OP_GETXATTR",
        "IORING_OP_SOCKET",
        "IORING_OP_URING_CMD",
        "IORING_OP_SEND_ZC",
        "IORING_OP_SENDMSG_ZC",
        "IORING_OP_READ_MULTISHOT",
        "IORING_OP_WAITID",
        "IORING_OP_FUTEX_WAIT",
        "IORING_OP_FUTEX_WAKE",
        "IORING_OP_FUTEX_WAITV",
        "IORING_OP_FIXED_FD_INSTALL",
        "IORING_OP_FTRUNCATE",
        "IORING_OP_LAST",
    };
    for (int op = IORING_OP_NOP; op < IORING_OP_LAST; ++op) {
        bool ok = isSupported(op);
        std::cerr << "opcode " << ops[op] << (ok ? "" : " not") << " supported"
                  << '\n';
    }
}

PlatformIOContext::PlatformIOContext() noexcept {
    mRing.ring_fd = -1;
}

void PlatformIOContext::setup(std::size_t entries) {
    unsigned int flags = 0;
#if CO_ASYNC_DIRECT
    flags |= IORING_SETUP_IOPOLL;
#endif
    throwingError(
        io_uring_queue_init(static_cast<unsigned int>(entries), &mRing, flags));
}

void PlatformIOContext::reserveBuffers(std::size_t nbufs) {
    auto oldBuf = std::move(mBuffers);
    mBuffers = std::make_unique<struct iovec[]>(nbufs);
    if (mCapBufs) {
        throwingError(io_uring_unregister_buffers(&mRing));
    }
    mCapBufs = static_cast<unsigned int>(nbufs);
    std::memcpy(mBuffers.get(), oldBuf.get(), sizeof(struct iovec) * mNumBufs);
    throwingError(io_uring_register_buffers_sparse(
        &mRing, static_cast<unsigned int>(nbufs)));
    std::vector<__u64> tags(mNumBufs, 0);
    throwingError(io_uring_register_buffers_update_tag(
        &mRing, 0, mBuffers.get(), tags.data(),
        static_cast<unsigned int>(mNumBufs)));
}

std::size_t
PlatformIOContext::addBuffers(std::span<std::span<char> const> bufs) {
    if (mNumBufs >= mCapBufs) {
        reserveBuffers(mCapBufs * 2 + 1);
    }
    auto outP = mBuffers.get() + mNumBufs;
    for (auto const &buf: bufs) {
        struct iovec iov;
        iov.iov_base = buf.data();
        iov.iov_len = buf.size();
        *outP++ = iov;
    }
    std::vector<__u64> tags(bufs.size(), 0);
    throwingError(io_uring_register_buffers_update_tag(
        &mRing, mNumBufs, mBuffers.get() + mNumBufs, tags.data(),
        static_cast<unsigned int>(bufs.size())));
    size_t ret = mNumBufs;
    mNumBufs += static_cast<unsigned int>(bufs.size());
    return ret;
}

void PlatformIOContext::reserveFiles(std::size_t nfiles) {
    auto oldBuf = std::move(mBuffers);
    mBuffers = std::make_unique<struct iovec[]>(nfiles);
    if (mCapFiles) {
        throwingError(io_uring_unregister_files(&mRing));
    }
    mCapFiles = static_cast<unsigned int>(nfiles);
    std::memcpy(mBuffers.get(), oldBuf.get(), sizeof(struct iovec) * mNumBufs);
    throwingError(io_uring_register_files_sparse(
        &mRing, static_cast<unsigned int>(nfiles)));
    std::vector<__u64> tags(mNumFiles, 0);
    throwingError(io_uring_register_files_update_tag(&mRing, 0, mFiles.get(),
                                                     tags.data(), mNumFiles));
}

std::size_t PlatformIOContext::addFiles(std::span<int const> files) {
    if (mNumFiles >= mCapFiles) {
        reserveBuffers(mCapFiles * 2 + 1);
    }
    auto outP = mFiles.get() + mNumFiles;
    for (auto const &file: files) {
        *outP++ = file;
    }
    std::vector<__u64> tags(files.size(), 0);
    throwingError(io_uring_register_files_update_tag(
        &mRing, mNumFiles, mFiles.get() + mNumFiles, tags.data(),
        static_cast<unsigned int>(files.size())));
    size_t ret = mNumFiles;
    mNumFiles += static_cast<unsigned int>(files.size());
    return ret;
}

PlatformIOContext::~PlatformIOContext() {
    if (mRing.ring_fd != -1) {
        io_uring_queue_exit(&mRing);
    }
}

thread_local PlatformIOContext *PlatformIOContext::instance;

bool PlatformIOContext::waitEventsFor(
    std::optional<std::chrono::steady_clock::duration> timeout) {
    // debug(), "wait", this, mNumSqesPending;
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(&mRing, &cqe, 1, tsp, nullptr);
    if (res == -ETIME) {
        return false;
    } else if (res < 0) [[unlikely]] {
        if (res == -EINTR) {
            return false;
        }
        throw std::system_error(-res, std::system_category());
    }
    unsigned head, numGot = 0;
    std::vector<std::coroutine_handle<>> tasks;
    io_uring_for_each_cqe(&mRing, head, cqe) {
#if CO_ASYNC_INVALFIX
        if (cqe->user_data == LIBURING_UDATA_TIMEOUT) [[unlikely]] {
            ++numGot;
            continue;
        }
#endif
        auto *op = reinterpret_cast<UringOp *>(cqe->user_data);
        op->mRes = cqe->res;
        tasks.push_back(op->mPrevious);
        ++numGot;
    }
    io_uring_cq_advance(&mRing, numGot);
    mNumSqesPending -= static_cast<std::size_t>(numGot);
    for (auto const &task: tasks) {
#if CO_ASYNC_DEBUG
        if (!task) [[likely]] {
            std::cerr << "null coroutine pushed into task queue\n";
        }
        if (task.done()) [[likely]] {
            std::cerr << "done coroutine pushed into task queue\n";
        }
#endif
        task.resume();
    }
    return true;
}
} // namespace co_async

#include <arpa/inet.h>








#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace co_async {
std::error_category const &getAddrInfoCategory() {
    static struct final : std::error_category {
        char const *name() const noexcept override {
            return "getaddrinfo";
        }

        std::string message(int e) const override {
            return gai_strerror(e);
        }
    } instance;

    return instance;
}

// Expected<IpAddress> IpAddress::fromString(char const *host) {
//     struct in_addr addr = {};
//     struct in6_addr addr6 = {};
//     if (1 == inet_pton(AF_INET, host, &addr)) {
//         return IpAddress(addr);
//     }
//     if (1 == inet_pton(AF_INET6, host, &addr6)) {
//         return IpAddress(addr6);
//     }
//     // gethostbyname is deprecated, let's use getaddrinfo instead:
//     struct addrinfo hints = {};
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     struct addrinfo *result;
//     int err = getaddrinfo(host, nullptr, &hints, &result);
//     if (err) [[unlikely]] {
// #if CO_ASYNC_DEBUG
//         std::cerr << host << ": " << gai_strerror(err) << '\n';
// #endif
//         return std::error_code(err, getAddrInfoCategory());
//     }
//     Finally fin = [&] {
//         freeaddrinfo(result);
//     };
//     for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
//         if (rp->ai_family == AF_INET) {
//             std::memcpy(&addr, &reinterpret_cast<struct sockaddr_in
//             *>(rp->ai_addr)->sin_addr,
//                         sizeof(in_addr));
//             return IpAddress(addr);
//         } else if (rp->ai_family == AF_INET6) {
//             std::memcpy(&addr6,
//                         &reinterpret_cast<struct sockaddr_in6
//                         *>(rp->ai_addr)->sin6_addr, sizeof(in6_addr));
//             return IpAddress(addr6);
//         }
//     }
//     [[unlikely]] {
// #if CO_ASYNC_DEBUG
//         std::cerr << host << ": no matching host address with ipv4 or
//         ipv6\n";
// #endif
//         return std::errc::bad_address;
//     }
// }
//
// String IpAddress::toString() const {
//     if (mAddr.index() == 1) {
//         char buf[INET6_ADDRSTRLEN + 1] = {};
//         inet_ntop(AF_INET6, &std::get<1>(mAddr), buf, sizeof(buf));
//         return buf;
//     } else if (mAddr.index() == 0) {
//         char buf[INET_ADDRSTRLEN + 1] = {};
//         inet_ntop(AF_INET, &std::get<0>(mAddr), buf, sizeof(buf));
//         return buf;
//     } else {
//         return "[invalid ip address or domain name]";
//     }
// }

auto AddressResolver::resolve_all() -> Expected<ResolveResult> {
    // gethostbyname is deprecated, let's use getaddrinfo instead:
    if (m_host.empty()) [[unlikely]] {
        return std::errc::invalid_argument;
    }
    struct addrinfo *result;
    int err = getaddrinfo(m_host.c_str(),
                          m_service.empty() ? nullptr : m_service.c_str(),
                          &m_hints, &result);
    if (err) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << m_host << ": " << gai_strerror(err) << '\n';
#endif
        return std::error_code(err, getAddrInfoCategory());
    }
    Finally fin = [&] {
        freeaddrinfo(result);
    };
    ResolveResult res;
    for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
        res.addrs
            .emplace_back(rp->ai_addr, rp->ai_addrlen, rp->ai_family,
                          rp->ai_socktype, rp->ai_protocol)
            .trySetPort(m_port);
    }
    if (res.addrs.empty()) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << m_host << ": no matching host address\n";
#endif
        return std::errc::bad_address;
    }
    res.service = std::move(m_service);
    return res;
}

Expected<SocketAddress> AddressResolver::resolve_one() {
    auto res = resolve_all();
    if (res.has_error()) [[unlikely]] {
        return res.error();
    }
    return res->addrs.front();
}

Expected<SocketAddress> AddressResolver::resolve_one(std::string &service) {
    auto res = resolve_all();
    if (res.has_error()) [[unlikely]] {
        return res.error();
    }
    service = std::move(res->service);
    return res->addrs.front();
}

SocketAddress::SocketAddress(struct sockaddr const *addr, socklen_t addrLen,
                             sa_family_t family, int sockType, int protocol)
    : mSockType(sockType),
      mProtocol(protocol) {
    std::memcpy(&mAddr, addr, addrLen);
    mAddr.ss_family = family;
    mAddrLen = addrLen;
}

std::string SocketAddress::host() const {
    if (family() == AF_INET) {
        auto &sin =
            reinterpret_cast<struct sockaddr_in const &>(mAddr).sin_addr;
        char buf[INET_ADDRSTRLEN] = {};
        inet_ntop(family(), &sin, buf, sizeof(buf));
        return buf;
    } else if (family() == AF_INET6) {
        auto &sin6 =
            reinterpret_cast<struct sockaddr_in6 const &>(mAddr).sin6_addr;
        char buf[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &sin6, buf, sizeof(buf));
        return buf;
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

int SocketAddress::port() const {
    if (family() == AF_INET) {
        auto port =
            reinterpret_cast<struct sockaddr_in const &>(mAddr).sin_port;
        return ntohs(port);
    } else if (family() == AF_INET6) {
        auto port =
            reinterpret_cast<struct sockaddr_in6 const &>(mAddr).sin6_port;
        return ntohs(port);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

void SocketAddress::trySetPort(int port) {
    if (family() == AF_INET) {
        reinterpret_cast<struct sockaddr_in &>(mAddr).sin_port =
            htons(static_cast<uint16_t>(port));
    } else if (family() == AF_INET6) {
        reinterpret_cast<struct sockaddr_in6 &>(mAddr).sin6_port =
            htons(static_cast<uint16_t>(port));
    }
}

String SocketAddress::toString() const {
    return host() + ':' + to_string(port());
}

// void SocketAddress::initFromHostPort(struct in_addr const &host, int port) {
//     struct sockaddr_in saddr = {};
//     saddr.sin_family = AF_INET;
//     std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
//     saddr.sin_port = htons(static_cast<uint16_t>(port));
//     std::memcpy(&mAddrIpv4, &saddr, sizeof(saddr));
//     mAddrLen = sizeof(saddr);
// }
//
// void SocketAddress::initFromHostPort(struct in6_addr const &host, int port) {
//     struct sockaddr_in6 saddr = {};
//     saddr.sin6_family = AF_INET6;
//     std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
//     saddr.sin6_port = htons(static_cast<uint16_t>(port));
//     std::memcpy(&mAddrIpv6, &saddr, sizeof(saddr));
//     mAddrLen = sizeof(saddr);
// }

SocketAddress get_socket_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddr);
    throwingErrorErrno(getsockname(
        sock.fileNo(), reinterpret_cast<struct sockaddr *>(&sa.mAddr),
        &sa.mAddrLen));
    return sa;
}

SocketAddress get_socket_peer_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddr);
    throwingErrorErrno(getpeername(
        sock.fileNo(), reinterpret_cast<struct sockaddr *>(&sa.mAddr),
        &sa.mAddrLen));
    return sa;
}

Task<Expected<SocketHandle>> createSocket(int family, int type, int protocol) {
    int fd = co_await expectError(
                 co_await UringOp().prep_socket(family, type, protocol, 0))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument,
                           [&] { return socket(family, type, protocol); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr) {
    SocketHandle sock = co_await co_await createSocket(
        addr.family(), addr.socktype(), addr.protocol());
    co_await expectError(co_await UringOp().prep_connect(
        sock.fileNo(), reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
        addr.mAddrLen))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout) {
    SocketHandle sock = co_await co_await createSocket(
        addr.family(), addr.socktype(), addr.protocol());
    auto ts = durationToKernelTimespec(timeout);
    co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_connect(
            sock.fileNo(),
            reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
            addr.mAddrLen),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM, 0);
    if (cancel.is_canceled()) [[unlikely]] {
        co_return std::errc::operation_canceled;
    }
    co_await expectError(
        co_await UringOp()
            .prep_connect(
                sock.fileNo(),
                reinterpret_cast<const struct sockaddr *>(&addr.mAddr),
                addr.mAddrLen)
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
                 .or_else(std::errc::invalid_argument, [&] { return connect(sock.fileNo(),
        reinterpret_cast<const struct sockaddr *>(&addr.mAddr), addr.mAddrLen); })
#endif
        ;
    co_return sock;
}

Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM, 0);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEADDR, 1);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEPORT, 1);
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_CORK, 0); */
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_NODELAY, 1); */
    /* co_await socketSetOption(sock, SOL_SOCKET, SO_KEEPALIVE, 1); */
    SocketListener serv(sock.releaseFile());
    co_await expectError(bind(
        serv.fileNo(), reinterpret_cast<struct sockaddr const *>(&addr.mAddr),
        addr.mAddrLen));
    co_await expectError(listen(serv.fileNo(), backlog));
    co_return serv;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener) {
    int fd = co_await expectError(
        co_await UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel) {
    int fd = co_await expectError(
        co_await UringOp()
            .prep_accept(listener.fileNo(), nullptr, nullptr, 0)
            .cancelGuard(cancel));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr) {
    int fd = co_await expectError(co_await UringOp().prep_accept(
        listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
        &peerAddr.mAddrLen, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(accept4(
                         listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel) {
    int fd = co_await expectError(
        co_await UringOp()
            .prep_accept(listener.fileNo(),
                         reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(accept4(
                         listener.fileNo(), reinterpret_cast<struct sockaddr *>(&peerAddr.mAddr),
                         &peerAddr.mAddrLen, 0)); })
#endif
        ;
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_send(sock.fileNo(), buf, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
                                       );
}

Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_send_zc(sock.fileNo(), buf, 0, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
                                       );
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf) {
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp().prep_recv(sock.fileNo(), buf, 0))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf,
                                         CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_send(sock.fileNo(), buf, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>> socket_write_zc(SocketHandle &sock,
                                            std::span<char const> buf,
                                            CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_send_zc(sock.fileNo(), buf, 0, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel) {
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp()
                                 .prep_recv(sock.fileNo(), buf, 0)
                                 .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp::link_ops(
            UringOp().prep_send(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(
        co_await expectError(co_await UringOp::link_ops(
            UringOp().prep_recv(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp::link_ops(
            UringOp().prep_send(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME))
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(send(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
    );
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    auto ts = durationToKernelTimespec(timeout);
    co_return static_cast<std::size_t>(co_await expectError(
        co_await UringOp::link_ops(
            UringOp().prep_recv(sock.fileNo(), buf, 0),
            UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME))
            .cancelGuard(cancel))
#if CO_ASYNC_INVALFIX
        .or_else(std::errc::invalid_argument, [&] { return expectError(recv(sock.fileNo(), buf.data(), buf.size(), 0)); })
#endif
        );
}

Task<Expected<>> socket_shutdown(SocketHandle &sock, int how) {
    co_return expectError(co_await UringOp().prep_shutdown(sock.fileNo(), how));
}
} // namespace co_async

#include <bearssl.h>












namespace co_async {
std::error_category const &bearSSLCategory() {
    static struct final : std::error_category {
        char const *name() const noexcept override {
            return "BearSSL";
        }

        std::string message(int e) const override {
            static std::pair<int, char const *> errors[] = {
                {
                    BR_ERR_OK,
                    "BR_ERR_OK",
                },
                {
                    BR_ERR_BAD_PARAM,
                    "BR_ERR_BAD_PARAM",
                },
                {
                    BR_ERR_BAD_STATE,
                    "BR_ERR_BAD_STATE",
                },
                {
                    BR_ERR_UNSUPPORTED_VERSION,
                    "BR_ERR_UNSUPPORTED_VERSION",
                },
                {
                    BR_ERR_BAD_VERSION,
                    "BR_ERR_BAD_VERSION",
                },
                {
                    BR_ERR_BAD_LENGTH,
                    "BR_ERR_BAD_LENGTH",
                },
                {
                    BR_ERR_TOO_LARGE,
                    "BR_ERR_TOO_LARGE",
                },
                {
                    BR_ERR_BAD_MAC,
                    "BR_ERR_BAD_MAC",
                },
                {
                    BR_ERR_NO_RANDOM,
                    "BR_ERR_NO_RANDOM",
                },
                {
                    BR_ERR_UNKNOWN_TYPE,
                    "BR_ERR_UNKNOWN_TYPE",
                },
                {
                    BR_ERR_UNEXPECTED,
                    "BR_ERR_UNEXPECTED",
                },
                {
                    BR_ERR_BAD_CCS,
                    "BR_ERR_BAD_CCS",
                },
                {
                    BR_ERR_BAD_ALERT,
                    "BR_ERR_BAD_ALERT",
                },
                {
                    BR_ERR_BAD_HANDSHAKE,
                    "BR_ERR_BAD_HANDSHAKE",
                },
                {
                    BR_ERR_OVERSIZED_ID,
                    "BR_ERR_OVERSIZED_ID",
                },
                {
                    BR_ERR_BAD_CIPHER_SUITE,
                    "BR_ERR_BAD_CIPHER_SUITE",
                },
                {
                    BR_ERR_BAD_COMPRESSION,
                    "BR_ERR_BAD_COMPRESSION",
                },
                {
                    BR_ERR_BAD_FRAGLEN,
                    "BR_ERR_BAD_FRAGLEN",
                },
                {
                    BR_ERR_BAD_SECRENEG,
                    "BR_ERR_BAD_SECRENEG",
                },
                {
                    BR_ERR_EXTRA_EXTENSION,
                    "BR_ERR_EXTRA_EXTENSION",
                },
                {
                    BR_ERR_BAD_SNI,
                    "BR_ERR_BAD_SNI",
                },
                {
                    BR_ERR_BAD_HELLO_DONE,
                    "BR_ERR_BAD_HELLO_DONE",
                },
                {
                    BR_ERR_LIMIT_EXCEEDED,
                    "BR_ERR_LIMIT_EXCEEDED",
                },
                {
                    BR_ERR_BAD_FINISHED,
                    "BR_ERR_BAD_FINISHED",
                },
                {
                    BR_ERR_RESUME_MISMATCH,
                    "BR_ERR_RESUME_MISMATCH",
                },
                {
                    BR_ERR_INVALID_ALGORITHM,
                    "BR_ERR_INVALID_ALGORITHM",
                },
                {
                    BR_ERR_BAD_SIGNATURE,
                    "BR_ERR_BAD_SIGNATURE",
                },
                {
                    BR_ERR_WRONG_KEY_USAGE,
                    "BR_ERR_WRONG_KEY_USAGE",
                },
                {
                    BR_ERR_NO_CLIENT_AUTH,
                    "BR_ERR_NO_CLIENT_AUTH",
                },
                {
                    BR_ERR_IO,
                    "BR_ERR_IO",
                },
                {
                    BR_ERR_X509_INVALID_VALUE,
                    "BR_ERR_X509_INVALID_VALUE",
                },
                {
                    BR_ERR_X509_TRUNCATED,
                    "BR_ERR_X509_TRUNCATED",
                },
                {
                    BR_ERR_X509_EMPTY_CHAIN,
                    "BR_ERR_X509_EMPTY_CHAIN",
                },
                {
                    BR_ERR_X509_INNER_TRUNC,
                    "BR_ERR_X509_INNER_TRUNC",
                },
                {
                    BR_ERR_X509_BAD_TAG_CLASS,
                    "BR_ERR_X509_BAD_TAG_CLASS",
                },
                {
                    BR_ERR_X509_BAD_TAG_VALUE,
                    "BR_ERR_X509_BAD_TAG_VALUE",
                },
                {
                    BR_ERR_X509_INDEFINITE_LENGTH,
                    "BR_ERR_X509_INDEFINITE_LENGTH",
                },
                {
                    BR_ERR_X509_EXTRA_ELEMENT,
                    "BR_ERR_X509_EXTRA_ELEMENT",
                },
                {
                    BR_ERR_X509_UNEXPECTED,
                    "BR_ERR_X509_UNEXPECTED",
                },
                {
                    BR_ERR_X509_NOT_CONSTRUCTED,
                    "BR_ERR_X509_NOT_CONSTRUCTED",
                },
                {
                    BR_ERR_X509_NOT_PRIMITIVE,
                    "BR_ERR_X509_NOT_PRIMITIVE",
                },
                {
                    BR_ERR_X509_PARTIAL_BYTE,
                    "BR_ERR_X509_PARTIAL_BYTE",
                },
                {
                    BR_ERR_X509_BAD_BOOLEAN,
                    "BR_ERR_X509_BAD_BOOLEAN",
                },
                {
                    BR_ERR_X509_OVERFLOW,
                    "BR_ERR_X509_OVERFLOW",
                },
                {
                    BR_ERR_X509_BAD_DN,
                    "BR_ERR_X509_BAD_DN",
                },
                {
                    BR_ERR_X509_BAD_TIME,
                    "BR_ERR_X509_BAD_TIME",
                },
                {
                    BR_ERR_X509_UNSUPPORTED,
                    "BR_ERR_X509_UNSUPPORTED",
                },
                {
                    BR_ERR_X509_LIMIT_EXCEEDED,
                    "BR_ERR_X509_LIMIT_EXCEEDED",
                },
                {
                    BR_ERR_X509_WRONG_KEY_TYPE,
                    "BR_ERR_X509_WRONG_KEY_TYPE",
                },
                {
                    BR_ERR_X509_BAD_SIGNATURE,
                    "BR_ERR_X509_BAD_SIGNATURE",
                },
                {
                    BR_ERR_X509_TIME_UNKNOWN,
                    "BR_ERR_X509_TIME_UNKNOWN",
                },
                {
                    BR_ERR_X509_EXPIRED,
                    "BR_ERR_X509_EXPIRED",
                },
                {
                    BR_ERR_X509_DN_MISMATCH,
                    "BR_ERR_X509_DN_MISMATCH",
                },
                {
                    BR_ERR_X509_BAD_SERVER_NAME,
                    "BR_ERR_X509_BAD_SERVER_NAME",
                },
                {
                    BR_ERR_X509_CRITICAL_EXTENSION,
                    "BR_ERR_X509_CRITICAL_EXTENSION",
                },
                {
                    BR_ERR_X509_NOT_CA,
                    "BR_ERR_X509_NOT_CA",
                },
                {
                    BR_ERR_X509_FORBIDDEN_KEY_USAGE,
                    "BR_ERR_X509_FORBIDDEN_KEY_USAGE",
                },
                {
                    BR_ERR_X509_WEAK_PUBLIC_KEY,
                    "BR_ERR_X509_WEAK_PUBLIC_KEY",
                },
                {
                    BR_ERR_X509_NOT_TRUSTED,
                    "BR_ERR_X509_NOT_TRUSTED",
                },
                {0, nullptr},
            };
            std::size_t u;
            for (u = 0; errors[u].second; u++) {
                if (errors[u].first == e) {
                    return errors[u].second;
                }
            }
            return std::to_string(e);
        }
    } instance;

    return instance;
}

namespace {
struct SSLPemDecoder {
private:
    std::unique_ptr<br_pem_decoder_context> pemDec =
        std::make_unique<br_pem_decoder_context>();
    std::string result;
    std::string objName;
    std::vector<std::pair<std::string, std::string>> objs;

    static void pemResultAppender(void *self, void const *buf,
                                  std::size_t len) {
        reinterpret_cast<SSLPemDecoder *>(self)->onResult(
            {reinterpret_cast<char const *>(buf), len});
    }

    void onResult(std::string_view s) {
        result.append(s);
    }

public:
    SSLPemDecoder() {
        br_pem_decoder_init(pemDec.get());
        br_pem_decoder_setdest(pemDec.get(), pemResultAppender, this);
    }

    Expected<> decode(std::string_view s) {
        while (auto n = br_pem_decoder_push(pemDec.get(), s.data(), s.size())) {
            switch (br_pem_decoder_event(pemDec.get())) {
            case BR_PEM_BEGIN_OBJ:
                objName = br_pem_decoder_name(pemDec.get());
                break;
            case BR_PEM_END_OBJ:
                objs.emplace_back(std::move(objName), std::move(result));
                result.clear();
                break;
            case BR_PEM_ERROR:
#if CO_ASYNC_DEBUG
                std::cerr << "PEM decoder error\n";
#endif
                return std::error_code(BR_ERR_X509_INVALID_VALUE, bearSSLCategory());
            }
            s.remove_prefix(n);
        }
        return {};
    }

    std::vector<std::pair<std::string, std::string>> const &objects() const {
        return objs;
    }

    static Expected<std::vector<std::string>> tryDecode(std::string_view s) {
        std::vector<std::string> res;
        if (s.find("-----BEGIN ") != s.npos) {
            SSLPemDecoder dec;
            if (auto e = dec.decode(s); !e) [[unlikely]] {
                return CO_ASYNC_ERROR_FORWARD(e);
            }
            for (auto &[k, v]: dec.objs) {
                res.push_back(std::move(v));
            }
        } else {
            res.emplace_back(s);
        }
        return res;
    }
};

struct SSLX509Decoder {
private:
    std::unique_ptr<br_x509_decoder_context> x509Dec =
        std::make_unique<br_x509_decoder_context>();
    std::string result;

    static void x509ResultAppender(void *self, void const *buf,
                                   std::size_t len) {
        reinterpret_cast<SSLX509Decoder *>(self)->onResult(
            {reinterpret_cast<char const *>(buf), len});
    }

    void onResult(std::string_view s) {
        result.append(s);
    }

public:
    SSLX509Decoder() {
        br_x509_decoder_init(x509Dec.get(), x509ResultAppender, this);
    }

    SSLX509Decoder &decode(std::string_view s) {
        br_x509_decoder_push(x509Dec.get(), s.data(), s.size());
        return *this;
    }

    Expected<std::string_view> getDN() const {
        int err = br_x509_decoder_last_error(x509Dec.get());
        if (err != BR_ERR_OK) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "X509 decoder error: " +
                             bearSSLCategory().message(err) + "\n";
#endif
            return std::error_code(err, bearSSLCategory());
        }
        return result;
    }

    br_x509_pkey *getPubKey() const {
        return br_x509_decoder_get_pkey(x509Dec.get());
    }
};
} // namespace

struct SSLServerPrivateKey {
private:
    br_skey_decoder_context skeyDec;

public:
    SSLServerPrivateKey() {
        br_skey_decoder_init(&skeyDec);
    }

    SSLServerPrivateKey &decodeBinary(std::string_view s) {
        br_skey_decoder_push(&skeyDec, s.data(), s.size());
        return *this;
    }

    Expected<> set(std::string_view pkey) {
        if (auto e = SSLPemDecoder::tryDecode(pkey)) {
            for (auto &s: *e) {
                decodeBinary(s);
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

    br_ec_private_key const *getEC() const {
        return br_skey_decoder_get_ec(&skeyDec);
    }

    br_rsa_private_key const *getRSA() const {
        return br_skey_decoder_get_rsa(&skeyDec);
    }
};

struct SSLClientTrustAnchor {
public:
    std::vector<br_x509_trust_anchor> trustAnchors;

    Expected<> addBinary(std::string_view certX506) {
        auto &x506 = x506Stores.emplace_back();
        x506.decode(certX506);
        auto dn = x506.getDN();
        if (dn.has_error()) {
            return dn.error();
        }
        trustAnchors.push_back({
            {reinterpret_cast<unsigned char *>(const_cast<char *>(dn->data())),
             dn->size()},
            BR_X509_TA_CA,
            *x506.getPubKey(),
        });
        return {};
    }

    Expected<> add(std::string_view certX506) {
        if (auto e = SSLPemDecoder::tryDecode(certX506)) [[likely]] {
            for (auto &s: *e) {
                if (auto e = addBinary(s); !e) [[unlikely]] {
                    return CO_ASYNC_ERROR_FORWARD(e);
                }
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

    bool empty() const {
        return trustAnchors.empty();
    }

    void clear() {
        trustAnchors.clear();
    }

private:
    std::vector<SSLX509Decoder> x506Stores;
};

struct SSLServerCertificate {
public:
    std::vector<br_x509_certificate> certificates;

    void addBinary(std::string certX506) {
        auto &cert = strStores.emplace_back(std::move(certX506));
        certificates.push_back(
            {reinterpret_cast<unsigned char *>(cert.data()), cert.size()});
    }

    Expected<> add(std::string_view certX506) {
        if (auto e = SSLPemDecoder::tryDecode(certX506)) [[likely]] {
            for (auto &s: *e) {
                addBinary(s);
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

private:
    std::vector<String> strStores;
};

struct SSLServerSessionCache {
    std::unique_ptr<unsigned char[]> mLruBuf;
    br_ssl_session_cache_lru mLru;

    explicit SSLServerSessionCache(std::size_t size = 512) {
        mLruBuf = std::make_unique<unsigned char[]>(size);
        br_ssl_session_cache_lru_init(&mLru, mLruBuf.get(), size);
    }
};

namespace {
struct SSLSocketStream : Stream {
private:
    SocketStream raw;
    br_ssl_engine_context *eng = nullptr;
    std::unique_ptr<char[]> iobuf =
        std::make_unique<char[]>(BR_SSL_BUFSIZE_BIDI);

public:
    explicit SSLSocketStream(SocketHandle file) : raw(std::move(file)) {}

protected:
    void setEngine(br_ssl_engine_context *eng_) {
        eng = eng_;
        br_ssl_engine_set_buffer(eng, iobuf.get(), BR_SSL_BUFSIZE_BIDI, 1);
    }

    Task<Expected<>> bearSSLRunUntil(unsigned target) {
        for (;;) {
            unsigned state = br_ssl_engine_current_state(eng);
            if (state & BR_SSL_CLOSED) {
                int err = br_ssl_engine_last_error(eng);
                if (err != BR_ERR_OK) [[unlikely]] {
#if CO_ASYNC_DEBUG
                    std::cerr
                        << "SSL error: " + bearSSLCategory().message(err) +
                               "\n";
#endif
                    co_return std::error_code(err, bearSSLCategory());
                }
                co_return {};
            }
            if (state & BR_SSL_SENDREC) {
                unsigned char *buf;
                std::size_t len, wlen;
                buf = br_ssl_engine_sendrec_buf(eng, &len);
                if (auto e = co_await raw.raw_write(
                        {reinterpret_cast<char const *>(buf), len});
                    e && *e != 0) {
                    wlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return CO_ASYNC_ERROR_FORWARD(e);
                        } else {
                            co_return std::errc::broken_pipe;
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return CO_ASYNC_ERROR_FORWARD(e);
                    } else {
                        co_return {};
                    }
                }
                br_ssl_engine_sendrec_ack(eng, wlen);
                continue;
            }
            if (state & target) {
                co_return {};
            }
            if (state & BR_SSL_RECVAPP) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "SSL write not ready\n";
#endif
                co_return std::errc::broken_pipe;
                // throw std::runtime_error("SSL write not ready");
            }
            if (state & BR_SSL_RECVREC) {
                unsigned char *buf;
                std::size_t len, rlen;
                buf = br_ssl_engine_recvrec_buf(eng, &len);
                if (auto e = co_await raw.raw_read(
                        {reinterpret_cast<char *>(buf), len});
                    e && *e != 0) {
                    rlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return CO_ASYNC_ERROR_FORWARD(e);
                        } else {
                            co_return std::errc::broken_pipe;
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return CO_ASYNC_ERROR_FORWARD(e);
                    } else {
                        co_return {};
                    }
                }
                br_ssl_engine_recvrec_ack(eng, rlen);
                continue;
            }
        }
    }

public:
    Task<Expected<>> raw_flush() override {
        br_ssl_engine_flush(eng, 0);
        co_await co_await bearSSLRunUntil(BR_SSL_SENDAPP | BR_SSL_RECVAPP);
        co_return {};
    }

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        unsigned char *buf;
        std::size_t alen;
        if (buffer.empty()) [[unlikely]] {
            co_return std::size_t(0);
        }
        co_await co_await bearSSLRunUntil(BR_SSL_RECVAPP);
        buf = br_ssl_engine_recvapp_buf(eng, &alen);
        if (alen > buffer.size()) {
            alen = buffer.size();
        }
        memcpy(buffer.data(), buf, alen);
        br_ssl_engine_recvapp_ack(eng, alen);
        co_return alen;
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        unsigned char *buf;
        std::size_t alen;
        if (buffer.empty()) [[unlikely]] {
            co_return std::size_t(0);
        }
        co_await co_await bearSSLRunUntil(BR_SSL_SENDAPP);
        buf = br_ssl_engine_sendapp_buf(eng, &alen);
        if (alen > buffer.size()) {
            alen = buffer.size();
        }
        memcpy(buf, buffer.data(), alen);
        br_ssl_engine_sendapp_ack(eng, alen);
        co_return alen;
    }

    Task<> raw_close() override {
#if CO_ASYNC_DEBUG
        if (br_ssl_engine_current_state(eng) != BR_SSL_CLOSED) [[unlikely]] {
            std::cerr << "SSL closed improperly\n"
                      << br_ssl_engine_current_state(eng);
        }
#endif
        br_ssl_engine_close(eng);
        eng = nullptr;
        co_return;
    }

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        raw.raw_timeout(timeout);
    }
};

struct SSLServerSocketStream : SSLSocketStream {
private:
    std::unique_ptr<br_ssl_server_context> ctx =
        std::make_unique<br_ssl_server_context>();

public:
    explicit SSLServerSocketStream(SocketHandle file,
                                   SSLServerCertificate const &cert,
                                   SSLServerPrivateKey const &pkey,
                                   std::span<char const *const> protocols,
                                   SSLServerSessionCache *cache = nullptr)
        : SSLSocketStream(std::move(file)) {
        if (auto rsa = pkey.getRSA()) {
            br_ssl_server_init_full_rsa(ctx.get(), std::data(cert.certificates),
                                        std::size(cert.certificates), rsa);
        } else if (auto ec = pkey.getEC()) {
            br_ssl_server_init_full_ec(ctx.get(), std::data(cert.certificates),
                                       std::size(cert.certificates),
                                       BR_KEYTYPE_EC, ec);
        } else [[unlikely]] {
            throw std::runtime_error(
                "invalid private key type, must be either RSA or EC");
        }
        setEngine(&ctx->eng);
        if (cache) {
            br_ssl_server_set_cache(ctx.get(), &cache->mLru.vtable);
        }
        br_ssl_engine_set_protocol_names(
            &ctx->eng, const_cast<char const **>(protocols.data()),
            protocols.size());
        br_ssl_server_reset(ctx.get());
    }
};

struct SSLClientSocketStream : SSLSocketStream {
private:
    std::unique_ptr<br_ssl_client_context> ctx =
        std::make_unique<br_ssl_client_context>();
    std::unique_ptr<br_x509_minimal_context> x509Ctx =
        std::make_unique<br_x509_minimal_context>();

public:
    explicit SSLClientSocketStream(SocketHandle file,
                                   SSLClientTrustAnchor const &ta,
                                   char const *host,
                                   std::span<char const *const> protocols)
        : SSLSocketStream(std::move(file)) {
        br_ssl_client_init_full(ctx.get(), x509Ctx.get(),
                                std::data(ta.trustAnchors),
                                std::size(ta.trustAnchors));
        setEngine(&ctx->eng);
        br_ssl_engine_set_protocol_names(
            &ctx->eng, const_cast<char const **>(protocols.data()),
            protocols.size());
        br_ssl_client_reset(ctx.get(), host, 0);
    }

    void ssl_reset(char const *host, bool resume) {
        br_ssl_client_reset(ctx.get(), host, resume);
    }

    std::string ssl_get_selected_protocol() {
        if (auto p = br_ssl_engine_get_selected_protocol(&ctx->eng)) {
            return p;
        }
        return {};
    }
};
} // namespace

Task<Expected<OwningStream>>
ssl_connect(char const *host, int port, SSLClientTrustAnchor const &ta,
            std::span<char const *const> protocols, std::string_view proxy,
            std::chrono::steady_clock::duration timeout) {
    auto conn =
        co_await co_await socket_proxy_connect(host, port, proxy, timeout);
    auto sock = make_stream<SSLClientSocketStream>(std::move(conn), ta, host,
                                                   protocols);
    sock.timeout(timeout);
    co_return sock;
}

OwningStream ssl_accept(SocketHandle file, SSLServerCertificate const &cert,
                        SSLServerPrivateKey const &pkey,
                        std::span<char const *const> protocols,
                        SSLServerSessionCache *cache) {
    return make_stream<SSLServerSocketStream>(std::move(file), cert, pkey,
                                              protocols, cache);
}

DefinePImpl(SSLServerPrivateKey);
DefinePImpl(SSLClientTrustAnchor);
Expected<> ForwardPImplMethod(SSLClientTrustAnchor, add,
                              (std::string_view content), content);
DefinePImpl(SSLServerCertificate);
Expected<> ForwardPImplMethod(SSLServerCertificate, add, (std::string_view content),
                        content);
DefinePImpl(SSLServerSessionCache);
} // namespace co_async








namespace co_async {

namespace {

struct PipeStreamBuffer {
    ConcurrentQueue<std::string> mChunks{64};
};

struct IPipeStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
#if CO_ASYNC_DEBUG
        auto e = co_await mPipe->mChunks.pop();
        if (e.has_error()) {
            std::cerr << "PipeStreamBuffer::pop(): " << e.error().message() << '\n';
            co_return CO_ASYNC_ERROR_FORWARD(e);
        }
        auto chunk = *e;
#else
        auto chunk = co_await co_await mPipe->mChunks.pop();
#endif
        auto n = std::min(buffer.size(), chunk.size());
        std::memcpy(buffer.data(), chunk.data(), n);
        co_return n;
    }

    Task<> raw_close() override {
        mPipe.reset();
        co_return;
    }

    explicit IPipeStream(std::shared_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::shared_ptr<PipeStreamBuffer> mPipe;
};

struct OPipeStream : Stream {
    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        if (auto p = mPipe.lock()) [[likely]] {
            if (buffer.empty()) [[unlikely]] {
                co_return std::size_t(0);
            }
            co_await co_await p->mChunks.push(
                std::string(buffer.data(), buffer.size()));
            co_return buffer.size();
        } else {
            co_return std::errc::broken_pipe;
        }
    }

    Task<> raw_close() override {
        if (auto p = mPipe.lock()) {
            (void)co_await p->mChunks.push(std::string());
        }
        co_return;
    }

    ~OPipeStream() {
        if (auto p = mPipe.lock()) {
            p->mChunks.try_push(std::string());
        }
    }

    explicit OPipeStream(std::weak_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::weak_ptr<PipeStreamBuffer> mPipe;
};

} // namespace

std::array<OwningStream, 2> pipe_stream() {
    auto pipePtr = std::make_shared<PipeStreamBuffer>();
    auto pipeWeakPtr = std::weak_ptr(pipePtr);
    return std::array{make_stream<IPipeStream>(std::move(pipePtr)),
                      make_stream<OPipeStream>(std::move(pipeWeakPtr))};
}

Task<Expected<>> pipe_forward(BorrowedStream &in, BorrowedStream &out) {
    while (true) {
        if (in.bufempty()) {
            if (!co_await in.fillbuf()) {
                break;
            }
        }
        auto n = co_await co_await out.write(in.peekbuf());
        if (n == 0) [[unlikely]] {
            co_return std::errc::broken_pipe;
        }
        in.seenbuf(n);
    }
    co_return {};
}

} // namespace co_async







namespace co_async {
namespace {
struct FileStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFile, buffer, co_await co_cancel);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFile, buffer, co_await co_cancel);
    }

    Task<> raw_close() override {
        (co_await fs_close(std::move(mFile))).value_or();
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit FileStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};
} // namespace

Task<Expected<OwningStream>> file_open(std::filesystem::path path,
                                       OpenMode mode) {
    co_return make_stream<FileStream>(co_await co_await fs_open(path, mode));
}

OwningStream file_from_handle(FileHandle handle) {
    return make_stream<FileStream>(std::move(handle));
}

Task<Expected<String>> file_read(std::filesystem::path path) {
    auto file = co_await co_await file_open(path, OpenMode::Read);
    co_return co_await file.getall();
}

Task<Expected<>> file_write(std::filesystem::path path,
                            std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Write);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

Task<Expected<>> file_append(std::filesystem::path path,
                             std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Append);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}
} // namespace co_async





#include <termios.h>
#include <unistd.h>

namespace co_async {
namespace {
struct StdioStream : Stream {
    explicit StdioStream(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFileIn, buffer, co_await co_cancel);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFileOut, buffer, co_await co_cancel);
    }

    FileHandle &in() const noexcept {
        return mFileIn;
    }

    FileHandle &out() const noexcept {
        return mFileOut;
    }

private:
    FileHandle &mFileIn;
    FileHandle &mFileOut;
};

template <int fileNo>
FileHandle &stdFileHandle() {
    static FileHandle h(fileNo);
    return h;
}

int rawStdinFileHandleImpl() {
    if (isatty(STDIN_FILENO)) {
        struct termios tc;
        tcgetattr(STDIN_FILENO, &tc);
        tc.c_lflag &= ~static_cast<tcflag_t>(ICANON);
        tc.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &tc);
    }
    return STDIN_FILENO;
}

FileHandle &rawStdinFileHandle() {
    static FileHandle h(rawStdinFileHandleImpl());
    return h;
}

} // namespace

OwningStream &stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        stdFileHandle<STDIN_FILENO>(), stdFileHandle<STDOUT_FILENO>());
    return s;
}

OwningStream &raw_stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        rawStdinFileHandle(), stdFileHandle<STDOUT_FILENO>());
    return s;
}
} // namespace co_async






#include <dirent.h>

namespace co_async {
namespace {
struct DirectoryStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_getdents(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit DirectoryStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};
} // namespace

DirectoryWalker::DirectoryWalker(FileHandle file)
    : mStream(make_stream<DirectoryStream>(std::move(file))) {}

DirectoryWalker::~DirectoryWalker() = default;

Task<Expected<String>> DirectoryWalker::DirectoryWalker::next() {
    struct LinuxDirent64 {
        int64_t d_ino;           /* 64-bit inode number */
        int64_t d_off;           /* 64-bit offset to next structure */
        unsigned short d_reclen; /* Size of this dirent */
        unsigned char d_type;    /* File type */
    } dent;

    co_await co_await mStream.getspan(
        std::span<char>(reinterpret_cast<char *>(&dent), 19));
    String rest;
    rest.reserve(dent.d_reclen - 19);
    co_await co_await mStream.getn(rest, dent.d_reclen - 19);
    co_return String(rest.data());
}

Task<Expected<DirectoryWalker>> dir_open(std::filesystem::path path) {
    auto handle = co_await co_await fs_open(path, OpenMode::Directory);
    co_return DirectoryWalker(std::move(handle));
}
} // namespace co_async






#if CO_ASYNC_ZLIB
# include <zlib.h>
#endif

namespace co_async {
#if CO_ASYNC_ZLIB
// borrowed from: https://github.com/intel/zlib/blob/master/examples/zpipe.c
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest) {
    /* decompress */
    int ret;
    unsigned have;
    z_stream strm;
    constexpr std::size_t chunk = kStreamBufferSize;
    unsigned char in[chunk];
    unsigned char out[chunk];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: inflateInit returned error\n";
# endif
        co_return std::errc::not_enough_memory;
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: inflate source read failed with error\n";
# endif
            (void)inflateEnd(&strm);
            co_return CO_ASYNC_ERROR_FORWARD(e);
        } else {
            strm.avail_in = *e;
        }
        if (strm.avail_in == 0) {
            break;
        }
        strm.next_in = in;
        do {
            strm.avail_out = chunk;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:  [[fallthrough]];
            case Z_DATA_ERROR: [[fallthrough]];
            case Z_MEM_ERROR:  (void)inflateEnd(&strm);
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: inflate error: " + std::to_string(ret) +
                                 ": " + std::string(strm.msg) + "\n";
# endif
                co_return std::errc::io_error;
            }
            have = chunk - strm.avail_out;
            if (auto e = co_await dest.putspan(
                    std::span<char const>((char const *)out, have));
                e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: inflate dest write failed with error\n";
# endif
                (void)inflateEnd(&strm);
                co_return CO_ASYNC_ERROR_FORWARD(e);
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: inflate got unexpected end of file\n";
# endif
        co_return std::errc::io_error;
    }
    co_return {};
}

Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest) {
    /* compress */
    int ret, flush;
    unsigned have;
    z_stream strm;
    constexpr std::size_t chunk = kStreamBufferSize;
    unsigned char in[chunk];
    unsigned char out[chunk];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: deflateInit returned error\n";
# endif
        co_return std::errc::not_enough_memory;
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: deflate source read failed with error\n";
# endif
            (void)deflateEnd(&strm);
            co_return CO_ASYNC_ERROR_FORWARD(e);
        } else {
            strm.avail_in = *e;
        }
        if (strm.avail_in == 0) {
            flush = Z_FINISH;
        } else {
            flush = Z_NO_FLUSH;
        }
        strm.next_in = in;
        do {
            strm.avail_out = chunk;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            have = chunk - strm.avail_out;
            if (auto e = co_await dest.putspan(
                    std::span<char const>((char const *)out, have));
                e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: deflate dest write failed with error\n";
# endif
                (void)deflateEnd(&strm);
                co_return CO_ASYNC_ERROR_FORWARD(e);
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */
    } while (flush != Z_FINISH);
    (void)deflateEnd(&strm);
    co_return {};
}
#else
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return std::errc::function_not_supported;
}

Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return std::errc::function_not_supported;
}
#endif
} // namespace co_async





namespace co_async {
namespace {
std::uint8_t fromHex(char c) {
    if ('0' <= c && c <= '9') {
        return static_cast<std::uint8_t>(c - '0');
    } else if ('A' <= c && c <= 'F') {
        return static_cast<std::uint8_t>(c - 'A' + 10);
    } else [[unlikely]] {
        return 0;
    }
}

bool isCharUrlSafe(char c) {
    if ('0' <= c && c <= '9') {
        return true;
    }
    if ('a' <= c && c <= 'z') {
        return true;
    }
    if ('A' <= c && c <= 'Z') {
        return true;
    }
    if (c == '-' || c == '_' || c == '.') {
        return true;
    }
    return false;
}
} // namespace

void URI::url_decode(String &r, std::string_view s) {
    std::size_t b = 0;
    while (true) {
        auto i = s.find('%', b);
        if (i == std::string_view::npos || i + 3 > s.size()) {
            r.append(s.data() + b, s.data() + s.size());
            break;
        }
        r.append(s.data() + b, s.data() + i);
        char c1 = s[i + 1];
        char c2 = s[i + 2];
        r.push_back(static_cast<char>((fromHex(c1) << 4) | fromHex(c2)));
        b = i + 3;
    }
}

String URI::url_decode(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_decode(r, s);
    return r;
}

void URI::url_encode(String &r, std::string_view s) {
    static constexpr char lut[] = "0123456789ABCDEF";
    for (char c: s) {
        if (isCharUrlSafe(c)) {
            r.push_back(c);
        } else {
            r.push_back('%');
            r.push_back(lut[static_cast<std::uint8_t>(c) >> 4]);
            r.push_back(lut[static_cast<std::uint8_t>(c) & 0xF]);
        }
    }
}

String URI::url_encode(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_encode(r, s);
    return r;
}

void URI::url_encode_path(String &r, std::string_view s) {
    static constexpr char lut[] = "0123456789ABCDEF";
    for (char c: s) {
        if (isCharUrlSafe(c) || c == '/') {
            r.push_back(c);
        } else {
            r.push_back('%');
            r.push_back(lut[static_cast<std::uint8_t>(c) >> 4]);
            r.push_back(lut[static_cast<std::uint8_t>(c) & 0xF]);
        }
    }
}

String URI::url_encode_path(std::string_view s) {
    String r;
    r.reserve(s.size());
    url_encode_path(r, s);
    return r;
}

URI URI::parse(std::string_view uri) {
    auto path = uri;
    URIParams params;
    if (auto i = uri.find('?'); i != std::string_view::npos) {
        path = uri.substr(0, i);
        do {
            uri.remove_prefix(i + 1);
            i = uri.find('&');
            auto pair = uri.substr(0, i);
            auto m = pair.find('=');
            if (m != std::string_view::npos) {
                auto k = pair.substr(0, m);
                auto v = pair.substr(m + 1);
                params.insert_or_assign(String(k), url_decode(v));
            }
        } while (i != std::string_view::npos);
    }
    String spath(path);
    if (spath.empty() || spath.front() != '/') [[unlikely]] {
        spath.insert(spath.begin(), '/');
    }
    return URI{spath, std::move(params)};
}

void URI::dump(String &r) const {
    r.append(path);
    char queryChar = '?';
    for (auto &[k, v]: params) {
        r.push_back(queryChar);
        url_encode(r, k);
        r.push_back('=');
        url_encode(r, v);
        queryChar = '&';
    }
}

String URI::dump() const {
    String r;
    dump(r);
    return r;
}
} // namespace co_async















namespace co_async {
HTTPContentEncoding
HTTPProtocolVersion11::httpContentEncodingByName(std::string_view name) {
    using namespace std::string_view_literals;
    static constexpr std::pair<std::string_view, HTTPContentEncoding>
        encodings[] = {
            {"gzip"sv, HTTPContentEncoding::Gzip},
            {"deflate"sv, HTTPContentEncoding::Deflate},
            {"identity"sv, HTTPContentEncoding::Identity},
        };
    for (auto const &[k, v]: encodings) {
        if (name == k) {
            return v;
        }
    }
    return HTTPContentEncoding::Identity;
}

Task<Expected<>> HTTPProtocolVersion11::parseHeaders(HTTPHeaders &headers) {
    using namespace std::string_view_literals;
    String line;
    while (true) {
        line.clear();
        co_await co_await sock.getline(line, "\r\n"sv);
        if (line.empty()) {
            break;
        }
        auto pos = line.find(':');
        if (pos == line.npos || pos == line.size() - 1 || line[pos + 1] != ' ')
            [[unlikely]] {
            co_return std::errc::protocol_error;
        }
        auto key = line.substr(0, pos);
        for (auto &c: key) {
            if (c >= 'A' && c <= 'Z') {
                c += 'a' - 'A';
            }
        }
        headers.insert_or_assign(std::move(key), line.substr(pos + 2));
    }
    headers.erase("connection"sv);
    co_return {};
}

Task<Expected<>>
HTTPProtocolVersion11::dumpHeaders(HTTPHeaders const &headers) {
    using namespace std::string_view_literals;
    for (auto const &[k, v]: headers) {
        co_await co_await sock.puts(k);
        co_await co_await sock.puts(": "sv);
        co_await co_await sock.puts(v);
        co_await co_await sock.puts("\r\n"sv);
    }
    // co_await co_await sock.puts("connection: keep-alive\r\n"sv);
    co_return {};
}

void HTTPProtocolVersion11::handleContentEncoding(HTTPHeaders &headers) {
    using namespace std::string_view_literals;
    mContentEncoding = HTTPContentEncoding::Identity;
    mContentLength = std::nullopt;
    bool needLength = true;
    if (auto transEnc = headers.get("transfer-encoding"sv)) {
        if (*transEnc == "chunked") [[likely]] {
            needLength = false;
        }
        headers.erase("transfer-encoding"sv);
    }
    if (auto contEnc = headers.get("content-encoding"sv)) {
        mContentEncoding = httpContentEncodingByName(*contEnc);
        headers.erase("content-encoding"sv);
    }
    if (needLength) {
        mContentLength =
            headers.get("content-length"sv, from_string<std::size_t>)
                .value_or(0);
        headers.erase("content-length"sv);
    }
}

void HTTPProtocolVersion11::handleAcceptEncoding(HTTPHeaders &headers) {
    using namespace std::string_view_literals;
    if (auto acceptEnc = headers.get("accept-encoding"sv)) {
        mAcceptEncoding = std::move(*acceptEnc);
        headers.erase("accept-encoding"sv);
    } else {
        mAcceptEncoding.clear();
    }
}

void HTTPProtocolVersion11::negotiateAcceptEncoding(
    HTTPHeaders &headers, std::span<HTTPContentEncoding const> encodings) {
    using namespace std::string_view_literals;
    mContentEncoding = HTTPContentEncoding::Identity;
    if (!mAcceptEncoding.empty()) {
        for (std::string_view encName: split_string(mAcceptEncoding, ", "sv)) {
            if (auto i = encName.find(';'); i != encName.npos) {
                encName = encName.substr(0, i);
            }
            auto enc = httpContentEncodingByName(encName);
            if (enc != HTTPContentEncoding::Identity) [[likely]] {
                if (std::find(encodings.begin(), encodings.end(), enc) !=
                    encodings.end()) {
                    mContentEncoding = enc;
                    break;
                }
            }
        }
    }
}

Task<Expected<>> HTTPProtocolVersion11::writeChunked(BorrowedStream &body) {
    using namespace std::string_view_literals;
    co_await co_await sock.puts("transfer-encoding: chunked\r\n\r\n"sv);
    do {
        auto bufSpan = body.peekbuf();
        auto n = bufSpan.size();
        /* debug(), std::string_view(bufSpan.data(), n); */
        if (n > 0) {
            char buf[sizeof(n) * 2 + 2] = {}, *ep = buf;
            do {
                *ep++ = "0123456789ABCDEF"[n & 15];
            } while (n >>= 4);
            std::reverse(buf, ep);
            *ep++ = '\r';
            *ep++ = '\n';
            co_await co_await sock.puts(
                std::string_view{buf, static_cast<std::size_t>(ep - buf)});
            co_await co_await sock.putspan(bufSpan);
            co_await co_await sock.puts("\r\n"sv);
            co_await co_await sock.flush();
        }
    } while (co_await body.fillbuf());
    co_await co_await sock.puts("0\r\n\r\n"sv);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>>
HTTPProtocolVersion11::writeChunkedString(std::string_view body) {
    using namespace std::string_view_literals;
    if (body.empty()) {
        co_await co_await sock.puts("\r\n"sv);
    } else {
        co_await co_await sock.puts("content-length: "sv);
        co_await co_await sock.puts(to_string(body.size()));
        co_await co_await sock.puts("\r\n\r\n"sv);
        co_await co_await sock.puts(body);
    }
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readChunked(BorrowedStream &body) {
    using namespace std::string_view_literals;
    if (mContentLength) {
        if (auto n = *mContentLength; n > 0) {
            String line;
            co_await co_await sock.getn(line, n);
            co_await co_await body.puts(line);
            co_await co_await body.flush();
        }
    } else {
        String line;
        while (true) {
            line.clear();
            co_await co_await sock.getline(line, "\r\n"sv);
            std::size_t n = std::strtoull(line.c_str(), nullptr, 16);
            if (n <= 0) {
                co_await co_await sock.dropn(2);
                break;
            }
            line.clear();
            co_await co_await sock.getn(line, n);
            co_await co_await body.puts(line);
            co_await co_await body.flush();
            co_await co_await sock.dropn(2);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readChunkedString(String &body) {
    using namespace std::string_view_literals;
    if (mContentLength) {
        if (auto n = *mContentLength; n > 0) {
            co_await co_await sock.getn(body, n);
        }
    } else {
        String line;
        while (true) {
            line.clear();
            co_await co_await sock.getline(line, "\r\n"sv);
            auto n = std::strtoul(line.c_str(), nullptr, 16);
            if (n == 0) {
                break;
            }
            co_await co_await sock.getn(body, n);
            co_await co_await sock.dropn(2);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeEncoded(BorrowedStream &body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await writeChunked(body);
    } break;
    case HTTPContentEncoding::Deflate: {
        co_await co_await sock.puts("content-encoding: deflate\r\n"sv);
        auto [r, w] = pipe_stream();
        co_await co_await when_all(
            co_bind([&body, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await zlib_deflate(body, w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            }),
            writeChunked(r));
    } break;
    case HTTPContentEncoding::Gzip: {
        co_await co_await sock.puts("content-encoding: gzip\r\n"sv);
        OwningStream pin, pout;
        auto pid = co_await co_await ProcessBuilder()
                       .path("gzip"sv)
                       .arg("-"sv)
                       .pipe_in(0, pin)
                       .pipe_out(1, pout)
                       .spawn();
        co_await co_await when_all(pipe_forward(body, pin), writeChunked(pout));
        co_await co_await wait_process(pid);
    } break;
    };
    co_return {};
}

Task<Expected<>>
HTTPProtocolVersion11::writeEncodedString(std::string_view body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await writeChunkedString(body);
    } break;
    default: {
        auto is = make_stream<IStringStream>(body);
        co_await co_await writeEncoded(is);
    } break;
    };
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readEncoded(BorrowedStream &body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await readChunked(body);
    } break;
    case HTTPContentEncoding::Deflate: {
        auto [r, w] = pipe_stream();
        co_await co_await when_all(
            pipe_bind(std::move(w), &std::decay_t<decltype(*this)>::readChunked,
                      this),
            co_bind([this, w = std::move(w)]() mutable -> Task<Expected<>> {
                co_await co_await readChunked(w);
                co_await co_await w.flush();
                co_await w.close();
                co_return {};
            }),
            zlib_deflate(r, body));
    } break;
    case HTTPContentEncoding::Gzip: {
        OwningStream pin, pout;
        auto pid = co_await co_await ProcessBuilder()
                       .path("gzip"sv)
                       .arg("-d"sv)
                       .arg("-"sv)
                       .pipe_in(0, pin)
                       .pipe_out(1, pout)
                       .spawn();
        co_await co_await when_all(
            co_bind([this, pin = std::move(pin)]() mutable -> Task<Expected<>> {
                co_await co_await readChunked(pin);
                co_await co_await pin.flush();
                co_await pin.close();
                co_return {};
            }),
            pipe_forward(pout, body));
        co_await co_await wait_process(pid);
    } break;
    };
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readEncodedString(String &body) {
    using namespace std::string_view_literals;
    switch (mContentEncoding) {
    case HTTPContentEncoding::Identity: {
        co_await co_await readChunkedString(body);
    } break;
    default: {
        auto os = make_stream<OStringStream>(body);
        co_await co_await readEncoded(os);
    } break;
    };
    co_return {};
}
#if CO_ASYNC_DEBUG
void HTTPProtocolVersion11::checkPhase(int from, int to) {
    // debug(), from, to, this;
    if (mPhase != from) [[unlikely]] {
        throw std::logic_error(
            "HTTPProtocol member function calling order wrong (phase = " +
            std::to_string(mPhase) + ", from = " + std::to_string(from) +
            ", to = " + std::to_string(to) + ")");
    }
    mPhase = to;
}
#else
void HTTPProtocolVersion11::checkPhase(int from, int to) {}
#endif
Task<Expected<>> HTTPProtocolVersion11::writeBodyStream(BorrowedStream &body) {
    checkPhase(1, 0);
    co_await co_await writeEncoded(body);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeBody(std::string_view body) {
    checkPhase(1, 0);
    co_await co_await writeEncodedString(body);
    co_await co_await sock.flush();
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readBodyStream(BorrowedStream &body) {
    checkPhase(-1, 0);
    co_await co_await readEncoded(body);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readBody(String &body) {
    checkPhase(-1, 0);
    co_await co_await readEncodedString(body);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeRequest(HTTPRequest const &req) {
    checkPhase(0, 1);
    using namespace std::string_view_literals;
    co_await co_await sock.puts(req.method);
    co_await co_await sock.putchar(' ');
    co_await co_await sock.puts(req.uri.dump());
    co_await co_await sock.puts(" HTTP/1.1\r\n"sv);
    co_await co_await dumpHeaders(req.headers);
    mContentEncoding = HTTPContentEncoding::Identity;
    co_return {};
}

void HTTPProtocolVersion11::initServerState() {
#if CO_ASYNC_DEBUG
    mPhase = 0;
#endif
}

void HTTPProtocolVersion11::initClientState() {
#if CO_ASYNC_DEBUG
    mPhase = 0;
#endif
}

Task<Expected<>> HTTPProtocolVersion11::readRequest(HTTPRequest &req) {
    checkPhase(0, -1);
    using namespace std::string_view_literals;
    String line;
    co_await co_await sock.getline(line, "\r\n"sv);
    auto pos = line.find(' ');
    if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    req.method = line.substr(0, pos);
    auto pos2 = line.find(' ', pos + 1);
    if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    req.uri = URI::parse(line.substr(pos + 1, pos2 - pos - 1));
    co_await co_await parseHeaders(req.headers);
    handleContentEncoding(req.headers);
    handleAcceptEncoding(req.headers);
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::writeResponse(HTTPResponse const &res) {
    checkPhase(0, 1);
    using namespace std::string_view_literals;
    co_await co_await sock.puts("HTTP/1.1 "sv);
    co_await co_await sock.puts(to_string(res.status));
    co_await co_await sock.putchar(' ');
    co_await co_await sock.puts(getHTTPStatusName(res.status));
    co_await co_await sock.puts("\r\n"sv);
    co_await co_await dumpHeaders(res.headers);
    mContentEncoding = HTTPContentEncoding::Identity;
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readResponse(HTTPResponse &res) {
    checkPhase(0, -1);
    using namespace std::string_view_literals;
    String line;
    co_await co_await sock.getline(line, "\r\n"sv);
    if (line.size() <= 9 || line.substr(0, 7) != "HTTP/1."sv || line[8] != ' ')
        [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
        res.status = *statusOpt;
    } else [[unlikely]] {
        co_return std::errc::protocol_error;
    }
    co_await co_await parseHeaders(res.headers);
    handleContentEncoding(res.headers);
    co_return {};
}

HTTPProtocolVersion11::HTTPProtocolVersion11(OwningStream sock)
    : HTTPProtocol(std::move(sock)) {}

HTTPProtocolVersion11::~HTTPProtocolVersion11() = default;
} // namespace co_async












namespace co_async {
String HTTPServerUtils::html_encode(std::string_view str) {
    String res;
    res.reserve(str.size());
    for (auto c: str) {
        switch (c) {
        case '&':  res.append("&amp;"); break;
        case '"':  res.append("&quot;"); break;
        case '\'': res.append("&apos;"); break;
        case '<':  res.append("&lt;"); break;
        case '>':  res.append("&gt;"); break;
        default:   res.push_back(c);
        }
    }
    return res;
}

Task<Expected<>> HTTPServerUtils::make_ok_response(HTTPServer::IO &io,
                                                   std::string_view body,
                                                   String contentType) {
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type", std::move(contentType)},
            },
    };
    co_await co_await io.response(res, body);
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_directory(HTTPServer::IO &io,
                                              std::filesystem::path path) {
    String dirPath{path.generic_string()};
    String content = "<h1>Files in " + dirPath + ":</h1>";
    auto parentPath = path.parent_path().generic_string();
    content +=
        "<a href=\"/" + URI::url_encode_path(parentPath) + "\">..</a><br>";
    auto dir = co_await co_await dir_open(path);
    while (auto entry = co_await dir.next()) {
        if (*entry == ".." || *entry == ".") {
            continue;
        }
        content +=
            "<a href=\"/" +
            URI::url_encode_path(make_path(dirPath, *entry).generic_string()) +
            "\">" + html_encode(*entry) + "</a><br>";
    }
    co_await co_await make_ok_response(io, content);
    co_return {};
}

Task<Expected<>> HTTPServerUtils::make_error_response(HTTPServer::IO &io,
                                                      int status) {
    auto error = to_string(status) + ' ' + String(getHTTPStatusName(status));
    HTTPResponse res{
        .status = status,
        .headers =
            {
                {"content-type", "text/html;charset=utf-8"},
            },
    };
    co_await co_await io.response(
        res, "<html><head><title>" + error +
                 "</title></head><body><center><h1>" + error +
                 "</h1></center><hr><center>co_async</center></body></html>");
    co_return {};
}

Task<Expected<>> HTTPServerUtils::make_response_from_file_or_directory(
    HTTPServer::IO &io, std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    if (stat->is_directory()) {
        co_return co_await make_response_from_directory(io, std::move(path));
    }
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_path(HTTPServer::IO &io,
                                         std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    if (stat->is_directory()) {
        co_return co_await make_response_from_directory(io, path);
    }
    /* if (stat->is_executable()) { */
    /*     co_return co_await make_response_from_cgi_script(io, path); */
    /* } */
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}

Task<Expected<>>
HTTPServerUtils::make_response_from_file(HTTPServer::IO &io,
                                         std::filesystem::path path) {
    auto stat = co_await fs_stat(path, STATX_MODE);
    if (!stat || stat->is_directory()) [[unlikely]] {
        co_return co_await make_error_response(io, 404);
    }
    if (!stat->is_readable()) [[unlikely]] {
        co_return co_await make_error_response(io, 403);
    }
    HTTPResponse res{
        .status = 200,
        .headers =
            {
                {"content-type",
                 guessContentTypeByExtension(path.extension().string())},
            },
    };
    auto f = co_await co_await file_open(path, OpenMode::Read);
    co_await co_await io.response(res, f);
    co_await f.close();
    co_return {};
}
} // namespace co_async





namespace co_async {
String timePointToHTTPDate(std::chrono::system_clock::time_point tp) {
    // format chrono time point into HTTP date format, e.g.:
    // Tue, 30 Apr 2024 07:31:38 GMT
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return String{ss.str()};
}

Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(String const &date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss.imbue(std::locale::classic());
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) [[unlikely]] {
        return std::errc::invalid_argument;
    }
    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

String httpDateNow() {
    std::time_t time = std::time(nullptr);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return String{ss.str()};
}

std::string_view getHTTPStatusName(int status) {
    using namespace std::string_view_literals;
    static constexpr std::pair<int, std::string_view> lut[] = {
        {100, "Continue"sv},
        {101, "Switching Protocols"sv},
        {102, "Processing"sv},
        {200, "OK"sv},
        {201, "Created"sv},
        {202, "Accepted"sv},
        {203, "Non-Authoritative Information"sv},
        {204, "No Content"sv},
        {205, "Reset Content"sv},
        {206, "Partial Content"sv},
        {207, "Multi-Status"sv},
        {208, "Already Reported"sv},
        {226, "IM Used"sv},
        {300, "Multiple Choices"sv},
        {301, "Moved Permanently"sv},
        {302, "Found"sv},
        {303, "See Other"sv},
        {304, "Not Modified"sv},
        {305, "Use Proxy"sv},
        {306, "Switch Proxy"sv},
        {307, "Temporary Redirect"sv},
        {308, "Permanent Redirect"sv},
        {400, "Bad Request"sv},
        {401, "Unauthorized"sv},
        {402, "Payment Required"sv},
        {403, "Forbidden"sv},
        {404, "Not Found"sv},
        {405, "Method Not Allowed"sv},
        {406, "Not Acceptable"sv},
        {407, "Proxy Authentication Required"sv},
        {408, "Request Timeout"sv},
        {409, "Conflict"sv},
        {410, "Gone"sv},
        {411, "Length Required"sv},
        {412, "Precondition Failed"sv},
        {413, "Payload Too Large"sv},
        {414, "URI Too Long"sv},
        {415, "Unsupported Media Type"sv},
        {416, "Range Not Satisfiable"sv},
        {417, "Expectation Failed"sv},
        {418, "I'm a teapot"sv},
        {421, "Misdirected Request"sv},
        {422, "Unprocessable Entity"sv},
        {423, "Locked"sv},
        {424, "Failed Dependency"sv},
        {426, "Upgrade Required"sv},
        {428, "Precondition Required"sv},
        {429, "Too Many Requests"sv},
        {431, "Request Header Fields Too Large"sv},
        {451, "Unavailable For Legal Reasons"sv},
        {500, "Internal Server Error"sv},
        {501, "Not Implemented"sv},
        {502, "Bad Gateway"sv},
        {503, "Service Unavailable"sv},
        {504, "Gateway Timeout"sv},
        {505, "HTTP Version Not Supported"sv},
        {506, "Variant Also Negotiates"sv},
        {507, "Insufficient Storage"sv},
        {508, "Loop Detected"sv},
        {510, "Not Extended"sv},
        {511, "Network Authentication Required"sv},
    };
    if (status == 200) {
        return "OK"sv;
    }
    auto it = std::lower_bound(
        std::begin(lut), std::end(lut), status,
        [](auto const &p, auto status) { return p.first < status; });
    if (it == std::end(lut) || it->first != status) [[unlikely]] {
        return "Unknown"sv;
    } else {
        return it->second;
    }
}

String guessContentTypeByExtension(std::string_view ext,
                                   char const *defaultType) {
    using namespace std::string_view_literals;
    if (ext == ".html"sv || ext == ".htm"sv) {
        return String{"text/html;charset=utf-8"sv};
    } else if (ext == ".css"sv) {
        return String{"text/css;charset=utf-8"sv};
    } else if (ext == ".js"sv) {
        return String{"application/javascript;charset=utf-8"sv};
    } else if (ext == ".txt"sv || ext == ".md"sv) {
        return String{"text/plain;charset=utf-8"sv};
    } else if (ext == ".json"sv) {
        return String{"application/json"sv};
    } else if (ext == ".png"sv) {
        return String{"image/png"sv};
    } else if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return String{"image/jpeg"sv};
    } else if (ext == ".gif"sv) {
        return String{"image/gif"sv};
    } else if (ext == ".xml"sv) {
        return String{"application/xml"sv};
    } else if (ext == ".pdf"sv) {
        return String{"application/pdf"sv};
    } else if (ext == ".mp4"sv) {
        return String{"video/mp4"sv};
    } else if (ext == ".mp3"sv) {
        return String{"audio/mp3"sv};
    } else if (ext == ".zip"sv) {
        return String{"application/zip"sv};
    } else if (ext == ".svg"sv) {
        return String{"image/svg+xml"sv};
    } else if (ext == ".wav"sv) {
        return String{"audio/wav"sv};
    } else if (ext == ".ogg"sv) {
        return String{"audio/ogg"sv};
    } else if (ext == ".mpg"sv || ext == ".mpeg"sv) {
        return String{"video/mpeg"sv};
    } else if (ext == ".webm"sv) {
        return String{"video/webm"sv};
    } else if (ext == ".ico"sv) {
        return String{"image/x-icon"sv};
    } else if (ext == ".rar"sv) {
        return String{"application/x-rar-compressed"sv};
    } else if (ext == ".7z"sv) {
        return String{"application/x-7z-compressed"sv};
    } else if (ext == ".tar"sv) {
        return String{"application/x-tar"sv};
    } else if (ext == ".gz"sv) {
        return String{"application/gzip"sv};
    } else if (ext == ".bz2"sv) {
        return String{"application/x-bzip2"sv};
    } else if (ext == ".xz"sv) {
        return String{"application/x-xz"sv};
    } else if (ext == ".zip"sv) {
        return String{"application/zip"sv};
    } else if (ext == ".tar.gz"sv || ext == ".tgz"sv) {
        return String{"application/tar+gzip"sv};
    } else if (ext == ".tar.bz2"sv || ext == ".tbz2"sv) {
        return String{"application/tar+bzip2"sv};
    } else if (ext == ".tar.xz"sv || ext == ".txz"sv) {
        return String{"application/tar+xz"sv};
    } else if (ext == ".doc"sv || ext == ".docx"sv) {
        return String{"application/msword"sv};
    } else if (ext == ".xls"sv || ext == ".xlsx"sv) {
        return String{"application/vnd.ms-excel"sv};
    } else if (ext == ".ppt"sv || ext == ".pptx"sv) {
        return String{"application/vnd.ms-powerpoint"sv};
    } else if (ext == ".csv"sv) {
        return String{"text/csv;charset=utf-8"sv};
    } else if (ext == ".rtf"sv) {
        return String{"application/rtf"sv};
    } else if (ext == ".exe"sv) {
        return String{"application/x-msdownload"sv};
    } else if (ext == ".msi"sv) {
        return String{"application/x-msi"sv};
    } else if (ext == ".bin"sv) {
        return String{"application/octet-stream"sv};
    } else {
        return String{defaultType};
    }
}

String capitalizeHTTPHeader(std::string_view key) {
    // e.g.: user-agent -> User-Agent
    String result(key);
    if (!result.empty()) [[likely]] {
        if ('a' <= result[0] && result[0] <= 'z') [[likely]] {
            result[0] -= 'a' - 'A';
        }
        for (std::size_t i = 1; i < result.size(); ++i) {
            if (result[i - 1] == '-' && 'a' <= result[i] && result[i] <= 'z')
                [[likely]] {
                result[i] -= 'a' - 'A';
            }
        }
    }
    return result;
}
} // namespace co_async















namespace co_async {
struct HTTPServer::Impl {
    struct Route {
        HTTPHandler mHandler;
        std::vector<String> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }
    };

    struct PrefixRoute {
        HTTPPrefixHandler mHandler;
        HTTPRouteMode mRouteMode;
        std::vector<String> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }

        bool checkSuffix(std::string_view &suffix) const {
            switch (mRouteMode) {
            case HTTPRouteMode::SuffixName: {
                if (suffix.starts_with('/')) {
                    suffix.remove_prefix(1);
                }
                if (suffix.empty()) [[unlikely]] {
                    return false;
                }
                // make sure no '/' in suffix
                if (suffix.find('/') != std::string_view::npos) [[unlikely]] {
                    return false;
                }
                return true;
            }
            case HTTPRouteMode::SuffixPath: {
                if (suffix.starts_with('/')) {
                    suffix.remove_prefix(1);
                }
                if (suffix.empty()) {
                    return true;
                }
                // make sure no ".." or "." after spliting by '/'
                for (auto const &part: split_string(suffix, '/')) {
                    switch (part.size()) {
                    case 2:
                        if (part[0] == '.' && part[1] == '.') [[unlikely]] {
                            return false;
                        }
                        break;
                    case 1:
                        if (part[0] == '.') [[unlikely]] {
                            return false;
                        }
                        break;
                    case 0: return false;
                    }
                }
                return true;
            }
            default: return true;
            }
        }
    };

    SimpleMap<String, Route> mRoutes;
    std::vector<std::pair<String, PrefixRoute>> mPrefixRoutes;
    HTTPHandler mDefaultRoute = [](IO &io) -> Task<Expected<>> {
        co_return co_await make_error_response(io, 404);
    };
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(30);
#if CO_ASYNC_DEBUG
    bool mLogRequests = false;
#endif

    Task<Expected<>> doHandleRequest(IO &io) const {
        if (auto route = mRoutes.at(io.request.uri.path)) {
            if (!route->checkMethod(io.request.method)) [[unlikely]] {
                co_await co_await make_error_response(io, 405);
                co_return {};
            }
            co_await co_await route->mHandler(io);
            co_return {};
        }
        for (auto const &[prefix, route]: mPrefixRoutes) {
            if (io.request.uri.path.starts_with(prefix)) {
                if (!route.checkMethod(io.request.method)) [[unlikely]] {
                    co_await co_await make_error_response(io, 405);
                    co_return {};
                }
                auto suffix = std::string_view(io.request.uri.path);
                suffix.remove_prefix(prefix.size());
                if (!route.checkSuffix(suffix)) [[unlikely]] {
                    co_await co_await make_error_response(io, 405);
                    co_return {};
                }
#if CO_ASYNC_DEBUG
                auto ret = co_await route.mHandler(io, suffix);
                if (ret.has_error() && mLogRequests) {
                    std::clog << "SERVER ERROR: " << ret.error() << '\n';
                }
                co_return ret;
#else
                co_await co_await route.mHandler(io, suffix);
                co_return {};
#endif
            }
        }
        co_await co_await mDefaultRoute(io);
        co_return {};
    }
};

Task<Expected<bool>> HTTPServer::IO::readRequestHeader() {
    mHttp->initServerState();
    co_return co_await (co_await mHttp->readRequest(request)).transform([] { return true; }).or_else(eofError(), [] { return false; });
}

Task<Expected<String>> HTTPServer::IO::request_body() {
#if CO_ASYNC_DEBUG
    if (mBodyRead) [[unlikely]] {
        throw std::runtime_error("request_body() may only be called once");
    }
#endif
    mBodyRead = true;
    String body;
    co_await co_await mHttp->readBody(body);
    co_return body;
}

Task<Expected<>> HTTPServer::IO::request_body_stream(OwningStream &out) {
#if CO_ASYNC_DEBUG
    if (mBodyRead) [[unlikely]] {
        throw std::runtime_error("request_body() may only be called once");
    }
#endif
    mBodyRead = true;
    co_await co_await mHttp->readBodyStream(out);
    co_return {};
}

Task<Expected<>> HTTPServer::IO::response(HTTPResponse resp,
                                          std::string_view content) {
#if CO_ASYNC_DEBUG
    mResponseSavedForDebug = resp;
#endif
    if (!mBodyRead) {
        co_await co_await request_body();
    }
    builtinHeaders(resp);
    co_await co_await mHttp->writeResponse(resp);
    co_await co_await mHttp->writeBody(content);
    mBodyRead = false;
    co_return {};
}

Task<Expected<>> HTTPServer::IO::response(HTTPResponse resp,
                                          OwningStream &body) {
#if CO_ASYNC_DEBUG
    mResponseSavedForDebug = resp;
#endif
    if (!mBodyRead) {
        co_await co_await request_body();
    }
    builtinHeaders(resp);
    co_await co_await mHttp->writeResponse(resp);
    co_await co_await mHttp->writeBodyStream(body);
    mBodyRead = false;
    co_return {};
}

void HTTPServer::IO::builtinHeaders(HTTPResponse &res) {
    res.headers.insert("server"_s, "co_async/0.0.1"_s);
    res.headers.insert("accept"_s, "*/*"_s);
    res.headers.insert("accept-ranges"_s, "bytes"_s);
    res.headers.insert("date"_s, httpDateNow());
}

HTTPServer::HTTPServer() : mImpl(std::make_unique<Impl>()) {}

HTTPServer::~HTTPServer() = default;

#if CO_ASYNC_DEBUG
void HTTPServer::enableLogRequests() {
    mImpl->mLogRequests = true;
}
#endif

void HTTPServer::timeout(std::chrono::steady_clock::duration timeout) {
    mImpl->mTimeout = timeout;
}

void HTTPServer::route(std::string_view methods, std::string_view path,
                       HTTPHandler handler) {
    mImpl->mRoutes.insert_or_assign(
        String(path),
        {handler, split_string(upper_string(methods), ' ').collect()});
}

void HTTPServer::route(std::string_view methods, std::string_view prefix,
                       HTTPRouteMode mode, HTTPPrefixHandler handler) {
    auto it = std::lower_bound(mImpl->mPrefixRoutes.begin(),
                               mImpl->mPrefixRoutes.end(), prefix,
                               [](auto const &item, auto const &prefix) {
                                   return item.first.size() > prefix.size();
                               });
    mImpl->mPrefixRoutes.insert(
        it,
        {String(prefix),
         {handler, mode, split_string(upper_string(methods), ' ').collect()}});
}

void HTTPServer::route(HTTPHandler handler) {
    mImpl->mDefaultRoute = handler;
}

Task<std::unique_ptr<HTTPProtocol>>
HTTPServer::prepareHTTPS(SocketHandle handle, SSLServerState &https) const {
    using namespace std::string_view_literals;
    static char const *const protocols[] = {
        /* "h2", */
        "http/1.1",
    };
    auto sock = ssl_accept(std::move(handle), https.cert, https.skey, protocols,
                           &https.cache);
    sock.timeout(mImpl->mTimeout);
    /* if (auto peek = co_await sock.peekn(2); peek && *peek == "h2"sv) { */
    /*     co_return std::make_unique<HTTPProtocolVersion2>(std::move(sock)); */
    /* } */
    co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
}

Task<std::unique_ptr<HTTPProtocol>>
HTTPServer::prepareHTTP(SocketHandle handle) const {
    auto sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(mImpl->mTimeout);
    co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
}

Task<Expected<>> HTTPServer::handle_http(SocketHandle handle) const {
    // #if CO_ASYNC_ALLOC
    //     std::pmr::unsynchronized_pool_resource pool{currentAllocator};
    //     ReplaceAllocator _ = &pool;
    // #endif
    /* int h = handle.fileNo(); */
#if CO_ASYNC_DEBUG
    auto err =
        co_await doHandleConnection(co_await prepareHTTP(std::move(handle)));
    if (err.has_error()) [[unlikely]] {
        std::cerr << err.mErrorLocation.file_name() << ":"
                  << err.mErrorLocation.line() << ": "
                  << err.mErrorLocation.function_name() << ": "
                  << err.error().message() << '\n';
        co_return err;
    }
#else
    co_await co_await doHandleConnection(
        co_await prepareHTTP(std::move(handle)));
#endif
    co_return {};
}

Task<Expected<>>
HTTPServer::handle_http_redirect_to_https(SocketHandle handle) const {
    using namespace std::string_literals;
    auto http = co_await prepareHTTP(std::move(handle));
    while (true) {
        IO io(http.get());
        if (!co_await co_await io.readRequestHeader()) {
            break;
        }
        if (auto host = io.request.headers.get("host")) {
            auto location = "https://"_s + *host + io.request.uri.dump();
            HTTPResponse res = {
                .status = 302,
                .headers =
                    {
                        {"location"_s, location},
                        {"content-type"_s, "text/plain"_s},
                    },
            };
            co_await co_await io.response(res, location);
        } else {
            co_await co_await make_error_response(io, 403);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPServer::handle_https(SocketHandle handle,
                                          SSLServerState &https) const {
    /* int h = handle.fileNo(); */
    co_await co_await doHandleConnection(
        co_await prepareHTTPS(std::move(handle), https));
    /* co_await UringOp().prep_shutdown(h, SHUT_RDWR); */
    co_return {};
}

Task<Expected<>>
HTTPServer::doHandleConnection(std::unique_ptr<HTTPProtocol> http) const {
    while (true) {
        IO io(http.get());
        if (!co_await co_await io.readRequestHeader()) {
            break;
        }
#if CO_ASYNC_DEBUG
        std::chrono::steady_clock::time_point t0;
        if (mImpl->mLogRequests) {
            std::clog << io.request.method + ' ' + io.request.uri.dump() + '\n';
            for (auto [k, v]: io.request.headers) {
                if (k == "cookie" || k == "set-cookie" ||
                    k == "authorization") {
                    v = "*****";
                }
                std::clog << "      " + capitalizeHTTPHeader(k) + ": " + v +
                                 '\n';
            }
            t0 = std::chrono::steady_clock::now();
        }
#endif
        co_await co_await mImpl->doHandleRequest(io);
#if CO_ASYNC_DEBUG
        if (mImpl->mLogRequests) {
            auto dt = std::chrono::steady_clock::now() - t0;
            std::clog << io.request.method + ' ' + io.request.uri.dump() + ' ' +
                             to_string(io.mResponseSavedForDebug.status) + ' ' +
                             String(getHTTPStatusName(
                                 io.mResponseSavedForDebug.status)) +
                             ' ' +
                             to_string(std::chrono::duration_cast<
                                           std::chrono::milliseconds>(dt)
                                           .count()) +
                             "ms\n";
            for (auto [k, v]: io.mResponseSavedForDebug.headers) {
                if (k == "cookie" || k == "set-cookie" ||
                    k == "authorization") {
                    v = "*****";
                }
                std::clog << "      " + capitalizeHTTPHeader(k) + ": " + v +
                                 '\n';
            }
        }
#endif
    }
    co_return {};
}

Task<Expected<>> HTTPServer::make_error_response(IO &io, int status) {
    auto error = to_string(status) + ' ' + String(getHTTPStatusName(status));
    HTTPResponse res{
        .status = status,
        .headers =
            {
                {"content-type", "text/html;charset=utf-8"},
            },
    };
    co_await co_await io.response(
        res, "<html><head><title>" + error +
                 "</title></head><body><center><h1>" + error +
                 "</h1></center><hr><center>co_async</center></body></html>");
    co_return {};
}
} // namespace co_async








namespace co_async {
Task<Expected<SocketHandle>>
socket_proxy_connect(char const *host, int port, std::string_view proxy,
                     std::chrono::steady_clock::duration timeout) {
    using namespace std::string_view_literals;
    if (proxy.empty()) {
        co_return co_await socket_connect(
            co_await AddressResolver().host(host).port(port).resolve_one());
    } else {
#if CO_ASYNC_DEBUG
        if (!proxy.starts_with("http://")) {
            std::cerr << "WARNING: both http_proxy and https_proxy variable "
                         "should starts with http://\n";
        }
#endif
        auto sock = co_await co_await socket_connect(
            co_await AddressResolver().host(proxy).resolve_one(),
            co_await co_cancel);
        String hostName(host);
        hostName += ':';
        hostName += to_string(port);
        String header("CONNECT "sv);
        header += hostName;
        header += " HTTP/1.1\r\nHost: "sv;
        header += hostName;
        header += "\r\nProxy-Connection: Keep-Alive\r\n\r\n"sv;
        std::span<char const> buf = header;
        std::size_t n = 0;
        do {
            n = co_await co_await socket_write(sock, buf, timeout);
            if (!n) [[unlikely]] {
                co_return std::errc::connection_reset;
            }
            buf = buf.subspan(n);
        } while (buf.size() > 0);
        using namespace std::string_view_literals;
        auto desiredResponse = "HTTP/1.1 200 Connection established\r\n\r\n"sv;
        String response(39, '\0');
        std::span<char> outbuf = response;
        do {
            n = co_await co_await socket_read(sock, outbuf, timeout);
            if (!n) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: proxy server failed to establish "
                             "connection: [" +
                                 response.substr(0, response.size() -
                                                        outbuf.size()) +
                                 "]\n";
#endif
                co_return std::errc::connection_reset;
            }
            outbuf = outbuf.subspan(n);
        } while (outbuf.size() > 0);
        if (std::string_view(response).substr(8) != desiredResponse.substr(8))
            [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: proxy server seems failed to establish "
                         "connection: [" +
                             response + "]\n";
#endif
            co_return std::errc::connection_reset;
        }
        co_return sock;
    }
}
} // namespace co_async

#endif

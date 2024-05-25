#pragma once

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
    static auto debug_deref_avoid(T const &t)
        -> std::enable_if_t<!std::is_void_v<decltype(*t)>, decltype(*t)> {
        return *t;
    }

    struct debug_special_void {
        char const (&repr() const)[5] {
            return "void";
        }
    };

    template <class T>
    static auto debug_deref_avoid(T const &t)
        -> std::enable_if_t<std::is_void_v<decltype(*t)>, debug_special_void> {
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
                        << std::setw(2) << static_cast<int>(c)
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
        char *p = abi::__cxa_demangle(name, 0, 0, &status);
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
            oss << static_cast<std::uint64_t>(
                static_cast<typename std::make_unsigned<T>::type>(t));
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
                             }) {
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
                                 (void)(bool)t;
                             }) {
            if ((bool)t) {
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
    DEBUG_COND(is_ostream_ok, std::declval<std::ostream &>()
                                  << std::declval<T const &>());
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

    DEBUG_COND(is_variant, std::variant_size<T>::value);
    DEBUG_COND(is_smart_pointer, static_cast<void const volatile *>(
                                     std::declval<T const &>().get()));
    DEBUG_COND(is_optional, (((void)*std::declval<T const &>(), (void)0),
                             ((void)(bool)std::declval<T const &>(), (void)0)));
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
            oss << static_cast<std::uint64_t>(
                static_cast<typename std::make_unsigned<T>::type>(t));
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
            int unused[] = {(call<Args>(std::forward<Args>(args)), 0)...};
            (void)unused;
        }
    };

    template <class T>
    struct debug_format_trait<
        T,
        typename std::enable_if<
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value &&
            debug_cond_enum<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
#  ifdef DEBUG_MAGIC_ENUM
            oss << DEBUG_MAGIC_ENUM(t);
#  else
            oss << debug_demangle(typeid(T).name()) << DEBUG_ENUM_BRACE[0];
            oss << static_cast<typename std::underlying_type<T>::type>(t);
            oss << DEBUG_ENUM_BRACE[1];
#  endif
        }
    };

    template <class V>
    struct debug_format_trait<std::type_info, V> {
        void operator()(std::ostream &oss, std::type_info const &t) const {
            oss << debug_demangle(t.name());
        }
    };

    template <class V>
    struct debug_format_trait<std::errc, V> {
        void operator()(std::ostream &oss, std::errc const &t) const {
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
        }
    };

    template <class V>
    struct debug_format_trait<std::error_code, V> {
        void operator()(std::ostream &oss, std::error_code const &t) const {
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
                oss << t.category().name() << " error " << t.value() << ": "
                    << t.message();
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
            debug_cond_is_member_repr<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            debug_format(oss, raw_repr_if_string(t.DEBUG_REPR_NAME()));
        }
    };

    template <class T>
    struct debug_format_trait<
        T,
        typename std::enable_if<
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
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
            !debug_cond_string<T>::value && !debug_cond_bool<T>::value &&
            !debug_cond_char<T>::value && !debug_cond_unicode_char<T>::value &&
            !debug_cond_integral_unsigned<T>::value &&
            !debug_cond_integral<T>::value &&
            !debug_cond_floating_point<T>::value &&
            !debug_cond_is_smart_pointer<T>::value &&
            !debug_cond_error_code<T>::value &&
            !debug_cond_reference_wrapper<T>::value &&
            !debug_cond_is_ostream_ok<T>::value &&
            !debug_cond_pointer<T>::value && !debug_cond_is_range<T>::value &&
            !debug_cond_is_tuple<T>::value && !debug_cond_enum<T>::value &&
            !debug_cond_is_member_repr<T>::value &&
            !debug_cond_is_member_repr_stream<T>::value &&
            !debug_cond_is_adl_repr<T>::value &&
            !debug_cond_is_adl_repr_stream<T>::value &&
            !debug_cond_is_variant<T>::value &&
            debug_cond_is_optional<T>::value>::type> {
        void operator()(std::ostream &oss, T const &t) const {
            if ((bool)t) {
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
            std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
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

    template <class T, std::enable_if_t<
                           std::is_convertible<T, std::string>::value ||
                               std::is_convertible<T, DEBUG_STRING_VIEW>::value,
                           int> = 0>
    static raw_repr_t<T> raw_repr_if_string(T const &value) {
        return {value};
    }

    template <class T, std::enable_if_t<
                           !(std::is_convertible<T, std::string>::value ||
                             std::is_convertible<T, DEBUG_STRING_VIEW>::value),
                           int> = 0>
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
#elif DEBUG_LEVEL
# ifdef DEBUG_SOURCE_LOCATION_FAKER
#  define debug() debug(true, DEBUG_SOURCE_LOCATION_FAKER)
# endif
#endif


#include <cassert>     // IWYU pragma: export
#include <cctype>      // IWYU pragma: export
#include <cerrno>      // IWYU pragma: export
#include <cfloat>      // IWYU pragma: export
#include <ciso646>     // IWYU pragma: export
#include <climits>     // IWYU pragma: export
#include <clocale>     // IWYU pragma: export
#include <cmath>       // IWYU pragma: export
#include <csetjmp>     // IWYU pragma: export
#include <csignal>     // IWYU pragma: export
#include <cstdarg>     // IWYU pragma: export
#include <cstddef>     // IWYU pragma: export
#include <cstdio>      // IWYU pragma: export
#include <cstdlib>     // IWYU pragma: export
#include <cstring>     // IWYU pragma: export
#include <ctime>       // IWYU pragma: export
#include <cwchar>      // IWYU pragma: export
#include <cwctype>     // IWYU pragma: export
#if __cplusplus >= 201103L
# include <ccomplex>   // IWYU pragma: export
# include <cfenv>      // IWYU pragma: export
# include <cinttypes>  // IWYU pragma: export
# if __has_include(<cstdalign>)
#  include <cstdalign> // IWYU pragma: export
# endif
# include <cstdbool>   // IWYU pragma: export
# include <cstdint>    // IWYU pragma: export
# include <ctgmath>    // IWYU pragma: export
# include <cuchar>     // IWYU pragma: export
#endif
// C++
#include <algorithm>           // IWYU pragma: export
#include <bitset>              // IWYU pragma: export
#include <complex>             // IWYU pragma: export
#include <deque>               // IWYU pragma: export
#include <exception>           // IWYU pragma: export
#include <fstream>             // IWYU pragma: export
#include <functional>          // IWYU pragma: export
#include <iomanip>             // IWYU pragma: export
#include <ios>                 // IWYU pragma: export
#include <iosfwd>              // IWYU pragma: export
#include <iostream>            // IWYU pragma: export
#include <istream>             // IWYU pragma: export
#include <iterator>            // IWYU pragma: export
#include <limits>              // IWYU pragma: export
#include <list>                // IWYU pragma: export
#include <locale>              // IWYU pragma: export
#include <map>                 // IWYU pragma: export
#include <memory>              // IWYU pragma: export
#include <new>                 // IWYU pragma: export
#include <numeric>             // IWYU pragma: export
#include <ostream>             // IWYU pragma: export
#include <queue>               // IWYU pragma: export
#include <set>                 // IWYU pragma: export
#include <sstream>             // IWYU pragma: export
#include <stack>               // IWYU pragma: export
#include <stdexcept>           // IWYU pragma: export
#include <streambuf>           // IWYU pragma: export
#include <string>              // IWYU pragma: export
#include <typeinfo>            // IWYU pragma: export
#include <utility>             // IWYU pragma: export
#include <valarray>            // IWYU pragma: export
#include <vector>              // IWYU pragma: export
#if __cplusplus >= 201103L
# include <array>              // IWYU pragma: export
# include <atomic>             // IWYU pragma: export
# include <chrono>             // IWYU pragma: export
# include <codecvt>            // IWYU pragma: export
# include <condition_variable> // IWYU pragma: export
# include <forward_list>       // IWYU pragma: export
# include <future>             // IWYU pragma: export
# include <initializer_list>   // IWYU pragma: export
# include <mutex>              // IWYU pragma: export
# include <random>             // IWYU pragma: export
# include <ratio>              // IWYU pragma: export
# include <regex>              // IWYU pragma: export
# include <scoped_allocator>   // IWYU pragma: export
# include <system_error>       // IWYU pragma: export
# include <thread>             // IWYU pragma: export
# include <tuple>              // IWYU pragma: export
# include <type_traits>        // IWYU pragma: export
# include <typeindex>          // IWYU pragma: export
# include <unordered_map>      // IWYU pragma: export
# include <unordered_set>      // IWYU pragma: export
#endif
#if __cplusplus >= 201402L
# include <shared_mutex> // IWYU pragma: export
#endif
#if __cplusplus >= 201703L
# include <any>             // IWYU pragma: export
# include <charconv>        // IWYU pragma: export
# include <execution>       // IWYU pragma: export
# include <filesystem>      // IWYU pragma: export
# include <memory_resource> // IWYU pragma: export
# include <optional>        // IWYU pragma: export
# include <string_view>     // IWYU pragma: export
# include <variant>         // IWYU pragma: export
#endif
#if __cplusplus >= 202002L
# if __has_include(<barrier>)
#  include <barrier> // IWYU pragma: export
# endif
# include <bit>      // IWYU pragma: export
# include <compare>  // IWYU pragma: export
# include <concepts> // IWYU pragma: export
# if __cpp_impl_coroutine
#  if __has_include(<coroutine>)
#   include <coroutine> // IWYU pragma: export
#  endif
# endif
# if __has_include(<latch>)
#  include <latch>      // IWYU pragma: export
# endif
# include <numbers>     // IWYU pragma: export
# include <ranges>      // IWYU pragma: export
# include <span>        // IWYU pragma: export
# if __has_include(<stop_token>)
#  include <stop_token> // IWYU pragma: export
# endif
# if __has_include(<semaphore>)
#  include <semaphore> // IWYU pragma: export
# endif
# if __has_include(<source_location>)
#  include <source_location> // IWYU pragma: export
# endif
# if __has_include(<syncstream>)
#  include <syncstream> // IWYU pragma: export
# endif
# include <version>     // IWYU pragma: export
#endif
#if __cplusplus > 202002L
# if __has_include(<expected>)
#  include <expected> // IWYU pragma: export
# endif
# if __has_include(<spanstream>)
#  include <spanstream> // IWYU pragma: export
# endif
# if __has_include(<stacktrace>)
#  include <stacktrace> // IWYU pragma: export
# endif
# if __has_include(<stdatomic.h>)
#  include <stdatomic.h> // IWYU pragma: export
# endif
#endif
/* #ifdef CO_ASYNC_DEBUG */
/* #define DEBUG_LEVEL 1 */
/*  */
/* #endif */




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
struct Void final {
    explicit Void() = default;

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
struct AvoidVoidTrait<void> final {
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






namespace co_async {
template <class T>
struct GeneratorPromise {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
        mFinal = true;
    }

    auto yield_value(T &&ret) {
        mResult.putValue(std::move(ret));
        return PreviousAwaiter(mPrevious);
    }

    auto yield_value(T const &ret) {
        mResult.putValue(ret);
        return PreviousAwaiter(mPrevious);
    }

    void return_void() {
        mFinal = true;
    }

    bool final() {
        if (mFinal) {
            if (mException) [[unlikely]] {
                std::rethrow_exception(mException);
            }
        }
        return mFinal;
    }

    T result() {
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<GeneratorPromise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious;
    bool mFinal = false;
    Uninitialized<T> mResult;
    std::exception_ptr mException{};
    GeneratorPromise &operator=(GeneratorPromise &&) = delete;
};

template <class T>
struct GeneratorPromise<T &> {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    auto final_suspend() noexcept {
        return PreviousAwaiter(mPrevious);
    }

    void unhandled_exception() noexcept {
        mException = std::current_exception();
        mResult = nullptr;
    }

    auto yield_value(T &ret) {
        mResult = std::addressof(ret);
        return PreviousAwaiter(mPrevious);
    }

    void return_void() {
        mResult = nullptr;
    }

    bool final() {
        if (!mResult) {
            if (mException) [[unlikely]] {
                std::rethrow_exception(mException);
            }
            return true;
        }
        return false;
    }

    T &result() {
        return *mResult;
    }

    auto get_return_object() {
        return std::coroutine_handle<GeneratorPromise>::from_promise(*this);
    }

    std::coroutine_handle<> mPrevious{};
    T *mResult;
    std::exception_ptr mException{};
    GeneratorPromise &operator=(GeneratorPromise &&) = delete;
};

template <class T, class P = GeneratorPromise<T>>
struct [[nodiscard]] Generator {
    using promise_type = P;

    Generator(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    Generator(Generator &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    Generator &operator=(Generator &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~Generator() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        std::coroutine_handle<promise_type>
        await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            mCoroutine.promise().mPrevious = coroutine;
            return mCoroutine;
        }

        std::optional<T> await_resume() const {
            if (mCoroutine.promise().final()) {
                return std::nullopt;
            }
            return mCoroutine.promise().result();
        }

        std::coroutine_handle<promise_type> mCoroutine;
    };

    auto operator co_await() const noexcept {
        return Awaiter(mCoroutine);
    }

    operator std::coroutine_handle<promise_type>() const noexcept {
        return mCoroutine;
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};
#if 0
template <class T, class A, class LoopRef>
struct GeneratorIterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    explicit GeneratorIterator(A awaiter, LoopRef loop) noexcept
        : mAwaiter(awaiter),
          mLoop(loop) {
        ++*this;
    }

    bool operator!=(std::default_sentinel_t) const noexcept {
        return mResult.has_value();
    }

    bool operator==(std::default_sentinel_t) const noexcept {
        return !(*this != std::default_sentinel);
    }

    friend bool operator==(std::default_sentinel_t,
                           GeneratorIterator const &it) noexcept {
        return it == std::default_sentinel;
    }

    friend bool operator!=(std::default_sentinel_t,
                           GeneratorIterator const &it) noexcept {
        return it == std::default_sentinel;
    }

    GeneratorIterator &operator++() {
        mAwaiter.mCoroutine.resume();
        mLoop.run();
        mResult = mAwaiter.await_resume();
        return *this;
    }

    GeneratorIterator operator++(int) {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    T &operator*() noexcept {
        return *mResult;
    }

    T *operator->() noexcept {
        return mResult.operator->();
    }

private:
    A mAwaiter;
    LoopRef mLoop;
    std::optional<T> mResult;
};

template <class Loop, class T, class P>
auto run_generator_on(Loop &loop, Generator<T, P> const &g) {
    using Awaiter = typename Generator<T, P>::Awaiter;

    struct GeneratorRange {
        explicit GeneratorRange(Awaiter awaiter, Loop &loop)
            : mAwaiter(awaiter),
              mLoop(loop) {
            mAwaiter.await_suspend(std::noop_coroutine());
        }

        auto begin() const noexcept {
            return GeneratorIterator<T, Awaiter, Loop &>(mAwaiter, mLoop);
        }

        std::default_sentinel_t end() const noexcept {
            return {};
        }

    private:
        Awaiter mAwaiter;
        Loop &mLoop;
    };

    return GeneratorRange(g.operator co_await(), loop);
};
#endif
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
template <class T>
struct ValueAwaiter {
    std::coroutine_handle<> mPrevious;
    Uninitialized<T> mValue;

    template <class... Args>
    explicit ValueAwaiter(std::in_place_t, Args &&...args) {
        mValue.putValue(std::forward<Args>(args)...);
    }

    explicit ValueAwaiter(std::in_place_t)
        requires(std::is_void_v<T>)
    {
        mValue.putValue(Void());
    }

    explicit ValueAwaiter(std::coroutine_handle<> previous)
        : mPrevious(previous) {}

    bool await_ready() const noexcept {
        return mPrevious == nullptr;
    }

    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> coroutine) const noexcept {
        return mPrevious;
    }

    T await_resume() noexcept {
        if constexpr (!std::is_void_v<T>) {
            return mValue.moveValue();
        }
    }
};
} // namespace co_async




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





#if CO_ASYNC_PERF

#endif




namespace co_async {
struct PromiseBase {
    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    struct FinalAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        template <class P>
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<P> coroutine) const noexcept {
            return static_cast<PromiseBase &>(coroutine.promise()).mPrevious;
        }

        void await_resume() const noexcept {}
    };

    auto final_suspend() noexcept {
        return FinalAwaiter();
    }

    void unhandled_exception() noexcept {
#if CO_ASYNC_EXCEPT
        mException = std::current_exception();
#else
        std::terminate();
#endif
    }

    void setPrevious(std::coroutine_handle<> previous) noexcept {
#if CO_ASYNC_DEBUG
        if (mPrevious) [[unlikely]] {
            std::cerr << "WARNING: co_wait'ing twice on a single task\n";
        }
#endif
        mPrevious = previous;
    }

    PromiseBase &operator=(PromiseBase &&) = delete;

protected:
    std::coroutine_handle<> mPrevious;
#if CO_ASYNC_EXCEPT
    std::exception_ptr mException{};
#endif
};

template <class T>
struct TaskPromise : PromiseBase {
    void return_value(T &&ret) {
        mResult.putValue(std::move(ret));
    }

    void return_value(T const &ret) {
        mResult.putValue(ret);
    }

    T result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

private:
    Uninitialized<T> mResult;
#if CO_ASYNC_PERF
public:
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <>
struct TaskPromise<void> : PromiseBase {
    void return_void() noexcept {}

    void result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }
#if CO_ASYNC_PERF
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class E>
struct TaskPromise<Expected<T, E>> : PromiseBase {
    void return_value(Expected<T, E> &&ret) {
        mResult.putValue(std::move(ret));
    }

    void return_value(Expected<T, E> const &ret) {
        mResult.putValue(ret);
    }

    /* void return_value(T &&ret) requires (!std::is_void_v<T>) { */
    /*     mResult.putValue(std::move(ret)); */
    /* } */
    /*  */
    /* void return_value(T const &ret) requires (!std::is_void_v<T>) { */
    /*     mResult.putValue(ret); */
    /* } */
    /*  */
    /* void return_void() requires (std::is_void_v<T>) { */
    /*     mResult.putValue(); */
    /* } */
    Expected<T, E> result() {
#if CO_ASYNC_EXCEPT
        if (mException) [[unlikely]] {
            std::rethrow_exception(mException);
        }
#endif
        return mResult.moveValue();
    }

    auto get_return_object() {
        return std::coroutine_handle<TaskPromise>::from_promise(*this);
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &&e) noexcept {
        if (e.has_error()) [[unlikely]] {
            if constexpr (std::is_void_v<E>) {
                mResult.putValue(Unexpected<E>());
            } else {
                static_assert(std::same_as<E2, E>,
                              "co_await'ing Expected's error type mismatch");
                mResult.putValue(Unexpected<E>(std::move(e.error())));
            }
            return ValueAwaiter<T2>(mPrevious);
        }
        if constexpr (std::is_void_v<T2>) {
            return ValueAwaiter<void>(std::in_place);
        } else {
            return ValueAwaiter<T2>(std::in_place, *std::move(e));
        }
    }

    template <class T2, class E2>
    ValueAwaiter<T2> await_transform(Expected<T2, E2> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class E2>
    ValueAwaiter<void>
    await_transform(std::vector<Expected<void, E2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                if constexpr (std::is_void_v<E>) {
                    mResult.putValue(Unexpected<E>());
                } else {
                    static_assert(
                        std::same_as<E2, E>,
                        "co_await'ing Expected's error type mismatch");
                    mResult.putValue(Unexpected<E>(std::move(e[i].error())));
                }
                return ValueAwaiter<void>(mPrevious);
            }
        }
        return ValueAwaiter<void>(std::in_place);
    }

    template <class T2, class E2>
    ValueAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2, E2>> &&e) noexcept {
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (e[i].has_error()) [[unlikely]] {
                if constexpr (std::is_void_v<E>) {
                    mResult.putValue(Unexpected<E>());
                } else {
                    static_assert(
                        std::same_as<E2, E>,
                        "co_await'ing Expected's error type mismatch");
                    mResult.putValue(Unexpected<E>(std::move(e[i].error())));
                }
                return ValueAwaiter<std::vector<T2>>(mPrevious);
            }
        }
        std::vector<T2> ret;
        ret.reserve(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) {
            ret.emplace_back(*std::move(e[i]));
        }
        return ValueAwaiter<std::vector<T2>>(std::in_place, std::move(ret));
    }

    template <class T2, class E2>
    ValueAwaiter<std::vector<T2>>
    await_transform(std::vector<Expected<T2, E2>> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class... Ts, class E2>
    ValueAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts, E2>...> &&e) noexcept {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            if (!([&] {
                    if (std::get<Is>(e).has_error()) [[unlikely]] {
                        if constexpr (std::is_void_v<E>) {
                            mResult.putValue(Unexpected<E>());
                        } else {
                            static_assert(
                                std::same_as<E2, E>,
                                "co_await'ing Expected's error type mismatch");
                            mResult.putValue(Unexpected<E>(
                                std::move(std::get<Is>(e).error())));
                        }
                        return false;
                    }
                    return true;
                }() &&
                  ...)) {
                return ValueAwaiter<std::tuple<Avoid<Ts>...>>(mPrevious);
            }
            return ValueAwaiter<std::tuple<Avoid<Ts>...>>(
                std::in_place, [&]() -> decltype(auto) {
                    return *std::move(std::get<Is>(e)), Void();
                }()...);
        }(std::make_index_sequence<sizeof...(Ts)>());
    }

    template <class... Ts, class E2>
    ValueAwaiter<std::tuple<Avoid<Ts>...>>
    await_transform(std::tuple<Expected<Ts, E2>...> &e) noexcept {
        return await_transform(std::move(e));
    }

    template <class U>
    U &&await_transform(U &&u) noexcept {
        return std::forward<U>(u);
    }

private:
    Uninitialized<Expected<T, E>> mResult;
#if CO_ASYNC_PERF
public:
    Perf mPerf;

    TaskPromise(std::source_location loc = std::source_location::current())
        : mPerf(loc) {}
#endif
};

template <class T, class P>
struct CustomPromise : TaskPromise<T> {
    auto get_return_object() {
        static_assert(std::is_base_of_v<CustomPromise, P>);
        return std::coroutine_handle<P>::from_promise(static_cast<P &>(*this));
    }
};

template <class T = void, class P = TaskPromise<T>>
struct [[nodiscard("did you forgot to co_await?")]] Task {
    using promise_type = P;

    /* Task(std::coroutine_handle<promise_type> coroutine) noexcept */
    /*     : mCoroutine(coroutine) { */
    /* } */
    /* Task(Task &&) = delete; */
    Task(std::coroutine_handle<promise_type> coroutine = nullptr) noexcept
        : mCoroutine(coroutine) {}

    Task(Task &&that) noexcept : mCoroutine(that.mCoroutine) {
        that.mCoroutine = nullptr;
    }

    Task &operator=(Task &&that) noexcept {
        std::swap(mCoroutine, that.mCoroutine);
    }

    ~Task() {
        if (!mCoroutine) {
            return;
        }
        // #if CO_ASYNC_DEBUG
        // if (!mCoroutine.done()) [[unlikely]] {
        // #if CO_ASYNC_PERF
        // auto &perf = mCoroutine.promise().mPerf;
        // std::cerr << "warning: task (" << perf.file << ":" << perf.line
        //<< ") destroyed undone\n";
        // #else
        // std::cerr << "warning: task destroyed undone\n";
        // #endif
        // }
        // #endif
        mCoroutine.destroy();
    }

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        std::coroutine_handle<P>
        await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            P &promise = mCoroutine.promise();
            promise.setPrevious(coroutine);
            return mCoroutine;
        }
#if CO_ASYNC_DEBUG
        T await_resume() const {
            auto coroutine = mCoroutine;
            mCoroutine = nullptr;
            return coroutine.promise().result();
        }

        explicit Awaiter(std::coroutine_handle<P> coroutine)
            : mCoroutine(coroutine) {}

        Awaiter(Awaiter &&that) noexcept
            : mCoroutine(std::exchange(that.mCoroutine, nullptr)) {}

        ~Awaiter() noexcept {
            if (mCoroutine && mCoroutine.done()) [[unlikely]] {
                std::cerr << "WARNING: done coroutine return value ignored\n";
                (void)mCoroutine.promise().result();
            }
        }

    private:
        mutable std::coroutine_handle<P> mCoroutine;
#else
        T await_resume() const {
            return mCoroutine.promise().result();
        }

        std::coroutine_handle<P> mCoroutine;
#endif
    };

    auto operator co_await() const noexcept {
        return Awaiter(mCoroutine);
    }

    std::coroutine_handle<promise_type> get() const noexcept {
        return mCoroutine;
    }

    std::coroutine_handle<promise_type> release() noexcept {
        return std::exchange(mCoroutine, nullptr);
    }

private:
    std::coroutine_handle<promise_type> mCoroutine;
};

template <class F, class... Args>
    requires(Awaitable<std::invoke_result_t<F, Args...>>)
inline auto co_bind(F &&f, Args &&...args) {
    return [](auto f) mutable -> std::invoke_result_t<F, Args...> {
        co_return co_await std::move(f)();
        /* std::optional o(std::move(f)); */
        /* decltype(auto) r = co_await std::move(*o)(); */
        /* o.reset(); */
        /* co_return */
        /*     typename AwaitableTraits<std::invoke_result_t<F,
         * Args...>>::RetType( */
        /*         std::forward<decltype(r)>(r)); */
    }(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
}
} // namespace co_async






namespace co_async {
template <Awaitable A, Awaitable F>
    requires(!std::is_invocable_v<F> &&
             !std::is_invocable_v<F, typename AwaitableTraits<A>::RetType>)
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
        result.putValue(co_await std::forward<decltype(t)>(t));
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
        std::get<Is>(result).moveValue()...);
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
            res.push_back(r.moveValue());
        }
        co_return res;
    }
}
} // namespace co_async





namespace co_async {
template <class T = void, class P = TaskPromise<T>>
struct TaskOwnedAwaiter : Task<T, P>::Awaiter {
private:
    Task<T, P> mTask;

public:
    explicit TaskOwnedAwaiter(Task<T, P> task)
        : Task<T, P>::Awaiter(task.operator co_await()),
          mTask(std::move(task)) {}
};
template <class T, class P>
TaskOwnedAwaiter(Task<T, P>) -> TaskOwnedAwaiter<T, P>;
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
    requires(!Awaitable<A> && std::is_invocable_v<A> &&
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
        /* #if CO_ASYNC_DEBUG */
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
/* #endif */
#else
        std::terminate();
#endif
    }

    void result() noexcept {}

    void return_void() noexcept {}

    auto get_return_object() {
        return std::coroutine_handle<IgnoreReturnPromise>::from_promise(*this);
    }

    void setPrevious(std::coroutine_handle<>) noexcept {}

    IgnoreReturnPromise &operator=(IgnoreReturnPromise &&) = delete;
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
            destructiveErase();
        }

    protected:
        void destructiveErase() {
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

template <class Value, class Compare = std::less<>>
struct ConcurrentRbTree : private RbTree<Value, Compare> {
private:
    using BaseTree = RbTree<Value, Compare>;

    void onDestructiveErase(BaseTree::RbNode *current) noexcept {
        std::lock_guard guard(mMutex);
        BaseTree::onDestructiveErase(current);
    }

public:
    struct NodeType : BaseTree::RbNode {
        NodeType() = default;
        NodeType(NodeType &&) = delete;

        ~NodeType() noexcept {
            destructiveErase();
        }

    protected:
        bool destructiveErase() {
            static_assert(
                std::is_base_of_v<NodeType, Value>,
                "Value type must be derived from RbTree<Value>::NodeType");
            if (this->rbTree) {
                auto lock =
                    static_cast<ConcurrentRbTree *>(this->rbTree)->lock();
                if (this->rbTree) [[likely]] {
                    lock->erase(static_cast<Value &>(*this));
                    this->rbTree = nullptr;
                    return true;
                }
            }
            return false;
        }
    };

    struct LockGuard {
    private:
        BaseTree *mThat;
        std::unique_lock<std::mutex> mGuard;

        explicit LockGuard(ConcurrentRbTree *that) noexcept
            : mThat(that),
              mGuard(that->mMutex) {}

        friend ConcurrentRbTree;

    public:
        BaseTree &operator*() const noexcept {
            return *mThat;
        }

        BaseTree *operator->() const noexcept {
            return mThat;
        }

        void unlock() noexcept {
            mGuard.unlock();
            mThat = nullptr;
        }
    };

    LockGuard lock() noexcept {
        return LockGuard(this);
    }

private:
    std::mutex mMutex;
};
} // namespace co_async







namespace co_async {
struct CancelToken;

struct [[nodiscard]] CancelSource {
private:
    struct CancellerBase : RbTree<CancellerBase>::NodeType {
        virtual Task<> doCancel() = 0;
        CancellerBase &operator=(CancellerBase &&) = delete;

        bool operator<(CancellerBase const &that) const noexcept {
            return this < &that;
        }

        virtual ~CancellerBase() = default;
    };

    template <class Canceller>
    struct CancellerImpl : CancellerBase {
        typename Canceller::OpType *mOp;

        explicit CancellerImpl(typename Canceller::OpType *op) : mOp(op) {}

        virtual Task<> doCancel() {
            return Canceller::doCancel(mOp);
        }
    };

    struct Impl {
        RbTree<CancellerBase> mCancellers;
        bool mCanceled;

        Task<> doCancel() {
            mCanceled = true;
            std::vector<Task<>> tasks;
            mCancellers.traverseInorder([&](CancellerBase &canceller) {
                tasks.push_back(canceller.doCancel());
            });
            mCancellers.clear();
            co_await when_all(tasks);
        }

        bool doIsCanceled() const noexcept {
            return mCanceled;
        }

        template <class Canceller, class Awaiter>
        Task<typename AwaitableTraits<Awaiter>::RetType>
        doGuard(Awaiter &&awaiter) {
            typename Canceller::OpType *op = std::addressof(awaiter);
            CancellerImpl<Canceller> cancellerImpl(op);
            mCancellers.insert(static_cast<CancellerBase &>(cancellerImpl));
            co_return co_await awaiter;
        }
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();
    friend CancelToken;

public:
    Task<> cancel() const {
        return mImpl->doCancel();
    }

    bool is_canceled() const {
        return mImpl->doIsCanceled();
    }

    template <class Canceller>
    auto invoke(auto &&awaiter) const {
        return mImpl->doGuard<Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }

    inline CancelToken token() const;
    CancelSource() = default;
    CancelSource(CancelSource &&) = default;
    CancelSource &operator=(CancelSource &&) = default;
};

struct CancelToken {
private:
    CancelSource::Impl *mImpl;

public:
    CancelToken() noexcept : mImpl(nullptr) {}

    CancelToken(CancelSource const &that) noexcept : mImpl(that.mImpl.get()) {}

    Task<> cancel() const {
        return mImpl->doCancel();
    }

    bool is_canceled() const noexcept {
        return mImpl->doIsCanceled();
    }

    Expected<> check() {
        if (mImpl->doIsCanceled()) [[unlikely]] {
            return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        return {};
    }

    template <class Canceller>
    auto guard(auto &&awaiter) const {
        return mImpl->doGuard<Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }

    auto guard(auto &&awaiter) const {
        return guard<typename std::decay_t<decltype(awaiter)>::Canceller>(
            std::forward<decltype(awaiter)>(awaiter));
    }
};

inline CancelToken CancelSource::token() const {
    return *this;
}
} // namespace co_async




namespace co_async {
template <class T, std::size_t Capacity = 0>
struct ConcurrentQueue {
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
        auto s = mStamp.load(std::memory_order_acquire);
        if (!canRead(s)) {
            /* mStamp.compare_exchange_weak(s, Stamp(0)); */
            return std::nullopt;
        }
        while (!mStamp.compare_exchange_weak(s, advectRead(s),
                                             std::memory_order_acq_rel)) {
            if (!canRead(s)) {
                return std::nullopt;
            }
        }
        return mHead[offsetRead(s)];
    }

    [[nodiscard]] bool push(T value) {
        auto s = mStamp.load(std::memory_order_acquire);
        if (!canWrite(s)) [[unlikely]] {
            return false;
        }
        while (!mStamp.compare_exchange_weak(s, advectWrite(s),
                                             std::memory_order_acq_rel)) {
            if (!canWrite(s)) [[unlikely]] {
                return false;
            }
        }
        mHead[offsetWrite(s)] = std::move(value);
        return true;
    }

    ConcurrentQueue() = default;
    ConcurrentQueue(ConcurrentQueue &&) = delete;

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
        return (offsetRead(s) & (Stamp)(kSize - 1)) !=
               ((offsetWrite(s) + (Stamp)(kSize - Capacity)) &
                (Stamp)(kSize - 1));
    }

    inline Stamp advectRead(Stamp s) const {
        return (Stamp)((((Stamp)(s >> Shift) + (Stamp)1) & (Stamp)(kSize - 1))
                       << Shift) |
               (s & (Stamp)(kSize - 1));
    }

    inline Stamp advectWrite(Stamp s) const {
        return (((s & (Stamp)(kSize - 1)) + (Stamp)1) & (Stamp)(kSize - 1)) |
               (Stamp)(s & ((Stamp)(kSize - 1) << Shift));
    }

    std::unique_ptr<T[]> mHead = std::make_unique<T[]>(kSize);
    std::atomic<Stamp> mStamp{0};
};

template <class T>
struct ConcurrentQueue<T, 0> {
    std::optional<T> pop() {
        std::lock_guard lck(mMutex);
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T p = std::move(mQueue.front());
        mQueue.pop_front();
        return p;
    }

    bool push(T p) {
        std::lock_guard lck(mMutex);
        mQueue.push_back(p);
        return true;
    }

private:
    std::deque<T> mQueue;
    std::mutex mMutex;
};

template <class T>
struct RingQueue {
    std::unique_ptr<T[]> mHead;
    T *mTail;
    T *mRead;
    T *mWrite;

    explicit RingQueue(std::size_t size)
        : mHead(std::make_unique<T[]>(size)),
          mTail(mHead.get() + size),
          mRead(mHead.get()),
          mWrite(mHead.get()) {}

    [[nodiscard]] std::size_t size() const noexcept {
        return mTail - mHead.get();
    }

    [[nodiscard]] std::optional<T> pop() {
        if (mRead == mWrite) {
            return std::nullopt;
        }
        T p = std::move(*mRead);
        mRead = mRead == mTail ? mHead.get() : mRead + 1;
        return p;
    }

    [[nodiscard]] bool push(T p) {
        T *nextWrite = mWrite == mTail ? mHead.get() : mWrite + 1;
        if (nextWrite == mRead) {
            return false;
        }
        *mWrite = std::move(p);
        mWrite = nextWrite;
        return true;
    }
};

template <class T>
struct InfinityQueue {
    [[nodiscard]] std::optional<T> pop() {
        if (mQueue.empty()) {
            return std::nullopt;
        }
        T p = std::move(mQueue.front());
        mQueue.pop_front();
        return p;
    }

    bool push(T p) {
        mQueue.push_back(p);
        return true;
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









#if CO_ASYNC_STEAL

#endif




namespace co_async {
struct IOContext;

struct GenericIOContext {
    struct TimerNode : CustomPromise<Expected<>, TimerNode>,
                       RbTree<TimerNode>::NodeType {
        using RbTree<TimerNode>::NodeType::destructiveErase;
        std::chrono::steady_clock::time_point mExpires;
        bool mCancelled = false;

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
                    return Unexpected{
                        std::make_error_code(std::errc::operation_canceled)};
                }
            }
        };

        struct Canceller {
            using OpType = Task<Expected<>, GenericIOContext::TimerNode>;

            static Task<> doCancel(OpType *op) {
                auto &promise = op->get().promise();
                promise.mCancelled = true;
                promise.destructiveErase();
                GenericIOContext::instance->enqueueJob(op->get());
                co_return;
            }

            static Expected<> earlyCancelValue(OpType *op) {
                return Unexpected{
                    std::make_error_code(std::errc::operation_canceled)};
            }
        };
    };

    bool runComputeOnly() {
        if (auto coroutine = mQueue.pop()) {
            coroutine->resume();
            return true;
        }
        return false;
    }

    bool runMTQueue() {
        std::unique_lock lock(mMTMutex);
        if (!mMTQueue.empty()) {
            auto coroutine = mMTQueue.front();
            mMTQueue.pop_front();
            lock.unlock();
            enqueueJob(coroutine);
            return true;
        }
        lock.unlock();
        return false;
    }

    std::optional<std::chrono::steady_clock::duration> runDuration() {
        while (true) {
            while (auto coroutine = mQueue.pop()) {
                coroutine->resume();
            }
            if (!mTimers.empty()) {
                auto &promise = mTimers.front();
                std::chrono::steady_clock::time_point now =
                    std::chrono::steady_clock::now();
                if (promise.mExpires <= now) {
                    promise.mCancelled = false;
                    promise.destructiveErase();
                    auto coroutine =
                        std::coroutine_handle<TimerNode>::from_promise(promise);
                    enqueueJob(coroutine);
                    continue;
                } else {
                    return promise.mExpires - now;
                }
            }
            return std::nullopt;
        }
    }

    void enqueueJob(std::coroutine_handle<> coroutine) {
        if (!mQueue.push(coroutine)) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "WARNING: coroutine queue overrun\n";
#endif
            std::lock_guard lock(mMTMutex);
            mMTQueue.push_back(coroutine);
        }
    }

    void enqueueJobMT(std::coroutine_handle<> coroutine) {
#if CO_ASYNC_STEAL
        enqueueJob(coroutine);
#else
        std::lock_guard lock(mMTMutex);
        mMTQueue.push_back(coroutine);
#endif
    }

    void enqueueTimerNode(TimerNode &promise) {
        mTimers.insert(promise);
    }

    void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            auto duration = runDuration();
            if (runMTQueue()) {
                continue;
            }
            if (duration) {
                std::this_thread::sleep_for(*duration);
            } else {
                break;
            }
        }
    }

    GenericIOContext() : mQueue(1 << 8) {}

    GenericIOContext(GenericIOContext &&) = delete;
    static inline thread_local GenericIOContext *instance;

private:
#if CO_ASYNC_STEAL
    ConcurrentQueue<std::coroutine_handle<>, (1 << 8) - 1> mQueue;
#else
    RingQueue<std::coroutine_handle<>> mQueue;
#endif
    RbTree<TimerNode> mTimers;
    std::mutex mMTMutex;
    std::list<std::coroutine_handle<>> mMTQueue;
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
    auto coroutine = wrapped.get();
    GenericIOContext::instance->enqueueJob(coroutine);
    wrapped.release();
}

inline void co_spawn(std::coroutine_handle<> coroutine) {
    GenericIOContext::instance->enqueueJob(coroutine);
}

inline Task<Expected<>, GenericIOContext::TimerNode>
co_sleep(std::chrono::steady_clock::time_point expires) {
    co_return co_await GenericIOContext::TimerNode::Awaiter(expires);
}

inline Task<Expected<>, GenericIOContext::TimerNode>
co_sleep(std::chrono::steady_clock::time_point expires, CancelToken cancel) {
    co_return co_await cancel.guard<GenericIOContext::TimerNode::Canceller>(
        co_sleep(expires));
}

inline Task<Expected<>, GenericIOContext::TimerNode>
co_sleep(std::chrono::steady_clock::duration timeout) {
    return co_sleep(std::chrono::steady_clock::now() + timeout);
}

inline Task<Expected<>, GenericIOContext::TimerNode>
co_sleep(std::chrono::steady_clock::duration timeout, CancelToken cancel) {
    return co_sleep(std::chrono::steady_clock::now() + timeout, cancel);
}

inline Task<> co_forever() {
    co_await std::suspend_always();
#if defined(__GNUC__) && defined(__has_builtin)
# if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
# endif
#endif
}

inline Task<> co_forever(CancelToken cancel) {
    struct ForeverAwaiter {
        struct Canceller {
            using OpType = ForeverAwaiter;

            static Task<> doCancel(OpType *op) {
                co_spawn(op->mPrevious);
                co_return;
            }

            static void earlyCancelValue(OpType *op) noexcept {}
        };

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) noexcept {
            mPrevious = coroutine;
        }

        void await_resume() const noexcept {}

        std::coroutine_handle<> mPrevious;
    };

    co_return co_await cancel.guard(ForeverAwaiter());
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
template <class F, class Timeout, class... Args>
    requires Awaitable<std::invoke_result_t<F, Args..., CancelToken>>
inline Task<std::optional<typename AwaitableTraits<
    std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>>
co_timeout(F &&task, Timeout timeout, Args &&...args) {
    CancelSource cs;
    CancelSource ct;
    std::optional<typename AwaitableTraits<
        std::invoke_result_t<F, Args..., CancelToken>>::AvoidRetType>
        result;
    co_await when_all(
        co_bind([&]() mutable -> Task<> {
            auto res =
                (co_await std::invoke(task, std::forward<Args>(args)..., ct),
                 Void());
            co_await cs.cancel();
            if (!ct.is_canceled()) {
                result = res;
            }
        }),
        co_bind([&]() mutable -> Task<> {
            (void)co_await co_sleep(timeout, cs);
            co_await ct.cancel();
        }));
    co_return result;
}
} // namespace co_async









namespace co_async {
struct TimedConditionVariable {
private:
    struct PromiseNode : CustomPromise<void, PromiseNode>,
                         RbTree<PromiseNode>::NodeType {
        bool operator<(PromiseNode const &that) const {
            if (!mExpires) {
                return false;
            }
            if (!that.mExpires) {
                return true;
            }
            return *mExpires < *that.mExpires;
        }

        void doCancel() {
            this->destructiveErase();
            co_spawn(std::coroutine_handle<PromiseNode>::from_promise(*this));
        }

        bool isCanceled() const noexcept {
            return this->rbTree == nullptr;
        }

        std::optional<std::chrono::steady_clock::time_point> mExpires;
    };

    RbTree<PromiseNode> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<PromiseNode> coroutine) const {
            mThat->pushWaiting(coroutine.promise());
        }

        void await_resume() const noexcept {}

        TimedConditionVariable *mThat;
    };

    PromiseNode *popWaiting() {
        if (mWaitingList.empty()) {
            return nullptr;
        }
        auto &promise = mWaitingList.front();
        mWaitingList.erase(promise);
        return &promise;
    }

    void pushWaiting(PromiseNode &promise) {
        mWaitingList.insert(promise);
    }

    struct Canceller {
        using OpType = Task<void, PromiseNode>;

        static Task<> doCancel(OpType *op) {
            op->get().promise().doCancel();
            co_return;
        }

        static void earlyCancelValue(OpType *op) noexcept {}
    };

    Task<> waitCancellable(std::chrono::steady_clock::time_point expires,
                           CancelToken cancel) {
        auto waiter = wait();
        waiter.get().promise().mExpires = expires;
        co_await cancel.guard<Canceller>(waiter);
    }

public:
    Task<void, PromiseNode> wait() {
        co_await Awaiter(this);
    }

    Task<Expected<>> wait(CancelToken cancel) {
        auto waiter = wait();
        co_await cancel.guard<Canceller>(waiter);
        if (waiter.get().promise().isCanceled()) {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        co_return {};
    }

    Task<Expected<>> wait(std::chrono::steady_clock::time_point expires) {
        auto res = co_await co_timeout(&TimedConditionVariable::waitCancellable,
                                       expires, this, expires);
        if (!res) {
            co_return Unexpected{
                std::make_error_code(std::errc::stream_timeout)};
        }
        co_return {};
    }

    Task<Expected<>> wait(std::chrono::steady_clock::duration timeout) {
        return wait(std::chrono::steady_clock::now() + timeout);
    }

    Task<> wait_until(std::invocable<> auto &&pred) {
        while (!std::invoke(pred)) {
            co_await wait();
        }
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred,
                                std::chrono::steady_clock::time_point expires) {
        while (!std::invoke(pred)) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await wait(expires)) {
                co_return Unexpected{
                    std::make_error_code(std::errc::stream_timeout)};
            }
        }
        co_return {};
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred,
                                std::chrono::steady_clock::duration timeout) {
        return wait_until(std::forward<decltype(pred)>(pred),
                          std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<>> wait_until(std::invocable<> auto &&pred,
                                CancelToken cancel) {
        while (!std::invoke(pred)) {
            if (cancel.is_canceled() || !co_await wait(cancel)) {
                co_return Unexpected{
                    std::make_error_code(std::errc::operation_canceled)};
            }
        }
    }

    void notify() {
        while (auto promise = popWaiting()) {
            co_spawn(
                std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }

    void notify_one() {
        if (auto promise = popWaiting()) {
            co_spawn(
                std::coroutine_handle<PromiseNode>::from_promise(*promise));
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        if (auto promise = popWaiting()) {
            return std::coroutine_handle<PromiseNode>::from_promise(*promise);
        }
        return nullptr;
    }
};

struct ConditionVariable {
private:
    std::deque<std::coroutine_handle<>> mWaitingList;

    struct Awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mThat->mWaitingList.push_back(coroutine);
        }

        void await_resume() const noexcept {}

        ConditionVariable *mThat;
    };

public:
    Awaiter operator co_await() noexcept {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        while (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            co_spawn(coroutine);
        }
    }

    void notify_one() {
        if (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            co_spawn(coroutine);
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        if (!mWaitingList.empty()) {
            auto coroutine = mWaitingList.front();
            mWaitingList.pop_front();
            return coroutine;
        }
        return nullptr;
    }
};

struct OneshotConditionVariable {
private:
    std::coroutine_handle<> mWaitingCoroutine{nullptr};
    bool mReady{false};

public:
    struct Awaiter {
        bool await_ready() const noexcept {
            return mThat->mReady;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
#if CO_ASYNC_DEBUG
            if (mThat->mWaitingCoroutine) [[unlikely]] {
                throw std::logic_error(
                    "please do not co_await on the same "
                    "OneshotConditionVariable or Future for multiple times");
            }
#endif
            mThat->mWaitingCoroutine = coroutine;
        }

        void await_resume() const noexcept {
#if CO_ASYNC_DEBUG
            if (!mThat->mReady) [[unlikely]] {
                throw std::logic_error("OneshotConditionVariable or Future "
                                       "waked up but not ready");
            }
#endif
            mThat->mReady = false;
        }

        OneshotConditionVariable *mThat;
    };

    Awaiter operator co_await() noexcept {
        return Awaiter(this);
    }

    Task<> wait() {
        co_await Awaiter(this);
    }

    void notify() {
        mReady = true;
        if (auto coroutine = mWaitingCoroutine) {
            mWaitingCoroutine = nullptr;
            co_spawn(coroutine);
        }
    }

    std::coroutine_handle<> notify_pop_coroutine() {
        mReady = true;
        if (auto coroutine = mWaitingCoroutine) {
            mWaitingCoroutine = nullptr;
            return coroutine;
        }
        return nullptr;
    }
};
} // namespace co_async







namespace co_async {
struct Semaphore {
private:
    std::size_t mCounter;
    std::size_t const mMaxCount;
    ConditionVariable mChanged;

public:
    explicit Semaphore(std::size_t maxCount = 1, std::size_t initialCount = 0)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::size_t count() const noexcept {
        return mCounter;
    }

    std::size_t max_count() const noexcept {
        return mMaxCount;
    }

    bool try_acquire() {
        auto count = mCounter;
        if (count > 0) {
            mCounter = count - 1;
            if (count == mMaxCount) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    bool try_release() {
        auto count = mCounter;
        if (count < mMaxCount) {
            mCounter = count + 1;
            if (count == 0) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    Task<> acquire() {
        while (!try_acquire()) {
            co_await mChanged.wait();
        }
    }

    Task<> release() {
        while (!try_release()) {
            co_await mChanged.wait();
        }
    }
};

struct TimedSemaphore {
private:
    std::size_t mCounter;
    std::size_t const mMaxCount;
    TimedConditionVariable mChanged;

public:
    explicit TimedSemaphore(std::size_t maxCount = 1,
                            std::size_t initialCount = 0)
        : mCounter(initialCount),
          mMaxCount(maxCount) {}

    std::size_t count() const noexcept {
        return mCounter;
    }

    std::size_t max_count() const noexcept {
        return mMaxCount;
    }

    bool try_acquire() {
        auto count = mCounter;
        if (count > 0) {
            mCounter = count - 1;
            if (count == mMaxCount) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    bool try_release() {
        auto count = mCounter;
        if (count < mMaxCount) {
            mCounter = count + 1;
            if (count == 0) {
                mChanged.notify_one();
            }
            return true;
        }
        return false;
    }

    Task<> acquire() {
        while (!try_acquire()) {
            co_await mChanged.wait();
        }
    }

    Task<> release() {
        while (!try_release()) {
            co_await mChanged.wait();
        }
    }

    Task<bool> try_acquire(std::chrono::steady_clock::duration timeout) {
        return try_acquire(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_acquire(std::chrono::steady_clock::time_point expires) {
        while (!try_acquire()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mChanged.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }

    Task<bool> try_release(std::chrono::steady_clock::duration timeout) {
        return try_release(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_release(std::chrono::steady_clock::time_point expires) {
        while (!try_release()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mChanged.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }
};
} // namespace co_async







namespace co_async {
struct BasicMutex {
    ConditionVariable mReady;
    std::atomic_bool mLocked;

    bool try_lock() {
        bool expect = false;
        return mLocked.compare_exchange_strong(
            expect, true, std::memory_order_acquire, std::memory_order_relaxed);
    }

    Task<> lock() {
        while (!try_lock()) {
            co_await mReady.wait();
        }
    }

    void unlock() {
        mLocked.store(false, std::memory_order_release);
        mReady.notify_one();
    }
};

struct BasicTimedMutex {
    TimedConditionVariable mReady;
    std::atomic_bool mLocked;

    bool try_lock() {
        bool expect = false;
        return mLocked.compare_exchange_strong(
            expect, true, std::memory_order_acquire, std::memory_order_relaxed);
    }

    Task<> lock() {
        while (!try_lock()) {
            co_await mReady.wait();
        }
    }

    Task<bool> try_lock(std::chrono::steady_clock::duration timeout) {
        return try_lock(std::chrono::steady_clock::now() + timeout);
    }

    Task<bool> try_lock(std::chrono::steady_clock::time_point expires) {
        while (!try_lock()) {
            if (std::chrono::steady_clock::now() > expires ||
                !co_await mReady.wait(expires)) {
                co_return false;
            }
        }
        co_return true;
    }

    void unlock() {
        mLocked.store(false, std::memory_order_release);
        mReady.notify_one();
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

    Locked try_lock() {
        if (auto e = mMutex.try_lock()) {
            return Locked(this);
        } else {
            return Locked();
        }
    }

    Task<Locked> lock() {
        co_await mMutex.lock();
        co_return Locked(this);
    }

    Task<Locked> try_lock_for(std::chrono::steady_clock::duration timeout) {
        if (!co_await mMutex.try_lock_for(timeout)) {
            co_return Locked();
        }
        co_return Locked(this);
    }

    Task<Locked> try_lock_until(std::chrono::steady_clock::time_point expires) {
        if (!co_await mMutex.try_lock_for(expires)) {
            co_return Locked();
        }
        co_return Locked(this);
    }

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
struct MutexImpl<M, void> : MutexImpl<M, Void> {};

template <class T = void>
struct Mutex : MutexImpl<BasicMutex, T> {};

template <class T = void>
struct TimedMutex : MutexImpl<BasicTimedMutex, T> {};

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
            return (bool)mLocked;
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
        Locked locked(co_await mMutex.lock(), this);
        if (mCalled.load(std::memory_order_relaxed)) {
            co_return Locked();
        }
        co_return std::move(locked);
    }
};
} // namespace co_async





namespace co_async {
inline Expected<int> expectError(int res) {
    if (res < 0) [[unlikely]] {
        return Unexpected{std::make_error_code(std::errc(-res))};
    }
    return res;
}

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

struct PlatformIOContextOptions {
    std::chrono::steady_clock::duration maxSleep = std::chrono::milliseconds(5);
    std::chrono::steady_clock::duration maxSleepInc =
        std::chrono::milliseconds(8);
    std::chrono::steady_clock::duration maxSleepLimit =
        std::chrono::milliseconds(100);
    std::optional<std::size_t> threadAffinity = std::nullopt;
};

struct PlatformIOContext {
    static void schedSetThreadAffinity(size_t cpu);
    bool
    waitEventsFor(std::size_t numBatch,
                  std::optional<std::chrono::steady_clock::duration> timeout);

    std::size_t pendingEventCount() const {
        return io_uring_cq_ready(&mRing);
    }

    struct io_uring *getRing() {
        return &mRing;
    }

    PlatformIOContext &operator=(PlatformIOContext &&) = delete;
    explicit PlatformIOContext(std::size_t entries = 512);
    ~PlatformIOContext();
    static thread_local PlatformIOContext *instance;

private:
    struct io_uring mRing;
};

struct [[nodiscard]] UringOp {
    UringOp() {
        struct io_uring *ring = PlatformIOContext::instance->getRing();
        mSqe = io_uring_get_sqe(ring);
        if (!mSqe) [[unlikely]] {
            throw std::bad_alloc();
        }
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

    static UringOp &link_ops(UringOp &&lhs, UringOp &&rhs) {
        lhs.mSqe->flags |= IOSQE_IO_LINK;
        rhs.mPrevious = std::noop_coroutine();
        return lhs;
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

public:
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

    UringOp &&prep_accept(int fd, struct sockaddr *addr, socklen_t *addrlen,
                          int flags) && {
        io_uring_prep_accept(mSqe, fd, addr, addrlen, flags);
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
        io_uring_prep_read(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                           offset);
        return std::move(*this);
    }

    UringOp &&prep_write(int fd, std::span<char const> buf,
                         std::uint64_t offset) && {
        io_uring_prep_write(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                            offset);
        return std::move(*this);
    }

    UringOp &&prep_read_fixed(int fd, std::span<char> buf, std::uint64_t offset,
                              int buf_index) && {
        io_uring_prep_read_fixed(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                                 offset, buf_index);
        return std::move(*this);
    }

    UringOp &&prep_write_fixed(int fd, std::span<char const> buf,
                               std::uint64_t offset, int buf_index) && {
        io_uring_prep_write_fixed(mSqe, fd, buf.data(),
                                  (unsigned int)buf.size(), offset, buf_index);
        return std::move(*this);
    }

    UringOp &&prep_readv(int fd, std::span<struct iovec const> buf,
                         std::uint64_t offset, int flags) && {
        io_uring_prep_readv2(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                             offset, flags);
        return std::move(*this);
    }

    UringOp &&prep_writev(int fd, std::span<struct iovec const> buf,
                          std::uint64_t offset, int flags) && {
        io_uring_prep_writev2(mSqe, fd, buf.data(), (unsigned int)buf.size(),
                              offset, flags);
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
                             (unsigned int)nbytes, flags);
        return std::move(*this);
    }
};

struct UringOpCanceller {
    using OpType = UringOp;

    static Task<> doCancel(OpType *op) {
        co_await UringOp().prep_cancel(op, IORING_ASYNC_CANCEL_ALL);
    }

    static int earlyCancelValue(OpType *op) noexcept {
        return -ECANCELED;
    }
};
} // namespace co_async







namespace co_async {
struct alignas(hardware_destructive_interference_size) IOContext {
private:
    GenericIOContext mGenericIO;
    PlatformIOContext mPlatformIO;
    std::jthread mThread;

    struct IOContextGuard {
        explicit IOContextGuard(IOContext *that) {
            if (IOContext::instance || GenericIOContext::instance ||
                PlatformIOContext::instance) [[unlikely]] {
                throw std::logic_error(
                    "each thread may contain only one IOContextGuard");
            }
            IOContext::instance = that;
            GenericIOContext::instance = &that->mGenericIO;
            PlatformIOContext::instance = &that->mPlatformIO;
        }

        ~IOContextGuard() {
            IOContext::instance = nullptr;
            GenericIOContext::instance = nullptr;
            PlatformIOContext::instance = nullptr;
        }

        IOContextGuard(IOContextGuard &&) = delete;
    };

public:
    explicit IOContext(std::in_place_t) {}

    explicit IOContext(PlatformIOContextOptions options = {}) {
        start(options);
    }

    IOContext(IOContext &&) = delete;

    void startHere(std::stop_token stop, PlatformIOContextOptions options,
                   std::span<IOContext> peerContexts) {
        IOContextGuard guard(this);
        if (options.threadAffinity) {
            PlatformIOContext::schedSetThreadAffinity(*options.threadAffinity);
        }
        auto maxSleep = options.maxSleep;
        while (!stop.stop_requested()) [[likely]] {
            auto duration = GenericIOContext::instance->runDuration();
            if (GenericIOContext::instance->runMTQueue()) {
                continue;
            }
            if (!duration || *duration > maxSleep) {
                duration = maxSleep;
            }
            bool hasEvent =
                PlatformIOContext::instance->waitEventsFor(1, duration);
            if (hasEvent) {
                auto t = maxSleep + options.maxSleepInc;
                if (t > options.maxSleepLimit) {
                    t = options.maxSleepLimit;
                }
                maxSleep = t;
            } else {
                maxSleep = options.maxSleep;
            }
#if CO_ASYNC_STEAL
            if (!hasEvent && !peerContexts.empty()) {
                for (IOContext *p = peerContexts.data();
                     p != peerContexts.data() + peerContexts.size(); ++p) {
                    if (p->mGenericIO.runComputeOnly()) {
                        break;
                    }
                }
            }
#endif
        }
    }

    void start(PlatformIOContextOptions options = {},
               std::span<IOContext> peerContexts = {}) {
        mThread = std::jthread([this, options = std::move(options),
                                peerContexts](std::stop_token stop) {
            this->startHere(stop, options, peerContexts);
        });
    }

    void spawn(std::coroutine_handle<> coroutine) {
        mGenericIO.enqueueJob(coroutine);
    }

    void spawn_mt(std::coroutine_handle<> coroutine) /* MT-safe */ {
        mGenericIO.enqueueJobMT(coroutine);
    }

    template <class T, class P>
    void spawn(Task<T, P> task) {
        auto wrapped = coSpawnStarter(std::move(task));
        auto coroutine = wrapped.get();
        mGenericIO.enqueueJob(coroutine);
        wrapped.release();
    }

    template <class T, class P>
    T join(Task<T, P> task) {
        return contextJoin(*this, std::move(task));
    }

    static inline thread_local IOContext *instance;
};

template <class T, class P>
inline Task<> contextJoinHelper(Task<T, P> task, std::condition_variable &cv,
                                Uninitialized<T> &result
#if CO_ASYNC_EXCEPT
                                ,
                                std::exception_ptr exception
#endif
) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        result.putValue((co_await task, Void()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: exception occurred in IOContext::join\n";
# endif
        exception = std::current_exception();
    }
#endif
    cv.notify_one();
}

template <class T, class P>
T contextJoin(IOContext &context, Task<T, P> task) {
    std::condition_variable cv;
    std::mutex mtx;
    Uninitialized<T> result;
#if CO_ASYNC_EXCEPT
    std::exception_ptr exception;
#endif
    context.spawn(contextJoinHelper(std::move(task), cv, result
#if CO_ASYNC_EXCEPT
                                    ,
                                    exception
#endif
                                    ));
    std::unique_lock lck(mtx);
    cv.wait(lck);
    lck.unlock();
#if CO_ASYNC_EXCEPT
    if (exception) [[unlikely]] {
        std::rethrow_exception(exception);
    }
#endif
    if constexpr (!std::is_void_v<T>) {
        return result.moveValue();
    }
}

inline auto co_resume_on(IOContext &context) {
    struct ResumeOnAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            mContext.spawn(coroutine);
        }

        void await_resume() const noexcept {}

        IOContext &mContext;
    };

    return ResumeOnAwaiter(context);
}
} // namespace co_async








namespace co_async {

struct ThreadPool {
private:
    struct Thread;

    std::mutex mWorkingMutex;
    std::list<Thread *> mWorkingThreads;
    std::mutex mFreeMutex;
    std::list<Thread *> mFreeThreads;
    std::mutex mThreadsMutex;
    std::list<Thread> mThreads;

    Thread *submitJob(std::function<void()> func);

public:
    Task<Expected<>> rawRun(std::function<void()> func) /* MT-safe */;
    Task<Expected<>> rawRun(std::function<void(std::stop_token)> func,
                            CancelToken cancel) /* MT-safe */;

    auto run(std::invocable auto func) /* MT-safe */
        -> Task<Expected<std::invoke_result_t<decltype(func)>>> {
        std::optional<Avoid<std::invoke_result_t<decltype(func)>>> res;
        auto e = co_await rawRun([&res, func = std::move(func)]() mutable {
            res = (func(), Void());
        });
        if (e.has_error()) [[unlikely]] {
            co_return Unexpected{e.error()};
        }
        if (!res) [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
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
            co_return Unexpected{e.error()};
        }
        if (!res) {
            co_return Unexpected{
                std::make_error_code(std::errc::operation_canceled)};
        }
        co_return std::move(*res);
    }

    std::size_t threads_count() /* MT-safe */;
    std::size_t working_threads_count() /* MT-safe */;

    ThreadPool();
    ~ThreadPool();
    ThreadPool &operator=(ThreadPool &&) = delete;
};

} // namespace co_async









namespace co_async {
template <class T = void>
struct [[nodiscard]] FutureToken;

template <class T = void>
struct [[nodiscard]] FutureSource {
public:
    struct Awaiter;

private:
    struct Impl : OneshotConditionVariable {
        Uninitialized<T> mValue;
#if CO_ASYNC_EXCEPT
        std::exception_ptr mException{nullptr};
#endif
        inline FutureSource::Awaiter makeAwaiter();
    };

    std::unique_ptr<Impl> mImpl = std::make_unique<Impl>();

public:
    struct Awaiter : OneshotConditionVariable::Awaiter {
        T await_resume() const noexcept {
            OneshotConditionVariable::Awaiter::await_resume();
            auto impl = static_cast<Impl *>(mThat);
            if constexpr (!std::is_void_v<T>) {
                return impl->mValue.moveValue();
            }
        }
    };

    FutureSource() = default;
    FutureSource(FutureSource &&) = default;
    FutureSource &operator=(FutureSource &&) = default;

    auto operator co_await() const noexcept {
        return mImpl->makeAwaiter();
    }

    inline FutureToken<T> token() const noexcept;
    template <class>
    friend struct FutureToken;
};

template <class T>
auto FutureSource<T>::Impl::makeAwaiter() -> FutureSource::Awaiter {
    return FutureSource::Awaiter(
        static_cast<OneshotConditionVariable &>(*this).operator co_await());
}

template <class T>
struct FutureToken {
    FutureToken(FutureSource<T> const &that) noexcept
        : mImpl(that.mImpl.get()) {}

    template <class... Args>
    void set_value(Args &&...args) {
        mImpl->mValue.putValue(std::forward<Args>(args)...);
        mImpl->notify();
    }
#if CO_ASYNC_EXCEPT
    void set_exception(std::exception_ptr e) {
        mImpl->mException = e;
        mImpl->mCondition.notify();
    }
#endif
    auto operator co_await() const noexcept {
        return mImpl->makeAwaiter();
    }

private:
    typename FutureSource<T>::Impl *mImpl;
};
template <class T>
FutureToken(FutureSource<T> &) -> FutureToken<T>;

template <class T>
inline FutureToken<T> FutureSource<T>::token() const noexcept {
    return FutureToken<T>(*this);
}

template <class T>
inline Task<void, IgnoreReturnPromise<AutoDestroyFinalAwaiter>>
coFutureHelper(FutureToken<T> future, Task<T> task) {
#if CO_ASYNC_EXCEPT
    try {
#endif
        future.set_value((co_await task, Void()));
#if CO_ASYNC_EXCEPT
    } catch (...) {
        future.set_exception(std::current_exception());
    }
#endif
}

template <class T>
inline FutureSource<T> co_future(Task<T> task) {
    FutureSource<T> future;
    auto wrapped = coFutureHelper(future.token(), std::move(task));
    auto coroutine = wrapped.get();
    GenericIOContext::instance->enqueueJob(coroutine);
    wrapped.release();
    return future;
}
} // namespace co_async








namespace co_async {
template <class T = void>
struct TaskGroup {
    std::vector<FutureSource<T>> mTasks;

    TaskGroup &add(FutureSource<T> future) {
        mTasks.push_back(std::move(future));
        return *this;
    }

    TaskGroup &add(Task<T> task) {
        add(co_future(std::move(task)));
        return *this;
    }

    template <class F, class... Args>
        requires(std::same_as<std::invoke_result_t<F, Args...>, Task<T>>)
    TaskGroup &add(F &&f, Args &&...args) {
        add(co_bind(std::forward<F>(f), std::forward<Args>(args)...));
        return *this;
    }

    Task<std::conditional_t<!std::is_void_v<T>, std::vector<T>, void>> wait() {
        auto ret = (co_await when_all(mTasks), Void());
        mTasks.clear();
        co_return Void() | std::move(ret);
    }

    auto operator co_await() {
        return TaskOwnedAwaiter(wait());
    }

    TaskGroup() = default;
    TaskGroup(TaskGroup &&) = default;
    TaskGroup &operator=(TaskGroup &&) = default;
};
} // namespace co_async









namespace co_async {
template <class T>
struct Queue {
private:
    RingQueue<T> mQueue;
    ConditionVariable mChanged;

public:
    explicit Queue(std::size_t size) : mQueue(size) {}

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mChanged;
        }
        mChanged.notify_one();
    }

    Task<T> pop(T value) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            co_await mChanged;
        }
    }
};

template <class T>
struct TimedQueue {
private:
    RingQueue<T> mQueue;
    TimedConditionVariable mChanged;

public:
    explicit TimedQueue(std::size_t size) : mQueue(size) {}

    Task<> push(T value) {
        while (!mQueue.push(std::move(value))) {
            co_await mChanged.wait();
        }
        mChanged.notify_one();
        co_return;
    }

    Task<Expected<>> push(T value,
                          std::chrono::steady_clock::duration timeout) {
        return push(std::move(value),
                    std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<>> push(T value,
                          std::chrono::steady_clock::time_point expires) {
        while (!mQueue.push(std::move(value))) {
            if (auto e = co_await mChanged.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
        mChanged.notify_one();
        co_return;
    }

    Task<T> pop() {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            co_await mChanged.wait();
        }
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::duration timeout) {
        return pop(std::chrono::steady_clock::now() + timeout);
    }

    Task<Expected<T>> pop(std::chrono::steady_clock::time_point expires) {
        while (true) {
            if (auto value = mQueue.pop()) {
                mChanged.notify_one();
                co_return std::move(*value);
            }
            if (auto e = co_await mChanged.wait(expires); e.has_error()) {
                co_return Unexpected{e.error()};
            }
        }
    }
};
} // namespace co_async








namespace co_async {
struct IOContextMT {
private:
    std::unique_ptr<IOContext[]> mWorkers;
    std::size_t mNumWorkers = 0;

public:
    explicit IOContextMT(std::in_place_t) {
        if (IOContextMT::instance) [[unlikely]] {
            throw std::logic_error(
                "each process may contain only one IOContextMT");
        }
        IOContextMT::instance = this;
    }

    ~IOContextMT() {
        IOContextMT::instance = nullptr;
    }

    explicit IOContextMT(PlatformIOContextOptions options = {},
                         std::size_t numWorkers = 0)
        : IOContextMT(std::in_place) {
        start(options, numWorkers);
    }

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

    static void start(PlatformIOContextOptions options = {},
                      std::size_t numWorkers = 0) {
        bool setAffinity;
        if (numWorkers == 0) {
            setAffinity = true;
            numWorkers = std::thread::hardware_concurrency();
            if (!numWorkers) [[unlikely]] {
                throw std::logic_error(
                    "failed to detect number of hardware threads");
            }
        } else {
            setAffinity = false;
        }
        instance->mWorkers = std::make_unique<IOContext[]>(numWorkers);
        instance->mNumWorkers = numWorkers;
        std::span<IOContext> peerSpan(instance->mWorkers.get(),
                                      instance->mNumWorkers);
        for (std::size_t i = 0; i < instance->mNumWorkers; ++i) {
            if (setAffinity) {
                options.threadAffinity = i;
            }
            instance->mWorkers[i].start(options, peerSpan);
        }
    }

    static void spawn(std::coroutine_handle<> coroutine) {
        instance->mWorkers[0].spawn(coroutine);
    }

    static void spawn_mt(std::coroutine_handle<> coroutine) {
        instance->mWorkers[0].spawn_mt(coroutine);
    }

    template <class T, class P>
    static T join(Task<T, P> task) {
        return instance->mWorkers[0].join(std::move(task));
    }

    static inline IOContextMT *instance;
};
} // namespace co_async




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
                    put("0123456789abcdef"[u & 0x0F]);
                    put("0123456789abcdef"[u >> 4]);
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
    NotImplemented,
};

inline std::error_category const &jsonCategory() {
    static struct : std::error_category {
        virtual char const *name() const noexcept {
            return "json";
        }

        virtual std::string message(int e) const {
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
                    hex |= (unsigned int)(c - '0');
                } else if ('a' <= c && c <= 'f') {
                    hex |= (unsigned int)(c - 'a' + 10);
                } else if ('A' <= c && c <= 'F') {
                    hex |= (unsigned int)(c - 'A' + 10);
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
                        str.push_back((char)(unsigned char)(hex));
                    } else if (hex <= 0x7FF) {
                        str.push_back((char)(unsigned char)(0xC0 | (hex >> 6)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0xFFFF) {
                        str.push_back(
                            (char)(unsigned char)(0xE0 | (hex >> 12)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | (hex & 0x3F)));
                    } else if (hex <= 0x10FFFF) {
                        str.push_back(
                            (char)(unsigned char)(0xF0 | (hex >> 18)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | ((hex >> 12) & 0x3F)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | ((hex >> 6) & 0x3F)));
                        str.push_back(
                            (char)(unsigned char)(0x80 | (hex & 0x3F)));
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
        std::visit([&](auto const &arg) { encoder->putValue(arg); }, value);
    }

    template <class T>
    static bool getValue(JsonValue::Union &data, T &value,
                         std::error_code &ec) {
        ec = make_error_code(JsonError::NotImplemented);
        return false;
        throw "TODO!! IMPLEMENT THIS!!";
        /* std::visit([&](auto const &arg) { */
        /*     using Arg = std::decay_t<decltype(arg)>; */
        /*     encoder->getValue(data, arg, ec); */
        /* }, data.inner); */
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
        return Unexpected{ec};
    }
    return std::move(value);
}

template <class T>
inline Expected<T> json_decode(std::string_view json) {
    T value{};
    std::error_code ec;
    if (!json_decode(json, value, ec)) [[unlikely]] {
        return Unexpected{ec};
    }
    return std::move(value);
}
} // namespace co_async




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
template <class T>
struct from_string_t;

template <>
struct from_string_t<std::string> {
    std::string operator()(std::string_view s) const {
        return std::string(s);
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
    void operator()(std::string &result, U &&value) const {
        to_string_t<std::decay_t<U>>()(result, std::forward<U>(value));
    }

    template <class U>
    std::string operator()(U &&value) const {
        std::string result;
        operator()(result, std::forward<U>(value));
        return result;
    }
};

template <>
struct to_string_t<std::string> {
    void operator()(std::string &result, std::string const &value) const {
        result.assign(value);
    }
};

template <>
struct to_string_t<std::string_view> {
    void operator()(std::string &result, std::string_view value) const {
        result.assign(value);
    }
};

template <std::integral T>
struct to_string_t<T> {
    void operator()(std::string &result, T value) const {
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
    void operator()(std::string &result, T value) const {
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

inline std::string lower_string(std::string_view s) {
    std::string ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'A' && c <= 'Z') {
            c += 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline std::string upper_string(std::string_view s) {
    std::string ret;
    ret.resize(s.size());
    std::transform(s.begin(), s.end(), ret.begin(), [](char c) {
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }
        return c;
    });
    return ret;
}

inline std::string trim_string(std::string_view s,
                               std::string_view trims = {" \t\r\n", 4}) {
    auto pos = s.find_first_not_of(trims);
    if (pos == std::string_view::npos) {
        return {};
    }
    auto end = s.find_last_not_of(trims);
    return std::string(s.substr(pos, end - pos + 1));
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

    std::vector<std::string> collect() const {
        std::vector<std::string> result;
        for (auto &&part: *this) {
            result.emplace_back(part);
        }
        return result;
    }

    template <std::size_t N>
        requires(N > 0)
    std::array<std::string, N> collect() const {
        std::array<std::string, N> result;
        std::size_t i = 0;
        for (auto it = begin(); it != end(); ++it, ++i) {
            if (i + 1 >= N) {
                result[i] = std::string(it.rest());
                break;
            }
            result[i] = std::string(*it);
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

struct [[nodiscard]] DirFilePath {
    DirFilePath(std::filesystem::path path) : mPath(path), mDirFd(AT_FDCWD) {}

    explicit DirFilePath(std::filesystem::path path, FileHandle const &dir)
        : mPath(path),
          mDirFd(dir.fileNo()) {}

    char const *c_str() const noexcept {
        return mPath.c_str();
    }

    std::filesystem::path const &path() const {
        return mPath;
    }

    int dir_file() const noexcept {
        return mDirFd;
    }

private:
    std::filesystem::path mPath;
    int mDirFd;
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
enum class OpenMode : int {
    Read = O_RDONLY | O_LARGEFILE | O_CLOEXEC,
    Write = O_WRONLY | O_TRUNC | O_CREAT | O_LARGEFILE | O_CLOEXEC,
    ReadWrite = O_RDWR | O_CREAT | O_LARGEFILE | O_CLOEXEC,
    Append = O_WRONLY | O_APPEND | O_CREAT | O_LARGEFILE | O_CLOEXEC,
    Directory = O_RDONLY | O_DIRECTORY | O_LARGEFILE | O_CLOEXEC,
};

inline std::filesystem::path make_path(std::string_view path) {
    return std::filesystem::path((char8_t const *)std::string(path).c_str());
}

template <std::convertible_to<std::string_view>... Ts>
    requires(sizeof...(Ts) >= 2)
inline std::filesystem::path make_path(Ts &&...chunks) {
    return (make_path(chunks) / ...);
}

inline Task<Expected<FileHandle>> fs_open(DirFilePath path, OpenMode mode,
                                          mode_t access = 0644) {
    int oflags = (int)mode;
    int fd = co_await expectError(co_await UringOp().prep_openat(
        path.dir_file(), path.c_str(), oflags, access));
    FileHandle file(fd);
    co_return file;
}

inline Task<Expected<>> fs_close(FileHandle file) {
    co_await expectError(co_await UringOp().prep_close(file.fileNo()));
    file.releaseFile();
    co_return {};
}

inline Task<Expected<>> fs_mkdir(DirFilePath path, mode_t access = 0755) {
    co_return expectError(
        co_await UringOp().prep_mkdirat(path.dir_file(), path.c_str(), access));
}

inline Task<Expected<>> fs_link(DirFilePath oldpath, DirFilePath newpath) {
    co_return expectError(
        co_await UringOp().prep_linkat(oldpath.dir_file(), oldpath.c_str(),
                                       newpath.dir_file(), newpath.c_str(), 0));
}

inline Task<Expected<>> fs_symlink(DirFilePath target, DirFilePath linkpath) {
    co_return expectError(co_await UringOp().prep_symlinkat(
        target.c_str(), linkpath.dir_file(), linkpath.c_str()));
}

inline Task<Expected<>> fs_unlink(DirFilePath path) {
    co_return expectError(
        co_await UringOp().prep_unlinkat(path.dir_file(), path.c_str(), 0));
}

inline Task<Expected<>> fs_rmdir(DirFilePath path) {
    co_return expectError(co_await UringOp().prep_unlinkat(
        path.dir_file(), path.c_str(), AT_REMOVEDIR));
}

inline Task<Expected<FileStat>>
fs_stat(DirFilePath path, unsigned int mask = STATX_BASIC_STATS | STATX_BTIME) {
    FileStat ret;
    co_await expectError(co_await UringOp().prep_statx(
        path.dir_file(), path.c_str(), 0, mask, ret.getNativeStatx()));
    co_return ret;
}

inline Task<Expected<std::uint64_t>> fs_stat_size(DirFilePath path) {
    FileStat ret;
    co_await expectError(co_await UringOp().prep_statx(
        path.dir_file(), path.c_str(), 0, STATX_SIZE, ret.getNativeStatx()));
    co_return ret.size();
}

inline Task<Expected<std::size_t>>
fs_read(FileHandle &file, std::span<char> buffer,
        std::uint64_t offset = (std::uint64_t)-1) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_read(file.fileNo(), buffer, offset));
}

inline Task<Expected<std::size_t>>
fs_write(FileHandle &file, std::span<char const> buffer,
         std::uint64_t offset = (std::uint64_t)-1) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_write(file.fileNo(), buffer, offset));
}

inline Task<Expected<>> fs_truncate(FileHandle &file, std::uint64_t size = 0) {
    co_return expectError(
        co_await UringOp().prep_ftruncate(file.fileNo(), (loff_t)size));
}

inline Task<Expected<std::size_t>>
fs_splice(FileHandle &fileIn, FileHandle &fileOut, std::size_t size,
          std::int64_t offsetIn = -1, std::int64_t offsetOut = -1) {
    co_return (std::size_t) co_await expectError(co_await UringOp().prep_splice(
        fileIn.fileNo(), offsetIn, fileOut.fileNo(), offsetOut, size, 0));
}

inline Task<Expected<std::size_t>> fs_getdents(FileHandle &dirFile,
                                               std::span<char> buffer) {
    int res = (int)getdents64(dirFile.fileNo(), buffer.data(), buffer.size());
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_return (std::size_t) co_await expectError(res);
}

inline Task<int> fs_nop() {
    co_return co_await UringOp().prep_nop();
}

inline Task<Expected<>> fs_cancel_fd(FileHandle &file) {
    co_return expectError(co_await UringOp().prep_cancel_fd(
        file.fileNo(), IORING_ASYNC_CANCEL_FD | IORING_ASYNC_CANCEL_ALL));
}
} // namespace co_async






namespace co_async {
inline constexpr std::size_t kStreamBufferSize = 8192;

struct Stream {
    virtual void raw_timeout(std::chrono::steady_clock::duration timeout) {}

    virtual Task<Expected<>> raw_seek(std::uint64_t pos) {
        co_return Unexpected{std::make_error_code(std::errc::invalid_seek)};
    }

    virtual Task<Expected<>> raw_flush() {
        co_return {};
    }

    virtual Task<> raw_close() {
        co_return;
    }

    virtual Task<Expected<std::size_t>> raw_read(std::span<char> buffer) {
        co_return Unexpected{std::make_error_code(std::errc::not_supported)};
    }

    virtual Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) {
        co_return Unexpected{std::make_error_code(std::errc::not_supported)};
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
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        char c = mInBuffer[mInIndex];
        ++mInIndex;
        co_return c;
    }

    Task<Expected<>> getline(std::string &s, char eol) {
        std::size_t start = mInIndex;
        while (true) {
            for (std::size_t i = start; i < mInEnd; ++i) {
                if (mInBuffer[i] == eol) {
                    s.append(mInBuffer.get() + start, i - start);
                    mInIndex = i + 1;
                    co_return {};
                }
            }
            s.append(mInBuffer.get() + start, mInEnd - start);
            mInIndex = 0;
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
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getline(std::string &s, std::string_view eol) {
    again:
        co_await co_await getline(s, eol.front());
        for (std::size_t i = 1; i < eol.size(); ++i) {
            if (bufempty()) {
                mInIndex = 0;
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
                mInIndex = 0;
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

    Task<Expected<std::string>> getline(char eol) {
        std::string s;
        co_await co_await getline(s, eol);
        co_return s;
    }

    Task<Expected<std::string>> getline(std::string_view eol) {
        std::string s;
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
                p = std::copy(mInBuffer.get() + start, mInBuffer.get() + end,
                              p);
                mInIndex = end;
                co_return {};
            }
            p = std::copy(mInBuffer.get() + start, mInBuffer.get() + mInEnd, p);
            mInIndex = 0;
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
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<>> getn(std::string &s, std::size_t n) {
        auto start = mInIndex;
        while (true) {
            auto end = start + n;
            if (end <= mInEnd) {
                s.append(mInBuffer.get() + mInIndex, n);
                mInIndex = end;
                co_return {};
            }
            auto m = mInEnd - mInIndex;
            n -= m;
            s.append(mInBuffer.get() + mInIndex, m);
            mInIndex = 0;
            co_await co_await fillbuf();
            start = 0;
        }
    }

    Task<Expected<std::string>> getn(std::size_t n) {
        std::string s;
        s.reserve(n);
        co_await co_await getn(s, n);
        co_return s;
    }

    Task<> dropall() {
        do {
            mInIndex = 0;
        } while (co_await fillbuf());
    }

    Task<> getall(std::string &s) {
        std::size_t start = mInIndex;
        do {
            s.append(mInBuffer.get() + start, mInEnd - start);
            start = 0;
            mInIndex = 0;
        } while (co_await fillbuf());
    }

    Task<std::string> getall() {
        std::string s;
        co_await getall(s);
        co_return s;
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<>> getstruct(T &ret) {
        return getspan(std::span<char>((char *)&ret, sizeof(T)));
    }

    template <class T>
        requires std::is_trivial_v<T>
    Task<Expected<T>> getstruct() {
        T ret;
        co_await co_await getstruct(ret);
        co_return ret;
    }

    std::span<char const> peekbuf() const noexcept {
        return {mInBuffer.get() + mInIndex, mInEnd - mInIndex};
    }

    void seenbuf(std::size_t n) noexcept {
        mInIndex += n;
    }

    Task<Expected<std::string>> getchunk() noexcept {
        if (bufempty()) {
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        auto buf = peekbuf();
        std::string ret(buf.data(), buf.size());
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
            mInIndex = 0;
            co_await co_await fillbuf();
        }
        co_return mInBuffer[mInIndex];
    }

    Task<Expected<>> peekn(std::string &s, std::size_t n) {
        while (mInEnd - mInIndex < n) {
            co_await co_await fillbuf();
        }
        s.append(mInBuffer.get() + mInIndex, n);
        co_return {};
    }

    Task<Expected<std::string>> peekn(std::size_t n) {
        std::string s;
        co_await co_await peekn(s, n);
        co_return s;
    }

    void allocinbuf(std::size_t size = kStreamBufferSize) {
        if (!mInBuffer) [[likely]] {
            mInBuffer = std::make_unique<char[]>(size);
            mInBufSize = size;
            mInIndex = 0;
            mInEnd = 0;
        }
    }

    Task<Expected<>> fillbuf() {
        if (!mInBuffer) {
            allocinbuf();
        }
        auto n = co_await co_await mRaw->raw_read(
            std::span(mInBuffer.get() + mInIndex, mInBufSize - mInIndex));
        if (n == 0) [[unlikely]] {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
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
        if (std::size_t(pe - p) <= mOutBufSize - mOutIndex) {
            auto b = mOutBuffer.get() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
        } else {
            auto b = mOutBuffer.get() + mOutIndex;
            auto const be = mOutBuffer.get() + mOutBufSize;
            mOutIndex = mOutBufSize;
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
            allocoutbuf();
        }
        auto p = s.data();
        auto const pe = s.data() + s.size();
        auto nMax = mOutBufSize - mOutIndex;
        auto n = std::size_t(pe - p);
        if (n <= nMax) {
            auto b = mOutBuffer.get() + mOutIndex;
            mOutIndex += std::size_t(pe - p);
            while (p < pe) {
                *b++ = *p++;
            }
            return n;
        } else {
            auto b = mOutBuffer.get() + mOutIndex;
            auto const be = mOutBuffer.get() + mOutBufSize;
            mOutIndex = mOutBufSize;
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
        return putspan(
            std::span<char const>((char const *)std::addressof(s), sizeof(T)));
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

    void allocoutbuf(std::size_t size = kStreamBufferSize) {
        if (!mOutBuffer) [[likely]] {
            mOutBuffer = std::make_unique<char[]>(size);
            mOutBufSize = size;
            mOutIndex = 0;
        }
    }

    Task<Expected<>> flush() {
        if (!mOutBuffer) {
            allocoutbuf();
            co_return {};
        }
        if (mOutIndex) [[likely]] {
            auto buf = std::span(mOutBuffer.get(), mOutIndex);
            auto len = co_await mRaw->raw_write(buf);
            while (len.has_value() && *len > 0 && *len != buf.size()) {
                buf = buf.subspan(*len);
                len = co_await mRaw->raw_write(buf);
            }
            if (len.has_error()) [[unlikely]] {
                co_return Unexpected{len.error()};
            }
            if (*len == 0) [[unlikely]] {
                co_return Unexpected{
                    std::make_error_code(std::errc::broken_pipe)};
            }
            mOutIndex = 0;
            co_await co_await mRaw->raw_flush();
        }
        co_return {};
    }

    bool buffull() const noexcept {
        return mOutIndex == mOutBufSize;
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
            std::memcpy(buffer.data(), mInBuffer.get() + mInIndex, n);
            mInIndex += n;
            co_return n;
        }
        co_return co_await mRaw->raw_read(buffer);
    }

    Task<Expected<std::size_t>> read(void *buffer, std::size_t len) {
        return read(std::span<char>((char *)buffer, len));
    }

    std::size_t tryread(void *buffer, std::size_t len) {
        return tryread(std::span<char>((char *)buffer, len));
    }

    Task<Expected<std::size_t>> write(std::span<char const> buffer) {
        if (!buffull()) {
            auto n = std::min(mInBufSize - mInIndex, buffer.size());
            co_await co_await putspan(buffer.subspan(0, n));
            co_return n;
        }
        co_return co_await mRaw->raw_write(buffer);
    }

    Task<Expected<std::size_t>> write(void const *buffer, std::size_t len) {
        return write(std::span<char const>((char const *)buffer, len));
    }

    Task<Expected<>> putspan(void const *buffer, std::size_t len) {
        return putspan(std::span<char const>((char const *)buffer, len));
    }

    std::size_t trywrite(void const *buffer, std::size_t len) {
        return trywrite(std::span<char const>((char const *)buffer, len));
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
    std::unique_ptr<char[]> mInBuffer;
    std::size_t mInIndex = 0;
    std::size_t mInEnd = 0;
    std::size_t mInBufSize = 0;
    std::unique_ptr<char[]> mOutBuffer;
    std::size_t mOutIndex = 0;
    std::size_t mOutBufSize = 0;
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
Task<Expected<std::string>> file_read(std::filesystem::path path);
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
struct PipeHandlePair {
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

inline Task<Expected<PipeHandlePair>> fs_pipe() {
    int p[2];
    int res = pipe2(p, 0);
    if (res < 0) [[unlikely]] {
        res = -errno;
    }
    co_await expectError(res);
    co_return PipeHandlePair{FileHandle(p[0]), FileHandle(p[1])};
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
    co_await expectError(
        co_await UringOp().prep_waitid(P_PID, (id_t)pid, &info, options, 0));
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
    };
}

inline Task<Expected<WaitProcessResult>>
wait_process(Pid pid, std::chrono::steady_clock::duration timeout,
             int options = WEXITED) {
    siginfo_t info{};
    auto ts = durationToKernelTimespec(timeout);
    auto ret = expectError(co_await UringOp::link_ops(
        UringOp().prep_waitid(P_PID, (id_t)pid, &info, options, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    if (ret == std::make_error_code(std::errc::operation_canceled)) {
        co_return Unexpected{std::make_error_code(std::errc::stream_timeout)};
    }
    co_await std::move(ret);
    co_return WaitProcessResult{
        .pid = info.si_pid,
        .status = info.si_status,
        .exitType = (WaitProcessResult::ExitType)info.si_code,
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
            co_return Unexpected{std::make_error_code(std::errc(errno))};
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
        std::string name;
        name.reserve(mEventBuffer->len);
        co_await co_await mStream.getn(name, mEventBuffer->len);
        name = name.c_str();
        auto path = mWatches.at(mEventBuffer->wd);
        if (!name.empty()) {
            path /= make_path(name);
        }
        co_return WaitFileResult{
            .path = std::move(path),
            .event = (FileEvent)mEventBuffer->mask,
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






#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace co_async {
struct SignalingContextMT {
    static void startMain(std::stop_token stop) {
        while (!stop.stop_requested()) [[likely]] {
            sigset_t s;
            sigemptyset(&s);
            std::unique_lock lock(instance->mMutex);
            for (auto [signo, waiters]: instance->mWaitingSignals) {
                sigaddset(&s, signo);
            }
            lock.unlock();
            int signo;
            throwingError(-sigwait(&s, &signo));
            lock.lock();
            std::deque<std::coroutine_handle<>> waiters;
            waiters.swap(instance->mWaitingSignals.at(signo));
            lock.unlock();
            for (auto coroutine: waiters) {
                IOContextMT::spawn_mt(coroutine);
            }
        }
    }

    struct SignalAwaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coroutine) const {
            std::lock_guard lock(instance->mMutex);
            instance->mWaitingSignals[mSigno].push_back(coroutine);
        }

        void await_resume() const noexcept {}

        int mSigno;
    };

    static SignalAwaiter waitSignal(int signo) {
        return SignalAwaiter(signo);
    }

    static void start() {
        instance->mWorker =
            std::jthread([](std::stop_token stop) { startMain(stop); });
    }

    static inline SignalingContextMT *instance;

    SignalingContextMT() {
        if (instance) {
            throw std::logic_error(
                "each process may contain only one SignalingContextMT");
        }
        instance = this;
        start();
    }

    SignalingContextMT(SignalingContextMT &&) = delete;

    ~SignalingContextMT() {
        instance = nullptr;
    }

private:
    std::map<int, std::deque<std::coroutine_handle<>>> mWaitingSignals;
    std::mutex mMutex;
    std::jthread mWorker;
};
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

struct IpAddress {
    explicit IpAddress(struct in_addr const &addr) noexcept : mAddr(addr) {}

    explicit IpAddress(struct in6_addr const &addr6) noexcept : mAddr(addr6) {}

    static Expected<IpAddress> parse(std::string_view host,
                                     bool allowIpv6 = true);
    static Expected<IpAddress> parse(char const *host, bool allowIpv6 = true);
    std::string toString() const;

    auto repr() const {
        return toString();
    }

    std::variant<struct in_addr, struct in6_addr> mAddr;
};

struct SocketAddress {
    SocketAddress() = default;

    static Expected<SocketAddress> parse(std::string_view host,
                                         int defaultPort = -1);
    explicit SocketAddress(IpAddress ip, int port);

    union {
        struct sockaddr_in mAddrIpv4;
        struct sockaddr_in6 mAddrIpv6;
        struct sockaddr mAddr;
    };

    socklen_t mAddrLen;

    sa_family_t family() const noexcept;

    IpAddress host() const;

    int port() const;

    std::string toString() const;

    auto repr() const {
        return toString();
    }

private:
    void initFromHostPort(struct in_addr const &host, int port);
    void initFromHostPort(struct in6_addr const &host, int port);
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
        return Unexpected{e.error()};
    }
    return val;
}

template <class T>
Expected<> socketSetOption(SocketHandle &sock, int level, int opt,
                           T const &optVal) {
    return expectError(
        setsockopt(sock.fileNo(), level, opt, &optVal, sizeof(optVal)));
}

Task<Expected<SocketHandle>> createSocket(int family, int type);
Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr);
Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout);

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel);
Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog = SOMAXCONN);
Task<Expected<SocketListener>>
listener_bind(std::pair<std::string, int> const &addr, int backlog = SOMAXCONN);
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
Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf, CancelToken cancel);
Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel);
Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout);
Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout);
Task<Expected<>> socket_shutdown(SocketHandle &sock, int how = SHUT_RDWR);
} // namespace co_async







namespace co_async {
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest);
Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest);
} // namespace co_async








namespace co_async {
std::error_category const &bearSSLCategory();

StructPImpl(SSLClientTrustAnchor) {
    Expected<> add(std::string content);
};

StructPImpl(SSLServerPrivateKey){};

StructPImpl(SSLServerCertificate) {
    void add(std::string content);
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
            co_return Unexpected{std::make_error_code(std::errc::invalid_seek)};
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
    OStringStream(std::string &output) noexcept : mOutput(output) {}

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        mOutput.append(buffer.data(), buffer.size());
        co_return buffer.size();
    }

    std::string &str() const noexcept {
        return mOutput;
    }

    std::string release() noexcept {
        return std::move(mOutput);
    }

private:
    std::string &mOutput;
};
} // namespace co_async






namespace co_async {

Task<Expected<std::array<OwningStream, 2>>> pipe_stream();
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
} // namespace co_async






#include <dirent.h>

namespace co_async {
struct DirectoryWalker {
    explicit DirectoryWalker(FileHandle file);
    DirectoryWalker(DirectoryWalker &&) = default;
    DirectoryWalker &operator=(DirectoryWalker &&) = default;
    ~DirectoryWalker();
    Task<Expected<std::string>> next();

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
        auto ret = co_await socket_read(mFile, buffer, mTimeout);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::stream_timeout)};
        }
        co_return ret;
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        auto ret = co_await socket_write(mFile, buffer, mTimeout);
        if (ret == std::make_error_code(std::errc::operation_canceled))
            [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::stream_timeout)};
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
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(10);
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

inline Task<Expected<OwningStream>>
tcp_accept(SocketListener &listener,
           std::chrono::steady_clock::duration timeout) {
    auto handle = co_await co_await listener_accept(listener);
    OwningStream sock = make_stream<SocketStream>(std::move(handle));
    sock.timeout(timeout);
    co_return sock;
}
} // namespace co_async





namespace co_async {
struct URIParams : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
};

struct URI {
    std::string path;
    URIParams params;

public:
    static void url_decode(std::string &r, std::string_view s);
    static std::string url_decode(std::string_view s);
    static void url_encode(std::string &r, std::string_view s);
    static std::string url_encode(std::string_view s);
    static void url_encode_path(std::string &r, std::string_view s);
    static std::string url_encode_path(std::string_view s);
    static URI parse(std::string_view uri);
    void dump(std::string &r) const;
    std::string dump() const;

    std::string repr() const {
        return dump();
    }
};
} // namespace co_async





namespace co_async {
std::string timePointToHTTPDate(std::chrono::system_clock::time_point tp);
Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(std::string const &date);
std::string httpDateNow();
std::string_view getHTTPStatusName(int status);
std::string guessContentTypeByExtension(
    std::string_view ext, char const *defaultType = "text/plain;charset=utf-8");
std::string capitalizeHTTPHeader(std::string_view key);
} // namespace co_async









namespace co_async {
struct HTTPHeaders : SimpleMap<std::string, std::string> {
    using SimpleMap<std::string, std::string>::SimpleMap;
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
    std::string method{"GET", 3};
    URI uri{std::string{"/", 1}, {}};
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
    virtual Task<Expected<>> readBody(std::string &body) = 0;
    virtual Task<Expected<>> writeRequest(HTTPRequest const &req) = 0;
    virtual Task<Expected<>> readRequest(HTTPRequest &req) = 0;
    virtual Task<Expected<>> writeResponse(HTTPResponse const &res) = 0;
    virtual Task<Expected<>> readResponse(HTTPResponse &res) = 0;
};

struct HTTPProtocolVersion11 : HTTPProtocol {
    using HTTPProtocol::HTTPProtocol;

protected:
    HTTPContentEncoding mContentEncoding;
    std::string mAcceptEncoding;
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
    Task<Expected<>> readChunkedString(std::string &body);
    Task<Expected<>> writeEncoded(BorrowedStream &body);
    Task<Expected<>> writeEncodedString(std::string_view body);
    Task<Expected<>> readEncoded(BorrowedStream &body);
    Task<Expected<>> readEncodedString(std::string &body);
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
    Task<Expected<>> readBody(std::string &body) override;
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
        Task<Expected<>> readRequestHeader();
        Task<Expected<std::string>> request_body();
        Task<Expected<>> request_body_stream(OwningStream &out);
        Task<Expected<>> response(HTTPResponse resp, std::string_view content);
        Task<Expected<>> response(HTTPResponse resp, OwningStream &body);

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
    HTTPServer();
    ~HTTPServer();
    HTTPServer(HTTPServer &&) = delete;
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
    static std::string html_encode(std::string_view str);
    static Task<Expected<>>
    make_ok_response(HTTPServer::IO &io, std::string_view body,
                     std::string contentType = "text/html;charset=utf-8");
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
        req.headers.insert("host"s, mHttpFactory->hostName());
        req.headers.insert("user-agent"s, "co_async/0.0.1"s);
        req.headers.insert("accept"s, "*/*"s);
#if CO_ASYNC_ZLIB
        req.headers.insert("accept-encoding"s, "deflate, gzip"s);
#else
        req.headers.insert("accept-encoding"s, "gzip"s);
#endif
    }

    Task<Expected<>> tryWriteRequestAndBody(HTTPRequest const &request,
                                            std::string_view body) {
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
        co_return Unexpected{ec};
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
    /*     co_return Unexpected{std::errc::connection_aborted}; */
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
            return Unexpected{
                std::make_error_code(std::errc::protocol_not_supported)};
        }
    }

    HTTPConnection() = default;

    Task<Expected<std::tuple<HTTPResponse, std::string>>>
    request(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        std::string body;
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBody(body);
        reset.neverMind();
        co_return std::tuple{std::move(res), std::move(body)};
    }

    Task<Expected<std::tuple<HTTPResponse, OwningStream>>>
    requestStreamed(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        std::string body;
        co_await co_await mHttp->readResponse(res);
        auto [r, w] = co_await co_await pipe_stream();
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
        TimedConditionVariable mFreeSlot;

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
        std::chrono::steady_clock::duration timeout = std::chrono::seconds(5),
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
                        return Unexpected{e.error()};
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
            (void)co_await pool->mFreeSlot.wait(std::chrono::milliseconds(100));
        }
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


































































#ifdef CO_ASYNC_IMPLEMENTATION





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
    auto cv = std::make_shared<ConditionVariable>();
    std::exception_ptr ep;
    submitJob(
        [cv, ctx = IOContext::instance, func = std::move(func), &ep]() mutable {
            try {
                func();
            } catch (...) {
                ep = std::current_exception();
            }
            if (auto coroutine = cv->notify_pop_coroutine()) [[likely]] {
                ctx->spawn_mt(coroutine);
            }
        });
    co_await *cv;
    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    co_return {};
}

Task<Expected<>> ThreadPool::rawRun(std::function<void(std::stop_token)> func,
                                    CancelToken cancel) {
    auto cv = std::make_shared<ConditionVariable>();
    std::stop_source stop;
    bool stopped = false;
    std::exception_ptr ep;
    submitJob([cv, ctx = IOContext::instance, func = std::move(func),
               stop = stop.get_token(), &ep]() mutable {
        try {
            func(stop);
        } catch (...) {
            ep = std::current_exception();
        }
        if (auto p = cv->notify_pop_coroutine()) [[likely]] {
            ctx->spawn_mt(p);
        }
    });

    struct Awaitable {
        ConditionVariable &cv;
        std::stop_source &stop;
        bool &stopped;

        auto operator co_await() const noexcept {
            return cv.operator co_await();
        }

        struct Canceller {
            using OpType = Awaitable;

            static Task<> doCancel(OpType *op) {
                op->stopped = true;
                op->stop.request_stop();
                co_return;
            }
        };
    };

    co_await cancel.guard(Awaitable(*cv, stop, stopped));
    if (ep) [[unlikely]] {
        std::rethrow_exception(ep);
    }
    if (stopped) {
        co_return Unexpected{
            std::make_error_code(std::errc::operation_canceled)};
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

PlatformIOContext::PlatformIOContext(std::size_t entries) {
    throwingError(io_uring_queue_init((unsigned int)entries, &mRing, 0));
}

PlatformIOContext::~PlatformIOContext() {
    io_uring_queue_exit(&mRing);
}

thread_local PlatformIOContext *PlatformIOContext::instance;

bool PlatformIOContext::waitEventsFor(
    std::size_t numBatch,
    std::optional<std::chrono::steady_clock::duration> timeout) {
    struct io_uring_cqe *cqe;
    struct __kernel_timespec ts, *tsp;
    if (timeout) {
        tsp = &(ts = durationToKernelTimespec(*timeout));
    } else {
        tsp = nullptr;
    }
    int res = io_uring_submit_and_wait_timeout(
        &mRing, &cqe, (unsigned int)numBatch, tsp, nullptr);
    if (res == -EINTR) [[unlikely]] {
        return false;
    }
    if (res == -ETIME) {
        return false;
    }
    throwingError(res);
    unsigned head, numGot = 0;
    io_uring_for_each_cqe(&mRing, head, cqe) {
        auto *op = reinterpret_cast<UringOp *>(cqe->user_data);
        op->mRes = cqe->res;
        GenericIOContext::instance->enqueueJob(op->mPrevious);
        ++numGot;
    }
    io_uring_cq_advance(&mRing, numGot);
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
    static struct : std::error_category {
        virtual char const *name() const noexcept {
            return "getaddrinfo";
        }

        virtual std::string message(int e) const {
            return gai_strerror(e);
        }
    } instance;

    return instance;
}

Expected<IpAddress> IpAddress::parse(std::string_view host, bool allowIpv6) {
    return parse(std::string(host).c_str(), allowIpv6);
}

Expected<IpAddress> IpAddress::parse(char const *host, bool allowIpv6) {
    struct in_addr addr = {};
    struct in6_addr addr6 = {};
    if (1 == inet_pton(AF_INET, host, &addr)) {
        return IpAddress(addr);
    }
    if (allowIpv6 && 1 == inet_pton(AF_INET6, host, &addr6)) {
        return IpAddress(addr6);
    }
    // gethostbyname is deprecated, let's use getaddrinfo instead:
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *result;
    int err = getaddrinfo(host, NULL, &hints, &result);
    if (err) [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << ip << ": " << gai_strerror(err) << '\n';
#endif
        return Unexpected{std::error_code(err, getAddrInfoCategory())};
    }
    Finally fin = [&] {
        freeaddrinfo(result);
    };
    for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            std::memcpy(&addr, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
                        sizeof(in_addr));
            return IpAddress(addr);
        } else if (allowIpv6 && rp->ai_family == AF_INET6) {
            std::memcpy(&addr6,
                        &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr,
                        sizeof(in6_addr));
            return IpAddress(addr6);
        }
    }
    [[unlikely]] {
#if CO_ASYNC_DEBUG
        std::cerr << ip << ": no matching host address with ipv4 or ipv6\n";
#endif
        return Unexpected{std::make_error_code(std::errc::bad_address)};
    }
}

std::string IpAddress::toString() const {
    if (mAddr.index() == 1) {
        char buf[INET6_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET6, &std::get<1>(mAddr), buf, sizeof(buf));
        return buf;
    } else if (mAddr.index() == 0) {
        char buf[INET_ADDRSTRLEN + 1] = {};
        inet_ntop(AF_INET, &std::get<0>(mAddr), buf, sizeof(buf));
        return buf;
    } else {
        return "[invalid ip address or domain name]";
    }
}

Expected<SocketAddress> SocketAddress::parse(std::string_view host,
                                             int defaultPort) {
    auto pos = host.rfind(':');
    std::string hostPart(host);
    std::optional<int> port;
    if (pos != std::string_view::npos) {
        hostPart = host.substr(0, pos);
        port = from_string<int>(host.substr(pos + 1));
        if (port < 0 || port > 65535) [[unlikely]] {
            port = std::nullopt;
        }
    }
    if (!port) {
        if (defaultPort == -1) [[unlikely]] {
            return Unexpected{std::make_error_code(std::errc::bad_address)};
        }
        port = defaultPort;
    }
    auto ip = IpAddress::parse(hostPart.c_str());
    if (ip.has_error()) [[unlikely]] {
        return Unexpected{ip.error()};
    }
    return SocketAddress(*ip, *port);
}

SocketAddress::SocketAddress(IpAddress ip, int port) {
    std::visit([&](auto const &addr) { initFromHostPort(addr, port); },
               ip.mAddr);
}

sa_family_t SocketAddress::family() const noexcept {
    return mAddr.sa_family;
}

IpAddress SocketAddress::host() const {
    if (family() == AF_INET) {
        return IpAddress(mAddrIpv4.sin_addr);
    } else if (family() == AF_INET6) {
        return IpAddress(mAddrIpv6.sin6_addr);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

int SocketAddress::port() const {
    if (family() == AF_INET) {
        return ntohs(mAddrIpv4.sin_port);
    } else if (family() == AF_INET6) {
        return ntohs(mAddrIpv6.sin6_port);
    } else [[unlikely]] {
        throw std::runtime_error("address family not ipv4 or ipv6");
    }
}

std::string SocketAddress::toString() const {
    return host().toString() + ":" + to_string(port());
}

void SocketAddress::initFromHostPort(struct in_addr const &host, int port) {
    struct sockaddr_in saddr = {};
    saddr.sin_family = AF_INET;
    std::memcpy(&saddr.sin_addr, &host, sizeof(saddr.sin_addr));
    saddr.sin_port = htons((uint16_t)port);
    std::memcpy(&mAddrIpv4, &saddr, sizeof(saddr));
    mAddrLen = sizeof(saddr);
}

void SocketAddress::initFromHostPort(struct in6_addr const &host, int port) {
    struct sockaddr_in6 saddr = {};
    saddr.sin6_family = AF_INET6;
    std::memcpy(&saddr.sin6_addr, &host, sizeof(saddr.sin6_addr));
    saddr.sin6_port = htons((uint16_t)port);
    std::memcpy(&mAddrIpv6, &saddr, sizeof(saddr));
    mAddrLen = sizeof(saddr);
}

SocketAddress get_socket_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddrIpv6);
    throwingErrorErrno(
        getsockname(sock.fileNo(), (sockaddr *)&sa.mAddr, &sa.mAddrLen));
    return sa;
}

SocketAddress get_socket_peer_address(SocketHandle &sock) {
    SocketAddress sa;
    sa.mAddrLen = sizeof(sa.mAddrIpv6);
    throwingErrorErrno(
        getpeername(sock.fileNo(), (sockaddr *)&sa.mAddr, &sa.mAddrLen));
    return sa;
}

Task<Expected<SocketHandle>> createSocket(int family, int type) {
    int fd = co_await expectError(
        co_await UringOp().prep_socket(family, type, 0, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    co_await expectError(co_await UringOp().prep_connect(
        sock.fileNo(), (const struct sockaddr *)&addr.mAddr, addr.mAddrLen));
    co_return sock;
}

Task<Expected<SocketHandle>>
socket_connect(SocketAddress const &addr,
               std::chrono::steady_clock::duration timeout) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    auto ts = durationToKernelTimespec(timeout);
    co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_connect(
            sock.fileNo(), (const struct sockaddr *)&addr.mAddr, addr.mAddrLen),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
    co_return sock;
}

Task<Expected<SocketHandle>> socket_connect(SocketAddress const &addr,
                                            CancelToken cancel) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    if (cancel.is_canceled()) [[unlikely]] {
        co_return Unexpected{
            std::make_error_code(std::errc::operation_canceled)};
    }
    co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_connect(sock.fileNo(),
                               (const struct sockaddr *)&addr.mAddr,
                               addr.mAddrLen)));
    co_return sock;
}

Task<Expected<SocketListener>> listener_bind(SocketAddress const &addr,
                                             int backlog) {
    SocketHandle sock =
        co_await co_await createSocket(addr.family(), SOCK_STREAM);
    co_await socketSetOption(sock, SOL_SOCKET, SO_REUSEADDR, 1);
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_CORK, 0); */
    /* co_await socketSetOption(sock, IPPROTO_TCP, TCP_NODELAY, 1); */
    /* co_await socketSetOption(sock, SOL_SOCKET, SO_KEEPALIVE, 1); */
    SocketListener serv(sock.releaseFile());
    co_await expectError(bind(
        serv.fileNo(), (struct sockaddr const *)&addr.mAddr, addr.mAddrLen));
    co_await expectError(listen(serv.fileNo(), backlog));
    co_return serv;
}

Task<Expected<SocketListener>>
listener_bind(std::pair<std::string, int> const &addr, int backlog) {
    co_return co_await listener_bind(
        co_await SocketAddress::parse(addr.first, addr.second));
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener) {
    int fd = co_await expectError(
        co_await UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             CancelToken cancel) {
    int fd = co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_accept(listener.fileNo(), nullptr, nullptr, 0)));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr) {
    int fd = co_await expectError(co_await UringOp().prep_accept(
        listener.fileNo(), (struct sockaddr *)&peerAddr.mAddr,
        &peerAddr.mAddrLen, 0));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<SocketHandle>> listener_accept(SocketListener &listener,
                                             SocketAddress &peerAddr,
                                             CancelToken cancel) {
    int fd = co_await expectError(co_await cancel.guard<UringOpCanceller>(
        UringOp().prep_accept(listener.fileNo(),
                              (struct sockaddr *)&peerAddr.mAddr,
                              &peerAddr.mAddrLen, 0)));
    SocketHandle sock(fd);
    co_return sock;
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_send(sock.fileNo(), buf, 0));
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock,
                                        std::span<char> buf) {
    co_return (std::size_t) co_await expectError(
        co_await UringOp().prep_recv(sock.fileNo(), buf, 0));
}

Task<Expected<std::size_t>> socket_write(SocketHandle &sock,
                                         std::span<char const> buf,
                                         CancelToken cancel) {
    co_return (std::size_t) co_await expectError(
        co_await cancel.guard<UringOpCanceller>(
            UringOp().prep_send(sock.fileNo(), buf, 0)));
}

Task<Expected<std::size_t>> socket_read(SocketHandle &sock, std::span<char> buf,
                                        CancelToken cancel) {
    co_return (std::size_t) co_await expectError(
        co_await cancel.guard<UringOpCanceller>(
            UringOp().prep_recv(sock.fileNo(), buf, 0)));
}

Task<Expected<std::size_t>>
socket_write(SocketHandle &sock, std::span<char const> buf,
             std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return (std::size_t) co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_send(sock.fileNo(), buf, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
}

Task<Expected<std::size_t>>
socket_read(SocketHandle &sock, std::span<char> buf,
            std::chrono::steady_clock::duration timeout) {
    auto ts = durationToKernelTimespec(timeout);
    co_return (std::size_t) co_await expectError(co_await UringOp::link_ops(
        UringOp().prep_recv(sock.fileNo(), buf, 0),
        UringOp().prep_link_timeout(&ts, IORING_TIMEOUT_BOOTTIME)));
}

Task<Expected<>> socket_shutdown(SocketHandle &sock, int how) {
    co_return expectError(co_await UringOp().prep_shutdown(sock.fileNo(), how));
}
} // namespace co_async

#include <bearssl.h>












namespace co_async {
std::error_category const &bearSSLCategory() {
    static struct : std::error_category {
        virtual char const *name() const noexcept {
            return "BearSSL";
        }

        virtual std::string message(int e) const {
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
                {0, 0},
            };
            std::size_t u;
            for (u = 0; errors[u].second; u++) {
                if (errors[u].first == e) {
                    return errors[u].second;
                }
            }
            return to_string(e);
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

    SSLPemDecoder &decode(std::string_view s) {
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
                throw std::runtime_error("PEM decoder error");
            }
            s.remove_prefix(n);
        }
        return *this;
    }

    std::vector<std::pair<std::string, std::string>> const &objects() const {
        return objs;
    }

    static std::vector<std::string> tryDecode(std::string_view s) {
        std::vector<std::string> res;
        if (s.find("-----BEGIN ") != s.npos) {
            SSLPemDecoder dec;
            dec.decode(s);
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
            return Unexpected{std::error_code(err, bearSSLCategory())};
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

    SSLServerPrivateKey &set(std::string_view pkey) {
        for (auto &s: SSLPemDecoder::tryDecode(pkey)) {
            decodeBinary(s);
        }
        return *this;
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
            return Unexpected{dn.error()};
        }
        trustAnchors.push_back({
            {(unsigned char *)dn->data(), dn->size()},
            BR_X509_TA_CA,
            *x506.getPubKey(),
        });
        return {};
    }

    Expected<> add(std::string_view certX506) {
        for (auto &s: SSLPemDecoder::tryDecode(certX506)) {
            if (auto e = addBinary(s); e.has_error()) {
                return e;
            }
        }
        return {};
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
        certificates.push_back({(unsigned char *)cert.data(), cert.size()});
    }

    void add(std::string_view certX506) {
        for (auto &s: SSLPemDecoder::tryDecode(certX506)) {
            addBinary(s);
        }
    }

private:
    std::vector<std::string> strStores;
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
                    co_return Unexpected{
                        std::error_code(err, bearSSLCategory())};
                }
                co_return {};
            }
            if (state & BR_SSL_SENDREC) {
                unsigned char *buf;
                std::size_t len, wlen;
                buf = br_ssl_engine_sendrec_buf(eng, &len);
                if (auto e = co_await raw.raw_write({(char const *)buf, len});
                    e && *e != 0) {
                    wlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return Unexpected{e.error()};
                        } else {
                            co_return Unexpected{
                                std::make_error_code(std::errc::broken_pipe)};
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return Unexpected{e.error()};
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
                throw std::runtime_error("SSL write not ready");
            }
            if (state & BR_SSL_RECVREC) {
                unsigned char *buf;
                std::size_t len, rlen;
                buf = br_ssl_engine_recvrec_buf(eng, &len);
                if (auto e = co_await raw.raw_read({(char *)buf, len});
                    e && *e != 0) {
                    rlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return Unexpected{e.error()};
                        } else {
                            co_return Unexpected{
                                std::make_error_code(std::errc::broken_pipe)};
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return Unexpected{e.error()};
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
                "invalid private key type, must be RSA or EC");
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
Expected<> ForwardPImplMethod(SSLClientTrustAnchor, add, (std::string content),
                              content);
DefinePImpl(SSLServerCertificate);
void ForwardPImplMethod(SSLServerCertificate, add, (std::string content),
                        content);
DefinePImpl(SSLServerSessionCache);
} // namespace co_async








namespace co_async {

namespace {

struct PipeStreamBuffer {
    InfinityQueue<std::string> mChunks;
    ConditionVariable mNonEmpty;
};

struct IPipeStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        while (true) {
            if (auto chunk = mPipe->mChunks.pop()) {
                auto n = std::min(buffer.size(), chunk->size());
                std::memcpy(buffer.data(), chunk->data(), n);
                co_return n;
            }
            co_await mPipe->mNonEmpty;
        }
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
            p->mChunks.push(std::string(buffer.data(), buffer.size()));
            p->mNonEmpty.notify_one();
            co_return buffer.size();
        } else {
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
        }
    }

    Task<> raw_close() override {
        if (auto p = mPipe.lock()) {
            p->mChunks.push(std::string());
            p->mNonEmpty.notify_one();
        }
        co_return;
    }

    ~OPipeStream() override {
        if (auto p = mPipe.lock()) {
            p->mChunks.push(std::string());
            p->mNonEmpty.notify_one();
        }
    }

    explicit OPipeStream(std::weak_ptr<PipeStreamBuffer> buffer)
        : mPipe(std::move(buffer)) {}

private:
    std::weak_ptr<PipeStreamBuffer> mPipe;
};

} // namespace

Task<Expected<std::array<OwningStream, 2>>> pipe_stream() {
    auto pipePtr = std::make_shared<PipeStreamBuffer>();
    auto pipeWeakPtr = std::weak_ptr(pipePtr);
    co_return std::array{make_stream<IPipeStream>(std::move(pipePtr)),
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
            co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
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
        co_return co_await fs_read(mFile, buffer);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFile, buffer);
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

Task<Expected<std::string>> file_read(std::filesystem::path path) {
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
    void disableTTYCanonAndEcho() {
        if (isatty(mFileIn.fileNo())) {
            struct termios tc;
            tcgetattr(mFileIn.fileNo(), &tc);
            tc.c_lflag &= ~(tcflag_t)ICANON;
            tc.c_lflag &= ~(tcflag_t)ECHO;
            tcsetattr(mFileIn.fileNo(), TCSANOW, &tc);
        }
    }

    explicit StdioStream(FileHandle &fileIn, FileHandle &fileOut)
        : mFileIn(fileIn),
          mFileOut(fileOut) {}

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFileIn, buffer);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFileOut, buffer);
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
} // namespace

OwningStream &stdio() {
    static thread_local OwningStream s = make_stream<StdioStream>(
        stdFileHandle<STDIN_FILENO>(), stdFileHandle<STDOUT_FILENO>());
    return s;
}
} // namespace co_async






#include <dirent.h>

namespace co_async {
namespace {
struct DirectoryStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) {
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

Task<Expected<std::string>> DirectoryWalker::DirectoryWalker::next() {
    struct LinuxDirent64 {
        int64_t d_ino;           /* 64-bit inode number */
        int64_t d_off;           /* 64-bit offset to next structure */
        unsigned short d_reclen; /* Size of this dirent */
        unsigned char d_type;    /* File type */
    } dent;

    co_await co_await mStream.getspan(std::span<char>((char *)&dent, 19));
    std::string rest;
    rest.reserve(dent.d_reclen - 19);
    co_await co_await mStream.getn(rest, dent.d_reclen - 19);
    co_return std::string(rest.data());
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
        co_return Unexpected{
            std::make_error_code(std::errc::not_enough_memory)};
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: inflate source read failed with error\n";
# endif
            (void)inflateEnd(&strm);
            co_return Unexpected{e.error()};
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
                co_return Unexpected{std::make_error_code(std::errc::io_error)};
            }
            have = chunk - strm.avail_out;
            if (auto e = co_await dest.putspan(
                    std::span<char const>((char const *)out, have));
                e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: inflate dest write failed with error\n";
# endif
                (void)inflateEnd(&strm);
                co_return Unexpected{e.error()};
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: inflate got unexpected end of file\n";
# endif
        co_return Unexpected{std::make_error_code(std::errc::io_error)};
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
        co_return Unexpected{
            std::make_error_code(std::errc::not_enough_memory)};
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: deflate source read failed with error\n";
# endif
            (void)deflateEnd(&strm);
            co_return Unexpected{e.error()};
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
                co_return Unexpected{e.error()};
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */
    } while (flush != Z_FINISH);
    (void)deflateEnd(&strm);
    co_return {};
}
#else
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return Unexpected{
        std::make_error_code(std::errc::function_not_supported)};
}

Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return Unexpected{
        std::make_error_code(std::errc::function_not_supported)};
}
#endif
} // namespace co_async





namespace co_async {
namespace {
std::uint8_t fromHex(char c) {
    if ('0' <= c && c <= '9') {
        return (std::uint8_t)(c - '0');
    } else if ('A' <= c && c <= 'F') {
        return (std::uint8_t)(c - 'A' + 10);
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

void URI::url_decode(std::string &r, std::string_view s) {
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
        r.push_back((char)((fromHex(c1) << 4) | fromHex(c2)));
        b = i + 3;
    }
}

std::string URI::url_decode(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    url_decode(r, s);
    return r;
}

void URI::url_encode(std::string &r, std::string_view s) {
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

std::string URI::url_encode(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    url_encode(r, s);
    return r;
}

void URI::url_encode_path(std::string &r, std::string_view s) {
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

std::string URI::url_encode_path(std::string_view s) {
    std::string r;
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
                params.insert_or_assign(std::string(k), url_decode(v));
            }
        } while (i != std::string_view::npos);
    }
    std::string spath(path);
    if (spath.empty() || spath.front() != '/') [[unlikely]] {
        spath.insert(spath.begin(), '/');
    }
    return URI{spath, std::move(params)};
}

void URI::dump(std::string &r) const {
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

std::string URI::dump() const {
    std::string r;
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
    std::string line;
    while (true) {
        line.clear();
        co_await co_await sock.getline(line, "\r\n"sv);
        if (line.empty()) {
            break;
        }
        auto pos = line.find(':');
        if (pos == line.npos || pos == line.size() - 1 || line[pos + 1] != ' ')
            [[unlikely]] {
            co_return Unexpected{
                std::make_error_code(std::errc::protocol_error)};
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
    co_await co_await sock.puts("connection: keep-alive\r\n"sv);
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
    bool hadHeader = false;
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
            if (!hadHeader) {
                co_await co_await sock.puts(
                    "transfer-encoding: chunked\r\n\r\n"sv);
                hadHeader = true;
            }
            co_await co_await sock.puts(
                std::string_view{buf, static_cast<std::size_t>(ep - buf)});
            co_await co_await sock.putspan(bufSpan);
            co_await co_await sock.puts("\r\n"sv);
            co_await co_await sock.flush();
        }
    } while (co_await body.fillbuf());
    if (hadHeader) {
        co_await co_await sock.puts("0\r\n\r\n"sv);
    } else {
        co_await co_await sock.puts("\r\n"sv);
    }
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
            std::string line;
            co_await co_await sock.getn(line, n);
            co_await co_await body.puts(line);
            co_await co_await body.flush();
        }
    } else {
        std::string line;
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
            /* debug(), line; */
            co_await co_await body.puts(line);
            co_await co_await body.flush();
            co_await co_await sock.dropn(2);
        }
    }
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readChunkedString(std::string &body) {
    using namespace std::string_view_literals;
    if (mContentLength) {
        if (auto n = *mContentLength; n > 0) {
            co_await co_await sock.getn(body, n);
        }
    } else {
        std::string line;
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
        auto [r, w] = co_await co_await pipe_stream();
        TaskGroup<Expected<>> group;
        group.add([&body, w = std::move(w)]() mutable -> Task<Expected<>> {
            co_await co_await zlib_deflate(body, w);
            co_await co_await w.flush();
            co_await w.close();
            co_return {};
        });
        group.add(writeChunked(r));
        co_await co_await group.wait();
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
        TaskGroup<Expected<>> group;
        group.add(pipe_forward(body, pin));
        group.add(writeChunked(pout));
        co_await co_await group.wait();
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
        auto [r, w] = co_await co_await pipe_stream();
        TaskGroup<Expected<>> group;
        group.add(pipe_bind(std::move(w),
                            &std::decay_t<decltype(*this)>::readChunked, this));
        group.add([this, w = std::move(w)]() mutable -> Task<Expected<>> {
            co_await co_await readChunked(w);
            co_await co_await w.flush();
            co_await w.close();
            co_return {};
        });
        group.add(zlib_deflate(r, body));
        co_await co_await group.wait();
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
        TaskGroup<Expected<>> group;
        group.add([this, pin = std::move(pin)]() mutable -> Task<Expected<>> {
            co_await co_await readChunked(pin);
            co_await co_await pin.flush();
            co_await pin.close();
            co_return {};
        });
        group.add(pipe_forward(pout, body));
        co_await co_await group.wait();
        co_await co_await wait_process(pid);
    } break;
    };
    co_return {};
}

Task<Expected<>> HTTPProtocolVersion11::readEncodedString(std::string &body) {
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
            to_string(mPhase) + ", from = " + to_string(from) +
            ", to = " + to_string(to) + ")");
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

Task<Expected<>> HTTPProtocolVersion11::readBody(std::string &body) {
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
    std::string line;
    if (!co_await sock.getline(line, "\r\n"sv) || line.empty()) {
        co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
    }
    auto pos = line.find(' ');
    if (pos == line.npos || pos == line.size() - 1) [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc::protocol_error)};
    }
    req.method = line.substr(0, pos);
    auto pos2 = line.find(' ', pos + 1);
    if (pos2 == line.npos || pos2 == line.size() - 1) [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc::protocol_error)};
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
    std::string line;
    if (!co_await sock.getline(line, "\r\n"sv) || line.empty()) [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc::broken_pipe)};
    }
    if (line.size() <= 9 || line.substr(0, 7) != "HTTP/1."sv || line[8] != ' ')
        [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc::protocol_error)};
    }
    if (auto statusOpt = from_string<int>(line.substr(9, 3))) [[likely]] {
        res.status = *statusOpt;
    } else [[unlikely]] {
        co_return Unexpected{std::make_error_code(std::errc::protocol_error)};
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
std::string HTTPServerUtils::html_encode(std::string_view str) {
    std::string res;
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
                                                   std::string contentType) {
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
    auto dirPath = path.generic_string();
    std::string content = "<h1>Files in " + dirPath + ":</h1>";
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
    auto error =
        to_string(status) + " " + std::string(getHTTPStatusName(status));
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
std::string timePointToHTTPDate(std::chrono::system_clock::time_point tp) {
    // format chrono time point into HTTP date format, e.g.:
    // Tue, 30 Apr 2024 07:31:38 GMT
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

Expected<std::chrono::system_clock::time_point>
httpDateToTimePoint(std::string const &date) {
    std::tm tm = {};
    std::istringstream ss(date);
    ss.imbue(std::locale::classic());
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    if (ss.fail()) [[unlikely]] {
        return Unexpected{std::make_error_code(std::errc::invalid_argument)};
    }
    std::time_t time = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time);
}

std::string httpDateNow() {
    std::time_t time = std::time(nullptr);
    std::tm tm = *std::gmtime(&time);
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
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

std::string guessContentTypeByExtension(std::string_view ext,
                                        char const *defaultType) {
    using namespace std::string_view_literals;
    using namespace std::string_literals;
    if (ext == ".html"sv || ext == ".htm"sv) {
        return "text/html;charset=utf-8"s;
    } else if (ext == ".css"sv) {
        return "text/css;charset=utf-8"s;
    } else if (ext == ".js"sv) {
        return "application/javascript;charset=utf-8"s;
    } else if (ext == ".txt"sv || ext == ".md"sv) {
        return "text/plain;charset=utf-8"s;
    } else if (ext == ".json"sv) {
        return "application/json"s;
    } else if (ext == ".png"sv) {
        return "image/png"s;
    } else if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return "image/jpeg"s;
    } else if (ext == ".gif"sv) {
        return "image/gif"s;
    } else if (ext == ".xml"sv) {
        return "application/xml"s;
    } else if (ext == ".pdf"sv) {
        return "application/pdf"s;
    } else if (ext == ".mp4"sv) {
        return "video/mp4"s;
    } else if (ext == ".mp3"sv) {
        return "audio/mp3"s;
    } else if (ext == ".zip"sv) {
        return "application/zip"s;
    } else if (ext == ".svg"sv) {
        return "image/svg+xml"s;
    } else if (ext == ".wav"sv) {
        return "audio/wav"s;
    } else if (ext == ".ogg"sv) {
        return "audio/ogg"s;
    } else if (ext == ".mpg"sv || ext == ".mpeg"sv) {
        return "video/mpeg"s;
    } else if (ext == ".webm"sv) {
        return "video/webm"s;
    } else if (ext == ".ico"sv) {
        return "image/x-icon"s;
    } else if (ext == ".rar"sv) {
        return "application/x-rar-compressed"s;
    } else if (ext == ".7z"sv) {
        return "application/x-7z-compressed"s;
    } else if (ext == ".tar"sv) {
        return "application/x-tar"s;
    } else if (ext == ".gz"sv) {
        return "application/gzip"s;
    } else if (ext == ".bz2"sv) {
        return "application/x-bzip2"s;
    } else if (ext == ".xz"sv) {
        return "application/x-xz"s;
    } else if (ext == ".zip"sv) {
        return "application/zip"s;
    } else if (ext == ".tar.gz"sv || ext == ".tgz"sv) {
        return "application/tar+gzip"s;
    } else if (ext == ".tar.bz2"sv || ext == ".tbz2"sv) {
        return "application/tar+bzip2"s;
    } else if (ext == ".tar.xz"sv || ext == ".txz"sv) {
        return "application/tar+xz"s;
    } else if (ext == ".doc"sv || ext == ".docx"sv) {
        return "application/msword"s;
    } else if (ext == ".xls"sv || ext == ".xlsx"sv) {
        return "application/vnd.ms-excel"s;
    } else if (ext == ".ppt"sv || ext == ".pptx"sv) {
        return "application/vnd.ms-powerpoint"s;
    } else if (ext == ".csv"sv) {
        return "text/csv;charset=utf-8"s;
    } else if (ext == ".rtf"sv) {
        return "application/rtf"s;
    } else if (ext == ".exe"sv) {
        return "application/x-msdownload"s;
    } else if (ext == ".msi"sv) {
        return "application/x-msi"s;
    } else if (ext == ".bin"sv) {
        return "application/octet-stream"s;
    } else {
        return std::string(defaultType);
    }
}

std::string capitalizeHTTPHeader(std::string_view key) {
    // e.g.: user-agent -> User-Agent
    std::string result(key);
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
        std::vector<std::string> mMethods;

        bool checkMethod(std::string_view method) const {
            return std::find(mMethods.begin(), mMethods.end(), method) !=
                   mMethods.end();
        }
    };

    struct PrefixRoute {
        HTTPPrefixHandler mHandler;
        HTTPRouteMode mRouteMode;
        std::vector<std::string> mMethods;

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

    SimpleMap<std::string, Route> mRoutes;
    std::vector<std::pair<std::string, PrefixRoute>> mPrefixRoutes;
    HTTPHandler mDefaultRoute = [](IO &io) -> Task<Expected<>> {
        co_return co_await make_error_response(io, 404);
    };
    std::chrono::steady_clock::duration mTimeout = std::chrono::seconds(5);
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
                co_await co_await route.mHandler(io, suffix);
                co_return {};
            }
        }
        co_await co_await mDefaultRoute(io);
        co_return {};
    }
};

Task<Expected<>> HTTPServer::IO::readRequestHeader() {
    mHttp->initServerState();
    co_await co_await mHttp->readRequest(request);
    co_return {};
}

Task<Expected<std::string>> HTTPServer::IO::request_body() {
#if CO_ASYNC_DEBUG
    if (mBodyRead) [[unlikely]] {
        throw std::runtime_error("request_body() may only be called once");
    }
#endif
    mBodyRead = true;
    std::string body;
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
    using namespace std::string_literals;
    res.headers.insert("server"s, "co_async/0.0.1"s);
    res.headers.insert("accept"s, "*/*"s);
    res.headers.insert("accept-ranges"s, "bytes"s);
    res.headers.insert("date"s, httpDateNow());
}

HTTPServer::HTTPServer() : mImpl(std::make_unique<Impl>()) {}

HTTPServer::~HTTPServer() = default;

void HTTPServer::timeout(std::chrono::steady_clock::duration timeout) {
    mImpl->mTimeout = timeout;
}

void HTTPServer::route(std::string_view methods, std::string_view path,
                       HTTPHandler handler) {
    mImpl->mRoutes.insert_or_assign(
        std::string(path),
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
        {std::string(prefix),
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
    /* int h = handle.fileNo(); */
    co_await co_await doHandleConnection(
        co_await prepareHTTP(std::move(handle)));
    /* co_await UringOp().prep_shutdown(h, SHUT_RDWR); */
    co_return {};
}

Task<Expected<>>
HTTPServer::handle_http_redirect_to_https(SocketHandle handle) const {
    using namespace std::string_literals;
    auto http = co_await prepareHTTP(std::move(handle));
    while (true) {
        IO io(http.get());
        if (!co_await io.readRequestHeader()) {
            break;
        }
        if (auto host = io.request.headers.get("host")) {
            auto location = "https://"s + *host + io.request.uri.dump();
            HTTPResponse res = {
                .status = 302,
                .headers =
                    {
                        {"location", location},
                        {"content-type", "text/plain"},
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
        if (!co_await io.readRequestHeader()) {
            break;
        }
#if CO_ASYNC_DEBUG
        std::chrono::steady_clock::time_point t0;
        if (mLogRequests) {
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
        if (mLogRequests) {
            auto dt = std::chrono::steady_clock::now() - t0;
            std::clog << io.request.method + ' ' + io.request.uri.dump() + ' ' +
                             std::to_string(io.mResponseSavedForDebug.status) +
                             ' ' +
                             std::string(getHTTPStatusName(
                                 io.mResponseSavedForDebug.status)) +
                             ' ' +
                             std::to_string(std::chrono::duration_cast<
                                                std::chrono::milliseconds>(dt)
                                                .count()) +
                             "ms\n";
            for (auto [k, v]: io.mResponseSavedForDebug.headers) {
                if (k == "cookie" || k == "set-cookie") {
                    v = "***";
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
    auto error =
        to_string(status) + " " + std::string(getHTTPStatusName(status));
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
    if (proxy.starts_with("http://")) {
        proxy.remove_prefix(7);
        auto sock = co_await co_await socket_connect(
            co_await SocketAddress::parse(proxy, 80));
        auto hostName = std::string(host) + ":" + to_string(port);
        std::string header = "CONNECT " + hostName +
                             " HTTP/1.1\r\nHost: " + hostName +
                             "\r\nProxy-Connection: Keep-Alive\r\n\r\n";
        std::span<char const> buf = header;
        std::size_t n = 0;
        do {
            n = co_await co_await socket_write(sock, buf, timeout);
            if (!n) [[unlikely]] {
                co_return Unexpected{
                    std::make_error_code(std::errc::connection_reset)};
            }
            buf = buf.subspan(n);
        } while (buf.size() > 0);
        using namespace std::string_view_literals;
        auto desiredResponse = "HTTP/1.1 200 Connection established\r\n\r\n"sv;
        std::string response(39, '\0');
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
                co_return Unexpected{
                    std::make_error_code(std::errc::connection_reset)};
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
            co_return Unexpected{
                std::make_error_code(std::errc::connection_reset)};
        }
        co_return sock;
    } else {
#if CO_ASYNC_DEBUG
        if (!proxy.empty()) {
            std::cerr << "WARNING: unsupported proxy scheme [" + proxy + "]\n";
        }
#endif
        co_return co_await socket_connect(co_await SocketAddress::parse(proxy));
    }
}
} // namespace co_async

#endif

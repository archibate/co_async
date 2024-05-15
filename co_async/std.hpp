#pragma once
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
#define DEBUG_LEVEL 1
#include <co_async/utils/debug.hpp>
#include <co_async/utils/reflect.hpp>

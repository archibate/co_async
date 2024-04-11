#pragma once

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>

#if __cplusplus >= 201103L
#include <ccomplex>
#include <cfenv>
#include <cinttypes>
#if __has_include(<cstdalign>)
#include <cstdalign>
#endif
#include <cstdbool>
#include <cstdint>
#include <ctgmath>
#include <cuchar>
#endif

// C++
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

#if __cplusplus >= 201103L
#include <array>
#include <atomic>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#endif

#if __cplusplus >= 201402L
#include <shared_mutex>
#endif

#if __cplusplus >= 201703L
#include <any>
#include <charconv>
#include <execution>
#include <filesystem>
#include <optional>
#include <memory_resource>
#include <string_view>
#include <variant>
#endif

#if __cplusplus >= 202002L
#if __has_include(<barrier>)
#include <barrier>
#endif
#include <bit>
#include <compare>
#include <concepts>
#if __cpp_impl_coroutine
# include <coroutine>
#endif
#if __has_include(<latch>)
#include <latch>
#endif
#include <numbers>
#include <ranges>
#include <span>
#if __has_include(<stop_token>)
#include <stop_token>
#endif
#if __has_include(<semaphore>)
#include <semaphore>
#endif
#if __has_include(<source_location>)
#include <source_location>
#endif
#if __has_include(<syncstream>)
#include <syncstream>
#endif
#include <version>
#endif

#if __cplusplus > 202002L
#if __has_include(<expected>)
#include <expected>
#endif
#if __has_include(<spanstream>)
#include <spanstream>
#endif
#if __has_include(<stacktrace>)
# include <stacktrace>
#endif
#if __has_include(<stdatomic.h>)
#include <stdatomic.h>
#endif
#endif

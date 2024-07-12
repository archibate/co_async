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

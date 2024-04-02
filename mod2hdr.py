import re
import os

dir = './co_async/'
out = 'co_async.hpp'

modules = {}
modules['std'] = {'dependencies': [], 'source': '''
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
// #include <execution>
#include <filesystem>
#include <optional>
#include <memory_resource>
#include <string_view>
#include <variant>
#endif

#if __cplusplus >= 202002L
#include <barrier>
#include <bit>
#include <compare>
#include <concepts>
#if __cpp_impl_coroutine
# include <coroutine>
#endif
#include <latch>
#include <numbers>
#include <ranges>
#include <span>
#if __has_include(<stop_token>)
#include <stop_token>
#endif
#include <semaphore>
#include <source_location>
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
'''}

GLOBAL_MODULE_FRAGMENT = re.compile(r'^\s*module;$')
IMPORT_STD = re.compile(r'^\s*import\s+std;$')
EXPORT_MODULE = re.compile(r'^\s*export\s+module\s+([a-zA-Z0-9_.]+)(:[a-zA-Z0-9_.]+)?;$')
IMPORT = re.compile(r'^\s*(export\s+)?import\s+(:)?([a-zA-Z0-9_.]+);$')
EXPORT = re.compile(r'^(\s*export\s+)')

# walk dir
for dirpath, dirnames, filenames in os.walk(dir):
    for filename in filenames:
        filepath = os.path.join(dirpath, filename)
        if filepath.endswith('.cpp'):
            with open(filepath, 'r') as f:
                current = None
                parent = None
                this_module = {'dependencies': [], 'source': ''}
                this_module['source'] += '/// ' + filepath + '\n'
                for line in f:
                    line = line.removesuffix('\n')
                    if 0: pass
                    elif m := re.match(GLOBAL_MODULE_FRAGMENT, line):
                        continue
                    elif m := re.match(EXPORT_MODULE, line):
                        parent, partition = m.groups()
                        if partition:
                            current = parent + partition
                        else:
                            current = parent
                        continue
                    elif m := re.match(IMPORT_STD, line):
                        this_module['dependencies'].append('std')
                        continue
                    elif m := re.match(IMPORT, line):
                        export, colon, partition = m.groups()
                        if colon:
                            assert parent
                            dependency = parent + ':' + partition
                        else:
                            dependency = partition
                        this_module['dependencies'].append(dependency)
                        continue
                    elif m := re.match(EXPORT, line):
                        line = line.removeprefix(m.group(0))
                    this_module['source'] += line + '\n'
                if current:
                    modules[current] = this_module


visited = set()
result = []

def dump(current):
    if current in visited:
        return
    visited.add(current)
    module = modules[current]
    for dependency in module['dependencies']:
        dump(dependency)
    result.append(module['source'])

for m in modules:
    dump(m)

source = '\n'.join(result)
source = '''#pragma once
/// Automatically Generated (DO NOT EDIT THIS FILE)
/// Source: https://github.com/archibate/co_async
{}
'''.format(source)
with open(out, 'w') as f:
    f.write(source)

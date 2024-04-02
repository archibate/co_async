#!/usr/bin/env python

headers = [
"assert.h",
"ctype.h",
"errno.h",
"limits.h",
"locale.h",
"math.h",
"setjmp.h",
"signal.h",
"stddef.h",
"stdio.h",
"stdlib.h",
"string.h",
"time.h",
"wchar.h",
"wctype.h",
"complex.h",
"fenv.h",
"inttypes.h",
"stdbool.h",
"stdint.h",
"uchar.h",
"cassert",
"cctype",
"cerrno",
"cfloat",
"ciso646",
"climits",
"clocale",
"cmath",
"csetjmp",
"csignal",
"cstdarg",
"cstddef",
"cstdio",
"cstdlib",
"cstring",
"ctime",
"cwchar",
"cwctype",
"ccomplex",
"cfenv",
"cinttypes",
"cstdbool",
"cstdint",
"ctgmath",
"cuchar",
"algorithm",
"bitset",
"complex",
"deque",
"exception",
"fstream",
"functional",
"iomanip",
"ios",
"iosfwd",
"iostream",
"istream",
"iterator",
"limits",
"list",
"locale",
"map",
"memory",
"new",
"numeric",
"ostream",
"queue",
"set",
"sstream",
"stack",
"stdexcept",
"streambuf",
"string",
"typeinfo",
"utility",
"valarray",
"vector",
"array",
"atomic",
"chrono",
"codecvt",
"condition_variable",
"forward_list",
"future",
"initializer_list",
"mutex",
"random",
"ratio",
"regex",
"scoped_allocator",
"system_error",
"thread",
"tuple",
"typeindex",
"type_traits",
"unordered_map",
"unordered_set",
"shared_mutex",
"any",
"charconv",
"execution",
"filesystem",
"optional",
"memory_resource",
"string_view",
"variant",
"barrier",
"bit",
"compare",
"concepts",
"coroutine",
"latch",
"numbers",
"ranges",
"span",
"semaphore",
"source_location",
"version",
]

import os
import subprocess
import multiprocessing
import time

tmp = 'std'

if not os.path.exists(tmp):
    os.mkdir(tmp)

this = os.path.dirname(__file__)
basec = '/usr/include'
basecxx = '/usr/include/c++/v1'
procs = []
for h in headers:
    with open(os.path.join(this, h), 'w') as f:
        p = os.path.join(basec, h)
        if not os.path.exists(p):
            p = os.path.join(basecxx, h)
    if os.path.exists(os.path.join(tmp, h) + '.pcm'):
        continue
    print('Compiling:', h)
    procs.append(subprocess.Popen('clang++ -w -stdlib=libc++ -std=c++20 -fmodule-header=system -xc++-system-header {} -o {}.pcm'.format(p, os.path.join(tmp, h)), shell=True))
    while True:
        num = 0
        for i, p in enumerate(procs):
            r = p.poll() if p is not None else 0
            if r is not None:
                if r != 0:
                    raise SystemExit(r)
                procs[i] = None
            else:
                num += 1
        if num < multiprocessing.cpu_count():
            break
        time.sleep(0.1)

for p in procs:
    if p is not None:
        r = p.wait()
        if r != 0:
            raise SystemExit(r)

with open(os.path.join(tmp, 'std.cppm'), 'w') as f:
    f.write('export module std;\n')
    for h in headers:
        if not h.endswith('.h'):
            f.write('import <{}>;\n'.format(h))

with open(os.path.join(tmp, 'std.modmap'), 'w') as f:
    f.write('\n'.join('-fmodule-file={}.pcm'.format(os.path.join(tmp, x)) for x in headers))

print('Compiling:', 'std')
subprocess.check_call('clang++ -w -stdlib=libc++ @{} -std=c++20 -xc++-module -c {} -o {}.cppm.o -fmodule-output={}.pcm'.format(os.path.join(tmp, 'std.modmap'), os.path.join(tmp, 'std.cppm'), os.path.join(tmp, 'std'), os.path.join(tmp, 'std')), shell=True)

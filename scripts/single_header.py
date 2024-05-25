#!/usr/bin/env python

import re
import os

dir = 'co_async'
core = 'co_async/co_async.hpp'

sources = []
headers = []

# walk dir, all hpp and cpp files:
for root, dirs, files in os.walk(dir):
    for file in files:
        if file.endswith(('.h', '.hpp', '.c', '.cpp')):
            # get full path of file
            path = os.path.join(root, file)
            # do something with file
            if file.endswith(('.c', '.cpp')):
                sources.append(path)
            else:
                headers.append(path)

sorted_headers = []
visited_headers = set()

def visit(path):
    if path in visited_headers:
        return
    visited_headers.add(path)
    content = open(path, 'r').read()
    for header in headers:
        if header == path:
            continue
        if re.findall(r'#\s*include\s*<' + header + '>', content):
            visit(header)
    sorted_headers.append(path)

visit(core)

print('#pragma once')
for header in sorted_headers:
    content = open(header, 'r').read()
    content = content.replace('#pragma once', '')
    content = content.replace('\ufeff', '')
    for header in headers:
        content = re.sub(r'#\s*include\s*<' + header + '>', '', content)
    print(content)

print('#ifdef ' + dir.upper() + '_IMPLEMENTATION')
for source in sources:
    content = open(source, 'r').read()
    for header in headers:
        content = re.sub(r'#\s*include\s*<' + header + '>', '', content)
    content = content.replace('\ufeff', '')
    print(content)
print('#endif')

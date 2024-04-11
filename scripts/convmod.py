#!/usr/bin/env python

import re
import os

os.chdir(os.path.join(os.path.abspath(os.path.dirname(__file__)), '..'))

GLOBAL_MODULE_FRAGMENT = re.compile(r'^\s*module;$')
IMPORT_STD = re.compile(r'^\s*import\s+std;$')
EXPORT_MODULE = re.compile(r'^\s*export\s+module\s+([a-zA-Z0-9_.]+)(:[a-zA-Z0-9_.]+)?;$')
IMPORT = re.compile(r'^\s*(export\s+)?import\s+(:)?([a-zA-Z0-9_.]+);$')
EXPORT = re.compile(r'^(\s*)export(\s+.*)$')
COMMENT_ALL = re.compile(r'^.*/\*\{(.*)\}\*/.*$')
COMMENT_PART = re.compile(r'^(.*)/\*\[(.*)\]\*/(.*)$')

def process(dir):
    for dirpath, dirnames, filenames in os.walk(dir):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            if filepath.endswith('.hpp') or filepath.endswith('.cpp'):
                res = ''
                with open(filepath, 'r') as f:
                    current = None
                    parent = None
                    for line in f:
                        line = line.removesuffix('\n')
                        if 0: pass
                        elif m := re.match(COMMENT_ALL, line):
                            line = m.group(1)
                        elif m := re.match(COMMENT_PART, line):
                            l, m, r = m.groups()
                            line = l + m + r
                        elif m := re.match(GLOBAL_MODULE_FRAGMENT, line):
                            line = '/*{' + line + '}*/'
                        elif m := re.match(EXPORT_MODULE, line):
                            parent, partition = m.groups()
                            if partition:
                                current = parent + partition
                            else:
                                current = parent
                            line = '#pragma once/*{' + line + '}*/'
                        elif m := re.match(IMPORT_STD, line):
                            line = '#include <co_async/std.hpp>/*{' + line + '}*/'
                        elif m := re.match(IMPORT, line):
                            export, colon, partition = m.groups()
                            if colon:
                                assert parent
                                dependency = parent + '/' + partition
                            else:
                                dependency = partition + '/' + partition
                            dependency = dependency.replace('.', '/')
                            line = '#include <' + dependency + '.hpp>/*{' + line + '}*/'
                        elif m := re.match(EXPORT, line):
                            before, after = m.groups()
                            line = before + '/*[export]*/' + after
                        res += line + '\n'
                if 1:
                    with open(filepath, 'w') as f:
                        f.write(res)
                else:
                    print(res)

def process_cmake(path):
    res = ''
    with open(path, 'r') as f:
        for line in f:
            if line == '#set(CO_ASYNC_MODULE ON)\n':
                line = 'set(CO_ASYNC_MODULE ON)\n'
            elif line == 'set(CO_ASYNC_MODULE ON)\n':
                line = '#set(CO_ASYNC_MODULE ON)\n'
            res += line
    if 1:
        with open(path, 'w') as f:
            f.write(res)
    else:
        print(res)

process('co_async')
process('examples')
process_cmake('CMakeLists.txt')

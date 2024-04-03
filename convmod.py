#!/usr/bin/env python

import re
import os

dir = './co_async/'

GLOBAL_MODULE_FRAGMENT = re.compile(r'^\s*module;$')
IMPORT_STD = re.compile(r'^\s*import\s+std;$')
EXPORT_MODULE = re.compile(r'^\s*export\s+module\s+([a-zA-Z0-9_.]+)(:[a-zA-Z0-9_.]+)?;$')
IMPORT = re.compile(r'^\s*(export\s+)?import\s+(:)?([a-zA-Z0-9_.]+);$')
EXPORT = re.compile(r'^(\s*export\s+)')

# walk dir
for dirpath, dirnames, filenames in os.walk(dir):
    for filename in filenames:
        filepath = os.path.join(dirpath, filename)
        if filepath.endswith('.cppm') or filepath.endswith('.cpp'):
            res = ''
            with open(filepath, 'r') as f:
                current = None
                parent = None
                for line in f:
                    line = line.removesuffix('\n')
                    if 0: pass
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
                        line = '#include <bits/stdc++.h>/*{' + line + '}*/'
                    elif m := re.match(IMPORT, line):
                        export, colon, partition = m.groups()
                        if colon:
                            assert parent
                            dependency = parent + ':' + partition
                        else:
                            dependency = partition
                        dependency = dependency.replace(':', '/').replace('.', '/')
                        line = '#include <' + dependency + '.cppm>/*{' + line + '}*/'
                    elif m := re.match(EXPORT, line):
                        export = m.group(0)
                        line = '/*[' + export + ']*/' + line[len(export):]
                    res += line + '\n'
            if 1:
                with open(filepath, 'w') as f:
                    f.write(res)
            else:
                print(res)

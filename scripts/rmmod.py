#!/usr/bin/env python

import re
import os

os.chdir(os.path.join(os.path.abspath(os.path.dirname(__file__)), '..'))

COMMENT_ALL = re.compile(r'^(.*)/\*\{(.*)\}\*/.*$')
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
                            line = l + r
                            line = line.strip()
                        res += line + '\n'
                if 1:
                    with open(filepath, 'w') as f:
                        f.write(res)
                else:
                    print(res)

process('co_async')
process('examples')

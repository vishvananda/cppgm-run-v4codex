#!/usr/bin/env python3
"""Build isolated API and sanitizer tools using the registered frontend sources."""
import concurrent.futures
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
flags = ['-std=c++11', '-Wall', '-O1', '-g', '-fno-omit-frame-pointer']
if '--sanitize' in sys.argv:
    flags += ['-fsanitize=address,undefined', '-fno-pie', '-no-pie']
flags += ['-I'+str(root/'dev/src')]
sets = dict(re.findall(r'^(FRONTEND_OBJ_BASENAMES_\S+)\s*:=\s*(.*)$',
                       (root/'dev/frontend_source_sets.mk').read_text(), re.M))
text = sets['FRONTEND_OBJ_BASENAMES_cppgm++']
while '$(' in text:
    text = re.sub(r'\$\(([^)]+)\)', lambda m: sets[m[1]], text)
sources = [root/'dev/src'/f'{name}.cpp' for name in text.split()]

def compile_one(pair):
    index, source = pair
    obj = out/f'{index}.o'
    subprocess.run(['g++', *flags, '-c', source, '-o', obj], check=True)
    return obj

with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
    objects = list(executor.map(compile_one, enumerate(sources)))
for name, main in [('api', root/'student.tests/pa5/check_api.cpp'),
                   ('compiler', root/'dev/cppgm++.cpp')]:
    subprocess.run(['g++', *flags, main, *objects, '-o', out/name], check=True)
print('Built', out/'api', 'and', out/'compiler', flush=True)

#!/usr/bin/env python3
"""Build the semantic API and a standalone compiler outside tracked paths."""
import concurrent.futures
import pathlib
import re
import subprocess
import sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
flags = ['-std=c++11','-Wall','-O1','-g','-fno-omit-frame-pointer','-I'+str(root/'dev/src')]
if '--sanitize' in sys.argv:
    flags += ['-fsanitize=address,undefined','-fno-pie','-no-pie']
sets = dict(re.findall(r'^(FRONTEND_OBJ_BASENAMES_\S+)\s*:=\s*(.*)$', (root/'dev/frontend_source_sets.mk').read_text(), re.M))
names = sets['FRONTEND_OBJ_BASENAMES_cppgm++']
while '$(' in names:
    names = re.sub(r'\$\(([^)]+)\)',lambda m:sets[m[1]],names)
sources = [root/'dev/src'/f'{n}.cpp' for n in names.split()]
def compile_one(pair):
    i, src = pair; obj=out/f'{i}.o'
    subprocess.run(['g++',*flags,'-c',src,'-o',obj],check=True)
    return obj
with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
    objects=list(executor.map(compile_one,enumerate(sources)))
for name,main in [('api',root/'student.tests/pa6/check_api.cpp'),('compiler',root/'dev/cppgm++.cpp'),('pa5-api',root/'student.tests/pa5/check_api.cpp')]:
    subprocess.run(['g++',*flags,main,*objects,'-o',out/name],check=True)
print('Built semantic API, PA5 API and standalone compiler',out)

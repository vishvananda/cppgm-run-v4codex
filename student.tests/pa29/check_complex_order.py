#!/usr/bin/env python3
"""Complex arithmetic is correct and has identical emission across generations."""
import pathlib
import subprocess
import sys
import tempfile

compilers = [pathlib.Path(p).resolve() for p in sys.argv[1:]]
assert len(compilers) == 2, 'pass seed and self compiler paths'
with tempfile.TemporaryDirectory(prefix='pa29-complex-order-') as directory:
    out = pathlib.Path(directory)
    for level in ('-O0','-O3'):
        for i,compiler in enumerate(compilers):
            source = pathlib.Path('student.tests/pa29/complex-order.cpp')
            for mode,ext in (('--emit-lowir','lowir'),('-c','o')):
                subprocess.run([compiler,level,mode,source,'-o',out/f'{i}.{ext}'],check=True)
            subprocess.run(['g++',out/f'{i}.o','-o',out/f'{i}.exe'],check=True)
            for args in ([],['a'],['a','b']):
                subprocess.run([out/f'{i}.exe',*args],check=True)
        for ext in ('lowir','o'):
            assert (out/f'0.{ext}').read_bytes() == (out/f'1.{ext}').read_bytes(), (level,ext)
print('complex add/sub: O0/O3, three precisions, runtime and exact seed/self emission PASS')

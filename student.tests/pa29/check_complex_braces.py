#!/usr/bin/env python3
"""Explicit GNU complex list checks, including constants, templates and effects."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else 'dev/cppgm++').resolve()
with tempfile.TemporaryDirectory(prefix='pa29-complex-braces-') as tmp:
    out = pathlib.Path(tmp)
    for level in ('-O0','-O3'):
        subprocess.run([compiler,level,'-c','student.tests/pa29/complex-braces.cpp','-o',out/'source.o'],check=True)
        subprocess.run(['g++',out/'source.o','-o',out/'source'],check=True)
        for args in ([],['input'],['more','input']):
            subprocess.run([out/'source',*args],check=True)
    for code in ('__complex__ double z{1.,2.,3.};',
                 'void f(double x){__complex__ float z{x,1.f};}',
                 'void f(){__complex__ double z{1.,"bad"};}'):
        src = out/'bad.cpp'; src.write_text(code+'\n')
        r = subprocess.run([compiler,'-c',src,'-o',out/'bad.o'],capture_output=True)
        assert r.returncode == 1, r
print('complex lists: O0/O3 constants, members, templates, arguments, references, arrays, effects and rejection PASS')

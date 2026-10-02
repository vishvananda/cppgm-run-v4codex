#!/usr/bin/env python3
"""Explicit cv and nested class-conditional reducers; optional compiler path."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
with tempfile.TemporaryDirectory(prefix='pa12-conditional-') as tmp:
    out = pathlib.Path(tmp)
    for name in ('conditional-const', 'nested-conditional-lifetime'):
        source = pathlib.Path('student.tests/pa12')/(name+'.cpp')
        for level in ('-O0', '-O3'):
            subprocess.run([compiler, level, '-c', source, '-o', out/'source.o'], check=True)
            subprocess.run(['g++', out/'source.o', '-o', out/'source'], check=True)
            for args in ([], ['2'], ['3', '3'], ['4', '4', '4']):
                subprocess.run([out/'source', *args], check=True)
print('class conditional cv and nested lifetime: O0/O3, four runtime branches PASS')

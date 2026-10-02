#!/usr/bin/env python3
"""Run explicitly from the root; accepts an optional compiler path."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
with tempfile.TemporaryDirectory(prefix='pa11-static-actions-') as tmp:
    root = pathlib.Path(tmp)
    for level in ('-O0', '-O3'):
        obj, exe = root/'static.o', root/'static'
        subprocess.run([compiler, level, '-c', 'student.tests/pa11/local-static-actions.cpp', '-o', obj], check=True)
        subprocess.run(['g++', obj, '-o', exe], check=True)
        subprocess.run([exe], check=True)
print('local static constructor actions: O0/O3 member defaults, once-only effects and zero initialization PASS')

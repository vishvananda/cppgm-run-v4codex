#!/usr/bin/env python3
"""Explicit expression/type-id ambiguity reducer; optional compiler path."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
source = pathlib.Path('student.tests/pa5/functional-pointer.cpp')
with tempfile.TemporaryDirectory(prefix='pa5-functional-') as tmp:
    out = pathlib.Path(tmp)
    subprocess.run([compiler, '--emit-ast', source, '-o', out/'source.ast'], check=True)
    for level in ('-O0', '-O3'):
        subprocess.run([compiler, level, '-c', source, '-o', out/'source.o'], check=True)
        subprocess.run(['g++', out/'source.o', '-o', out/'source'], check=True)
        subprocess.run([out/'source'], check=True)
print('functional pointer operand and abstract pointer/reference casts: AST and O0/O3 runtime PASS')

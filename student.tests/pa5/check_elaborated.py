#!/usr/bin/env python3
"""Run from the root: python3 student.tests/pa5/check_elaborated.py [compiler]."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
cases = [
    pathlib.Path('student.tests/pa5/elaborated-call.cpp').read_text(),
    'struct item {}; int item(int,int); void f(int a) { struct item *p; item(a,a); }',
    'namespace ns { struct item {}; int item(int,int); } '
    'void f(int a) { struct ns::item *p; ns::item(a,a); }',
    'struct item {}; void f() { struct item; item *p; }',
    'void f() { struct fresh *p; fresh *q; }',
    'struct item {}; void f() { struct item { int n; }; item p; }',
]
with tempfile.TemporaryDirectory(prefix='pa5-elaborated-') as tmp:
    root = pathlib.Path(tmp)
    for index, source in enumerate(cases):
        src, ast = root/'case.cpp', root/'case.ast'
        src.write_text(source)
        subprocess.run([compiler, '--emit-ast', '-o', ast, src], check=True)
        output = ast.read_text()
        if index < 3:
            assert 'call-expression' in output, (index, output)
        else:
            assert 'decl-specifier TT_IDENTIFIER:' in output, (index, output)
    obj, exe = root/'case.o', root/'case'
    subprocess.run([compiler, '-O3', '-c', 'student.tests/pa5/elaborated-call.cpp', '-o', obj], check=True)
    subprocess.run(['g++', obj, '-o', exe], check=True)
    subprocess.run([exe], check=True)
print('elaborated class lookup: 6 parser cases and runtime PASS')

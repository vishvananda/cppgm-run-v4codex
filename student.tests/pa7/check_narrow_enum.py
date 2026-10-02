#!/usr/bin/env python3
"""Run explicitly from the root; accepts an optional compiler path."""
import pathlib
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
with tempfile.TemporaryDirectory(prefix='pa7-enum-') as tmp:
    root = pathlib.Path(tmp)
    for level in ('-O0', '-O3'):
        obj, exe = root/'switch.o', root/'switch'
        subprocess.run([compiler, level, '-c', 'student.tests/pa7/narrow-enum-switch.cpp', '-o', obj], check=True)
        subprocess.run(['g++', obj, '-o', exe], check=True)
        subprocess.run([exe], check=True)
    for body in ('if (e) return 1;', 'switch (e) { case 1: return 1; }', 'return +e;'):
        src = root/'bad.cpp'
        src.write_text('enum class E : unsigned char { a = 1 }; int f(E e) {' + body + ' return 0; }')
        result = subprocess.run([compiler, '-c', src, '-o', obj], capture_output=True)
        assert result.returncode == 1, (body, result)
print('narrow scoped enum switches: O0/O3 signed/unsigned runtime and 3 rejection cases PASS')

#!/usr/bin/env python3
"""Hosted trap: python3 student.tests/pa29/check_trap.py [compiler]."""
import pathlib
import resource
import subprocess
import sys
import tempfile

compiler = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve()
resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
with tempfile.TemporaryDirectory(prefix='pa29-trap-') as tmp:
    root = pathlib.Path(tmp)
    for level in ('-O0', '-O3'):
        obj, exe = root/'trap.o', root/'trap'
        subprocess.run([compiler, level, '-c', 'student.tests/pa29/trap.cpp', '-o', obj], check=True)
        subprocess.run(['g++', obj, '-o', exe], check=True)
        subprocess.run([exe], check=True)
        for args in (['nested'], ['qualified', 'call']):
            result = subprocess.run([exe, *args])
            assert result.returncode < 0, (level, args, result.returncode)
    for source in ('int main() { __builtin_trap(1); }',
                   'template<class T> void f() { ::__builtin_trap(1); } int main() { f<int>(); }'):
        src = root/'bad.cpp'
        src.write_text(source)
        result = subprocess.run([compiler, '-c', src, '-o', obj], capture_output=True)
        assert result.returncode == 1, result
print('trap builtin: O0/O3 ordinary and template termination, probe, noexcept, arity PASS')

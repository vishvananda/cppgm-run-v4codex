#!/usr/bin/env python3
"""Explicit typed LowIR/native helper schedule and runtime checks."""
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
compiler = root/'dev/cppgm++'
objects = subprocess.check_output([
    'make', '-s', '-C', str(root/'dev'), '--eval',
    'print-objects: ; @echo $(call frontend_objs,cppgm++) $(call frontend_objs,lowir)',
    'print-objects'], text=True).split()
objects = list(dict.fromkeys(str((root/'dev'/p).resolve()) for p in objects))
with tempfile.TemporaryDirectory(prefix='pa29-float-order-') as tmp:
    out = pathlib.Path(tmp)
    harness, obj, exe = out/'check', out/'float.o', out/'float'
    subprocess.run(['g++', '-std=c++11', '-I'+str(root/'dev/src'),
                    root/'student.tests/pa29/float-helper-order.cpp', *objects, '-o', harness], check=True)
    source = root/'student.tests/pa29/ordered-float.cpp'
    for level in ('-O0', '-O3'):
        lowir = out/'float.lowir'
        subprocess.run([compiler, level, '--emit-lowir', '-c', source, '-o', lowir], check=True)
        for command in ([harness, lowir, obj], [compiler, level, '-c', source, '-o', obj]):
            subprocess.run(command, check=True)
            subprocess.run(['g++', obj, '-o', exe], check=True)
            for args in ([], ['input'], ['another', 'input']):
                subprocess.run([exe, *args], check=True)
print('floating helper schedules: complete stable order, validation, direct/replay O0/O3 runtime PASS')

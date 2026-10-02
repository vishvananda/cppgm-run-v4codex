#!/usr/bin/env python3
"""Explicit whole-stage hosted trace; retain the implementation controls too."""
import pathlib
import runpy
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
sys.argv = [str(root/'student.tests/pa31/trace205.py'), str(out)]
trace = runpy.run_path(sys.argv[0], run_name='__main__')
run, sha = trace['run'], trace['sha']
sections, unwind = trace['sections'], trace['unwind']
r = trace['r']
r['passed'] = False
trace['save']()
cc = root/'dev/cppgm++'
for src in sorted((root/'student.tests/pa31/source206').glob('*.cpp')):
    name = src.stem
    obj, ir, rebuilt, mir = [out/(name+suffix) for suffix in
                             ['.o', '.lowir', '.rebuilt.o', '.mir']]
    run([cc, '-O0', '-c', src, '-o', obj])
    plain = sha(obj)
    run([cc, '-O0', '-c', '--stats', src, '-o', obj])
    assert sha(obj) == plain
    run([cc, '-c', '--emit-lowir', '--validate-lowir', src, '-o', ir])
    run([trace['adapter'], ir, rebuilt, mir])
    a, b = sections(obj), sections(rebuilt)
    keys = {k for k in a if k.startswith('.text')}
    assert keys == {k for k in b if k.startswith('.text')}
    assert all(a[k] == b[k] for k in keys)
    da, db = [run(['objdump', '-dr', p]) for p in [obj, rebuilt]]
    assert trace['disassembly'](da) == trace['disassembly'](db)
    assert unwind(obj) == unwind(rebuilt)
    nm = run(['nm', '-C', obj])
    assert 'dormant' not in nm
    for kind, image in [('direct', obj), ('rebuilt', rebuilt)]:
        exe = out/(name+'-'+kind)
        run(['g++', image, '-o', exe])
        for args in ([['3'], ['6']] if name == 'interactions' else [[]]):
            run([exe, *args])
    r['images'][name] = dict(source_sha256=sha(src), direct_object_sha256=plain,
        rebuilt_object_sha256=sha(rebuilt), lowir_sha256=sha(ir), mir_sha256=sha(mir),
        text_identical=True, function_cfi_identical=True,
        named_relocations_identical=True, telemetry_identical=True)
    trace['save']()
assert sha(cc) == r['compiler_sha256']
r['passed'] = True
trace['save']()
print(len(r['commands']), 'whole-stage trace commands passed')

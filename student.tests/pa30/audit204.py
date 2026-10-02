#!/usr/bin/env python3
"""Explicit PA30 final-audit controls, image reuse and end-to-end inspection.

Usage: audit204.py SCRATCH
Generated objects/views remain outside the checkout; JSON evidence is retained.
"""
import hashlib
import json
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
evidence = root / 'student.tests/pa30/evidence204'
cc = root / 'dev/cppgm++'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


record = dict(binaries={n: sha(root / 'dev' / n)
                       for n in ['cppgm++', 'lowir', 'lowir2native']},
              commands=[], images={}, views={}, controls={})


def save():
    (evidence / 'inspection.json').write_text(json.dumps(record, indent=2) + '\n')


def run(args, reject=False):
    args = list(map(str, args))
    p = subprocess.run(args, capture_output=True, text=True, timeout=180)
    record['commands'].append(dict(args=args, status=p.returncode,
                                   stdout=p.stdout, stderr=p.stderr,
                                   expected_rejection=reject))
    save()
    assert (p.returncode != 0) == reject, record['commands'][-1]
    return p


for src in sorted((root / 'student.tests/pa30/source204').glob('*.lowir')):
    reject = '.reject.' in src.name
    run([root / 'dev/lowir', src, '-o', out / 'roundtrip.lowir'], reject)
    run([root / 'dev/lowir2native', src, '-o', out / 'ir-program'], reject)
    if not reject:
        run([out / 'ir-program'])

# Compile the explicit external-input adapter against exactly the final objects.
objects = [p for p in sorted((root / 'obj/dev').rglob('*.o'))
           if 'entry' not in p.parts and not p.name.startswith('test_runner')]
adapter = out / 'adapter'
run(['g++', '-std=c++11', '-O2', '-I' + str(root / 'dev/src'),
     root / 'student.tests/pa30/ir-object204.cpp', *objects, '-o', adapter])
record['adapter_sha256'] = sha(adapter)
for name, src in [('interactions', root / 'student.tests/pa30/source204/interactions.cpp'),
                  ('optimization', root / 'student.tests/pa30/source198/optimization.cpp')]:
    obj, ir, mir = (out / (name + ext) for ext in ['.o', '.lowir', '.mir'])
    run([cc, '-O0', '-c', src, '-o', obj])
    direct = sha(obj)
    stats = run([cc, '-O0', '-c', '--stats', src, '-o', obj])
    assert sha(obj) == direct
    run(['g++', obj, '-o', out / name])
    run([out / name])
    if name == 'interactions':
        run([out / name, 'vary'])
    run([cc, '-c', '--emit-lowir', '--validate-lowir', src, '-o', ir])
    canonical = out / (name + '.canonical')
    run([root / 'dev/lowir', ir, '-o', canonical])
    run([root / 'dev/lowir', canonical, '-o', out / 'twice.lowir'])
    assert canonical.read_bytes() == (out / 'twice.lowir').read_bytes()
    rebuilt = out / (name + '.rebuilt.o')
    run([adapter, canonical, rebuilt, mir])
    run(['g++', rebuilt, '-o', out / (name + '.rebuilt')])
    run([out / (name + '.rebuilt')])
    # Presentation aliases can reorder ELF symbols. Compare text, symbol names
    # and relocations via retained views; don't demand incidental ELF ordering.
    view = dict(lowir=ir.read_text(), mir=mir.read_text())
    for label, path in [('direct', obj), ('rebuilt', rebuilt)]:
        view[label] = {kind: run(cmd + [path]).stdout for kind, cmd in [
            ('disassembly', ['objdump', '-dr']), ('symbols', ['readelf', '-Ws']),
            ('frames', ['readelf', '--debug-dump=frames'])]}
    record['views'][name] = view
    record['images'][name] = dict(source_sha256=sha(src), direct_object_sha256=direct,
                                 rebuilt_object_sha256=sha(rebuilt), telemetry_identical=True,
                                 counters=[json.loads(s) for s in stats.stderr.splitlines()
                                           if s.startswith('{')])
    save()

# Rerun accumulated positive/negative controls against the final binary.
for n in [198, 199, 200, 201, 202, 203]:
    directory = out / ('controls' + str(n))
    run([sys.executable, root / ('student.tests/pa30/check%d.py' % n), directory])
    data = json.loads((directory / 'controls.json').read_text())
    assert data['compiler_sha256'] == record['binaries']['cppgm++']
    record['controls'][str(n)] = data
    save()

# The validator repair cannot change production code generation. Prove reuse
# of the entry differential executions with freshly rebuilt identical objects.
old = json.loads((root / 'student.tests/pa30/evidence203/differential.json').read_text())
images = []
for row in old['cases']:
    src = root / row['source']
    assert sha(src) == row['sha256'] and row['equal']
    run([cc, '-O0', '-c', src, '-o', out / 'differential.o'])
    historical = root / 'student.tests/pa30/evidence204/entry-differential.json'
    # audit entry reran the same frozen source and executable pair.
    entry = json.loads(historical.read_text())
    previous = next(r for r in entry['cases'] if r['source'] == row['source'])
    command = next(c['args'] for c in previous['commands']
                   if c['label'] == 'student' and '-c' in c['args'])
    object_path = pathlib.Path(command[command.index('-o') + 1])
    assert previous['equal'] and sha(out / 'differential.o') == sha(object_path)
    images.append(dict(source=row['source'], source_sha256=sha(src),
                       object_sha256=sha(object_path)))
record['differential_images'] = images
assert all(sha(root / 'dev' / n) == h for n, h in record['binaries'].items())
save()
print(len(record['commands']), 'audit commands passed;', len(images), 'differential images identical')

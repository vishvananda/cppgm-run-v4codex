#!/usr/bin/env python3
"""Run definition/demand controls explicitly; keep generated files in OUT."""
import hashlib
import json
import pathlib
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
compiler = pathlib.Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else root / 'dev/cppgm++'
rows = []


def run(args, ok=True):
    args = list(map(str, args))
    p = subprocess.run(args, capture_output=True, text=True, timeout=60)
    rows.append(dict(command=args, status=p.returncode, expected_success=ok,
                     stdout=p.stdout, stderr=p.stderr))
    (out / 'checks.json').write_text(json.dumps(rows, indent=2) + '\n')
    assert (p.returncode == 0) == ok, (args, p.returncode, p.stdout, p.stderr)
    return p


images = {}
for src in sorted((root / 'student.tests/pa29/controls189').glob('*.cpp')):
    ok = not src.stem.endswith('-reject')
    for cc in [compiler, 'g++', 'clang++']:
        obj = out / (src.stem + '.o')
        exe = out / src.stem
        run([cc, '-std=c++11', '-O0', '-c', src, '-o', obj], ok)
        if ok:
            run(['g++', obj, '-o', exe])
            run([exe, 'runtime-input'])
            if cc == compiler:
                syms = run(['nm', '-C', obj]).stdout
                assert 'dormant' not in syms
                images[src.stem] = dict(object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest(),
                                       symbols=syms)
    if ok:
        lir = out / (src.stem + '.lowir')
        run([compiler, '--emit-lowir', src, '-o', lir])
        run([root / 'dev/lowir', lir, '-o', out / (src.stem + '.canonical')])
        assert 'dormant' not in lir.read_text()

# The actual fixture names and ordinary-name reducers obey the same source
# declarations. No source is changed in the contract directory.
fixtures = [
    '600-hosted-nothrow-default-constructible-shorthand',
    '700-hosted-char-traits-primary-conversion-shims',
    '700-hosted-nothrow-invocable-cache-default',
]
observations = []
for stem in fixtures:
    src = root / 'pa29/tests/compile' / (stem + '.t')
    for cc in [compiler, 'g++', 'clang++']:
        # The driver accepts .t directly; host compilers need an explicit language.
        args = [cc, '-std=c++11'] + ([] if cc == compiler else ['-x', 'c++'])
        p = run([*args, '-c', src, '-o', out / 'reject.o'], False)
        observations.append(dict(source=str(src.relative_to(root)), compiler=str(cc),
                                 status=p.returncode, stderr=p.stderr))

# Supply the absent semantic definitions through an explicit personal header.
# The original fixture bytes and all its assertions remain intact.
for stem, header in zip(fixtures, ['trait-definitions.h', 'character-definitions.h',
                                   'invocation-definition.h']):
    src = root / 'pa29/tests/compile' / (stem + '.t')
    for cc in [compiler, 'g++', 'clang++']:
        args = [cc, '-std=c++11'] + ([] if cc == compiler else ['-x', 'c++'])
        run([*args, '-include', root / 'student.tests/pa29/controls189' / header,
             '-c', src, '-o', out / 'defined-original.o'])
        run(['g++', out / 'defined-original.o', '-o', out / 'defined-original'])
        run([out / 'defined-original'])

# Preserve positive coverage of the library spellings without inventing their
# definitions. These copies are personal tests, never replacement fixtures.
variants = [
    ('defined-traits', lambda s: s.replace('ordinary', 'std')),
    ('defined-conversions', lambda s: s.replace('traits', 'char_traits')),
    ('source-trait-identity', lambda s: s.replace('property', '__is_nothrow_invocable')
                                      .replace('invert', '__not_').replace('cache', '__cache_default')),
]
for name, transform in variants:
    src = out / (name + '-spelling.cpp')
    src.write_text(transform((root / 'student.tests/pa29/controls189' / (name + '.cpp')).read_text()))
    for cc in [compiler, 'g++', 'clang++']:
        run([cc, '-std=c++11', '-O0', '-c', src, '-o', out / 'spelling.o'])
        run(['g++', out / 'spelling.o', '-o', out / 'spelling'])
        run([out / 'spelling', 'runtime-input'])

(out / 'images.json').write_text(json.dumps(images, indent=2) + '\n')
(out / 'fixture-observations.json').write_text(json.dumps(observations, indent=2) + '\n')
print(f'{len(rows)} control commands passed')

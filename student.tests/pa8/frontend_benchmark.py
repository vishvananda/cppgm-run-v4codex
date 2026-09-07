#!/usr/bin/env python3
"""Recheck the unchanged PA7 template-demand corpus at the PA8 audit boundary.
Usage: frontend_benchmark.py measure|verify FROZEN_COMPILER observations.json
Both variants use the same binary; this measures absolute cost and scaling.
The PA7 generator, protocol, work assertions and budgets are reused unchanged.
"""
from pathlib import Path
import hashlib, importlib.util, json, subprocess, sys

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('pa7_benchmark', ROOT/'student.tests/pa7/benchmark.py')
prior = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prior)
binary, output = Path(sys.argv[2]).resolve(), Path(sys.argv[3])
options = dict(prefix='semantics-template-demand', repeat_factor=2)
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

if sys.argv[1] == 'measure':
    prior.measure([binary, binary], output, **options)
    data = json.loads(output.read_text())
    commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip()
    # Both variants really are the current frontend, not the PA6 revision used
    # by the inherited harness's default cross-stage comparison.
    data['source_commits'] = [commit, commit]
    paths = [ROOT/'dev/cppgm++.cpp', ROOT/'dev/frontend_source_sets.mk', Path(__file__),
             ROOT/'student.tests/pa7/benchmark.py', ROOT/'student.tests/pa6/measure.py',
             ROOT/'student.tests/pa6/bench_inputs.py']
    for group in ('preprocess','posttoken','syntax','semantic','support'):
        paths.extend(p for p in (ROOT/'dev/src'/group).rglob('*') if p.suffix in ('.h','.cpp'))
    data['source_hashes'] = {str(p.relative_to(ROOT)):sha(p) for p in paths}
    output.write_text(json.dumps(data, indent=2)+'\n')
else:
    assert sys.argv[1] == 'verify'
    data = json.loads(output.read_text())

for p, h in data['source_hashes'].items():
    assert sha(ROOT/p) == h, ('source changed', p)
assert data['binaries'][0]['sha256'] == data['binaries'][1]['sha256']
prior.verify([binary, binary], data, **options)
prior.prior.report(data)

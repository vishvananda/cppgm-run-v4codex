#!/usr/bin/env python3
"""Reuse the fixed compiler/common benchmarks with frozen seed/self binaries.

Run without concurrent builds or tests. Each inherited harness keeps A/A and
six ABBA blocks, compiler latency/RSS, checked runtime, text sizes and hashes.
"""
import json
import os
import pathlib
import subprocess
import sys

out = pathlib.Path(sys.argv[1]).resolve()
compilers = [str(pathlib.Path(p).resolve()) for p in sys.argv[2:4]]
assert len(compilers) == 2
out.mkdir(parents=True, exist_ok=True)
rows = []
for lane, script, level in [('selfhost', 'selfhost_performance.py', None),
                            ('common-o0', 'common_levels.py', '0'),
                            ('common-o2', 'common_levels.py', '2')]:
    args = ['python3', 'student.tests/pa32/'+script, str(out/lane), *compilers]
    env = dict(os.environ, PERF_CPU=os.environ.get('PERF_CPU', '2'))
    if level is not None:
        env.update(PA32_BASE_LEVEL=level, PA32_FINAL_LEVEL=level)
    with (out/(lane+'.log')).open('w') as log:
        result = subprocess.run(args, env=env, stdout=log, stderr=subprocess.STDOUT)
    rows.append(dict(command=args, status=result.returncode,
                     environment={k:env[k] for k in ('PERF_CPU','PA32_BASE_LEVEL','PA32_FINAL_LEVEL') if k in env}))
    (out/'commands.json').write_text(json.dumps(rows, indent=2)+'\n')
    assert result.returncode == 0, lane
    data = json.loads((out/lane/'performance.json').read_text())
    images = [data['images']] if level is None else data['images'].values()
    key = 'sha256' if level is None else 'object_sha256'
    assert all(image['A'][key] == image['B'][key] for image in images), (lane, 'object divergence')
    # Compare actual semantic/IR/selection work separately from elapsed time.
    # No added timing ratio gate: PA34 retains its documented command limits.
    seen = {}
    for row in data['runs']:
        if row.get('mode') != 'compile':
            continue
        counters = [{k:v for k,v in phase.items() if not k.endswith('_ms') and not k.endswith('_rss_kib')}
                    for phase in row['phase_counters']]
        workload = row['workload']
        assert seen.setdefault(workload, counters) == counters, (lane, workload, 'work divergence')
    print(lane, 'PASS: identical outputs' + (' and work' if level is not None else ''), flush=True)

#!/usr/bin/env python3
"""Verify every retained audit performance observation and compare typed work."""
import hashlib, json
from pathlib import Path

root = Path(__file__).resolve().parent / 'evidence174'
delta, scaling = {}, {}
total = 0
for series in ['cumulative', 'audit', 'affected', 'repeat']:
    data = json.loads((root / (series+'-performance.json')).read_text())
    assert all(row['status'] == 0 for row in data['runs'])
    assert len(data['runs']) == (544 if series == 'affected' else 56 if series == 'repeat' else 224)
    total += len(data['runs'])
    for name in data['summary']:
        counts, samples = {}, {}
        for label in 'AB':
            rows = [row for row in data['runs'] if row['workload'] == name
                    and row['mode'] == 'compile' and row['label'] == label]
            if not rows:
                continue
            facts = [{k: v for phase in row['phase_counters']
                      for k, v in phase.items()
                      if not k.endswith('_ms') and k != 'peak_rss_kib'}
                     for row in rows]
            assert facts[0] and all(fact == facts[0] for fact in facts), (series, name, label)
            counts[label], samples[label] = facts[0], len(rows)
        if 'A' in counts:
            a, b = counts['A'], counts['B']
            delta[series+'/'+name] = dict(compile_samples=samples,
                equal_existing_counters=sum(a[k] == b[k] for k in a),
                differences={k: [a.get(k), b.get(k)] for k in sorted(a.keys() | b.keys())
                             if a.get(k) != b.get(k)},
                images_equal=data['images'][name]['A'] == data['images'][name]['B'])
        else:
            assert samples == {'B': 8}
            check = data['entry_checks'][name]
            assert check['compile_status'] or check['runtime_status']
            discard = name.startswith('discard')
            n = int(name.removeprefix('discard').removeprefix('capture'))
            expected = ({'semantic_fold_steps': 2*n-1, 'semantic_type_query_work': 5*n+11,
                         'full_expression_work': 8*n+92, 'full_expression_regions': n+2,
                         'semantic_specializations': 4, 'semantic_body_checks': 5}
                        if discard else {'semantic_closures': n, 'semantic_capture_patterns': 1,
                         'semantic_capture_candidates': 0, 'semantic_capture_edges': 0,
                         'semantic_specializations': 2*n, 'template_body_transitions': 2*n,
                         'semantic_body_checks': 2*n+4, 'full_expression_work': 18*n+43,
                         'semantic_type_query_work': n+3})
            assert all(counts['B'][k] == v for k, v in expected.items()), name
            assert data['images'][name]['B']['text_bytes'] == (72*n+610 if discard else 143*n+235)
            scaling[name] = dict(compile_samples=8, counters=counts['B'],
                                 asserted=expected, image=data['images'][name]['B'])
    for binary in data['binaries'].values():
        p = Path(binary['path'])
        if p.exists():
            assert hashlib.sha256(p.read_bytes()).hexdigest() == binary['sha256']

for name, value in [('counter-delta.json', delta), ('scaling-counters.json', scaling)]:
    (root/name).write_text(json.dumps(value, indent=2)+'\n')
print(total, 'final observations checked; all measured work counters are deterministic')

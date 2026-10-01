#!/usr/bin/env python3
"""Check work counts in the retained PA29 implementation173 measurements."""
import json
from pathlib import Path

root = Path(__file__).resolve().parent / 'evidence173'
common = json.loads((root / 'common-performance.json').read_text())
closure = json.loads((root / 'closure-performance.json').read_text())
delta = {}
scaling = {}
for data in [common, closure]:
    assert all(row['status'] == 0 for row in data['runs'])
    for name in data['summary']:
        counters = {}
        samples = {}
        for label in 'AB':
            rows = [row for row in data['runs'] if row['workload'] == name
                    and row['mode'] == 'compile' and row['label'] == label]
            if not rows:
                continue
            counts = [{k: v for phase in row['phase_counters']
                       for k, v in phase.items()
                       if not k.endswith('_ms') and k != 'peak_rss_kib'}
                      for row in rows]
            assert all(count == counts[0] for count in counts), name
            counters[label] = counts[0]
            samples[label] = len(rows)
        if 'A' in counters:
            a, b = counters['A'], counters['B']
            delta[name] = {
                'compile_samples': samples,
                'equal_existing_counters': sum(a[k] == b[k] for k in a),
                'differences': {k: [a.get(k), b.get(k)]
                                for k in sorted(a.keys() | b.keys())
                                if a.get(k) != b.get(k)}}
        else:
            n = int(name.removeprefix('closures').removeprefix('calls'))
            is_closure = name.startswith('closures')
            expected = {
                'semantic_closures': n if is_closure else 2,
                'semantic_capture_edges': n if is_closure else 2,
                'semantic_capture_patterns': 1 if is_closure else 2,
                'semantic_capture_candidates': 1 if is_closure else 2,
                'semantic_specializations': 3*n if is_closure else n+1,
                'template_body_transitions': 3*n if is_closure else n+1,
                'semantic_body_checks': 3*n+2 if is_closure else n+4}
            assert all(counters['B'][k] == v for k, v in expected.items()), name
            text = 212*n-245 if is_closure else 67*n+202
            assert data['images'][name]['B']['text_bytes'] == text, name
            scaling[name] = {'compile_samples': samples['B'],
                             'counters': expected, 'text_bytes': text}

for name, image in common['images'].items():
    assert image['A'] == image['B'], name
assert len(common['runs']) == 224 and len(closure['runs']) == 152
assert all(row['status'] != 0 for row in closure['entry_checks'].values())
for name, value in [('counter-delta.json', delta), ('scaling-counters.json', scaling)]:
    (root / name).write_text(json.dumps(value, indent=2) + '\n')
print('376 final observations checked; deterministic work counts and text formulas pass')

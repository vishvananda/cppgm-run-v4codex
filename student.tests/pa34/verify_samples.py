#!/usr/bin/env python3
"""Independently recompute retained A/A and ABBA records, without timing gates.

Usage: verify_samples.py EVIDENCE_DIRECTORY OUTPUT_JSON
Reads plain JSON or deterministic gzip copies. Does not import timing harnesses.
"""
import gzip
import hashlib
import json
import math
import pathlib
import statistics
import sys

directory = pathlib.Path(sys.argv[1])


def load(name):
    path = directory / (name + '.json')
    if path.exists():
        return json.loads(path.read_text())
    return json.loads(gzip.decompress(path.with_suffix('.json.gz').read_bytes()))


def sha(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def same(actual, expected):
    if isinstance(expected, dict):
        assert actual.keys() == expected.keys(), (actual.keys(), expected.keys())
        for key in expected:
            same(actual[key], expected[key])
    elif isinstance(expected, list):
        assert len(actual) == len(expected)
        for a, b in zip(actual, expected):
            same(a, b)
    elif isinstance(expected, float):
        assert math.isclose(actual, expected, rel_tol=1e-12), (actual, expected)
    else:
        assert actual == expected, (actual, expected)


def summarize(rows):
    assert len(rows) == 28
    for block in range(7):
        selected = [r for r in rows if r['block'] == block]
        assert ''.join(r['label'] for r in selected) == ('AAAA' if block == 0 else 'ABBA')
    assert all(r['status'] == 0 and r['wall_s'] > 0 and r['peak_rss_kib'] > 0 for r in rows)
    aa = [r['wall_s'] for r in rows if r['block'] == 0]
    pairs = []
    for block in range(1, 7):
        times = {k: [r['wall_s'] for r in rows if r['block'] == block and r['label'] == k] for k in 'AB'}
        pairs.append(sum(times['B']) / sum(times['A']))
    result = dict(AA_range_s=[min(aa), max(aa)], paired_ratios=pairs,
                  paired_ratio_median=statistics.median(pairs), paired_ratio_range=[min(pairs), max(pairs)])
    for label in 'AB':
        samples = [r for r in rows if r['block'] and r['label'] == label]
        times = [r['wall_s'] for r in samples]
        result[label] = dict(median_s=statistics.median(times), range_s=[min(times), max(times)],
                             peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
    return result


report = dict(datasets={}, observations=0, timed_object_hashes=0)
for name in ('selfhost', 'common-o0', 'common-o2', 'stack-baseline', 'stack-dispatch'):
    if not any((directory / (name + ext)).exists() for ext in ('.json', '.json.gz')):
        continue
    data = load(name)
    for binary in data['binaries'].values():
        assert sha(binary['path']) == binary['sha256'], binary
    groups = {}
    for row in data['runs']:
        key = (row.get('workload', 'compiler-source'), row.get('mode', 'compile'))
        groups.setdefault(key, []).append(row)
    summaries = {}
    for (workload, mode), rows in groups.items():
        summary = summarize(rows)
        common = name.startswith('common')
        recorded = data['summary'][workload][mode] if common else data['summary']
        comparable = dict(summary)
        if name.startswith('stack'):
            comparable['aa_range_s'] = comparable.pop('AA_range_s')
            comparable['median_ratio'] = comparable.pop('paired_ratio_median')
            del comparable['paired_ratio_range']
        same(comparable, recorded)
        images = data['images'][workload] if common else data['images']
        assert images['A'] == images['B'], (name, workload, 'output divergence')
        counters = None
        for row in rows:
            if 'object_sha256' in row:
                key = 'object_sha256' if common else 'sha256'
                assert row['object_sha256'] == images[row['label']][key]
                report['timed_object_hashes'] += 1
            if mode == 'compile' and 'phase_counters' in row:
                work = [{k: v for k, v in phase.items() if not k.endswith(('_ms', '_rss_kib'))}
                        for phase in row['phase_counters']]
                assert work, (name, workload, 'missing telemetry')
                if counters is None:
                    counters = work
                assert work == counters, (name, workload, 'work divergence')
        summaries[workload + '/' + mode] = summary
    if name.startswith('common'):
        assert data['flags']['A'] == data['flags']['B']
    report['datasets'][name] = summaries
    report['observations'] += len(data['runs'])
pathlib.Path(sys.argv[2]).write_text(json.dumps(report, indent=2) + '\n')
print('Verified', report['observations'], 'observations and', report['timed_object_hashes'], 'timed output hashes')

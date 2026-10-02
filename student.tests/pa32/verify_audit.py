#!/usr/bin/env python3
"""Verify checkpoint-210 bindings, raw measurements, scope and progress.

Historical sources are checked at their recorded handoffs, including symlink
targets. Numerical performance diagnostics are reported, not extra course gates.
"""
import hashlib
import json
import os
import pathlib
import re
import statistics
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
EVIDENCE = pathlib.Path(__file__).resolve().parent / 'evidence210'


def sha(content):
    return hashlib.sha256(content).hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def historical_sources(stage, commit):
    folder = EVIDENCE.parent / ('evidence' + stage)
    binding = json.loads((folder / 'binding.json').read_text())
    modes = {}
    for row in git('ls-tree', '-r', commit).decode().splitlines():
        header, path = row.split('\t', 1)
        modes[path] = header.split()[0]
    sources = binding.get('source_hashes', binding.get('source_sha256'))
    for path, expected in sources.items():
        resolved = path
        while modes[resolved] == '120000':
            target = git('show', commit + ':' + resolved).decode()
            resolved = os.path.normpath(os.path.join(os.path.dirname(resolved), target))
        assert sha(git('show', commit + ':' + resolved)) == expected, (stage, path)
    return len(sources)


def measurement(path):
    data = json.loads(path.read_text())
    if not data.get('runs'):
        return 0
    binaries = data.get('binaries', {'compiler': data.get('binary')})
    if isinstance(binaries, dict):
        for binary in binaries.values():
            if isinstance(binary, dict) and 'path' in binary:
                assert sha(pathlib.Path(binary['path']).read_bytes()) == binary['sha256'], binary
    groups = {}
    for row in data['runs']:
        assert row.get('status', 0) == 0, path
        groups.setdefault((row.get('workload'), row.get('mode')), []).append(row)
    for (workload, mode), rows in groups.items():
        assert len(rows) == 28, (path, workload, mode)
        for block in range(7):
            assert ''.join(r['label'] for r in rows if r['block'] == block) == ('AAAA' if block == 0 else 'ABBA')
        ratios = [statistics.mean(r['wall_s'] for r in rows if r['block'] == block and r['label'] == 'B') /
                  statistics.mean(r['wall_s'] for r in rows if r['block'] == block and r['label'] == 'A')
                  for block in range(1, 7)]
        summary = data['summary']
        if workload:
            summary = summary[workload]
        if mode:
            summary = summary[mode]
        assert ratios == summary['paired_ratios'], path
        assert statistics.median(ratios) == summary['paired_ratio_median'], path
        assert [min(ratios), max(ratios)] == summary['paired_ratio_range'], path
        if 'AA_range_s' in summary:
            aa = [r['wall_s'] for r in rows if not r['block']]
            assert summary['AA_range_s'] == [min(aa), max(aa)], path
        for label in 'AB':
            samples = [r for r in rows if r['block'] and r['label'] == label]
            if label in summary:
                assert summary[label]['median_s'] == statistics.median(r['wall_s'] for r in samples)
                if 'peak_rss_kib' in samples[0]:
                    assert summary[label]['peak_rss_kib'] == max(r['peak_rss_kib'] for r in samples)
    return len(data['runs'])


binding = json.loads((EVIDENCE / 'binding.json').read_text())
for path, expected in binding['source_sha256'].items():
    assert sha((ROOT / path).read_bytes()) == expected, path
for path, expected in binding['evidence_sha256'].items():
    assert sha((EVIDENCE / path).read_bytes()) == expected, path
for path, expected in binding['artifact_sha256'].items():
    assert sha(pathlib.Path(path).read_bytes()) == expected, path
assert not git('diff', binding['review_base'], '--', *binding['contract_paths'])
assert git('rev-list', '--reverse', binding['review_base'] + '..' + binding['entry_commit']).decode().splitlines() == binding['entry_commits']

history = []
for stage, commit in [('207', '399faac3'), ('208', 'd2e412d6'), ('209', 'e409c2d8')]:
    sources = historical_sources(stage, commit)
    observations = sum(measurement(p) for p in (EVIDENCE.parent / ('evidence' + stage)).glob('*.json'))
    history.append(dict(stage=stage, source_bindings=sources, observations=observations))
current = sum(measurement(EVIDENCE / name) for name in binding['measurements'])

art = pathlib.Path(binding['artifact_dir'])
assert sha((ROOT / 'dev/cppgm++').read_bytes()) == sha((art / 'reviewed-cppgm').read_bytes())
assert sha((ROOT / 'dev/lowiropt').read_bytes()) == sha((art / 'reviewed-lowiropt').read_bytes())
def failures(name):
    return set(re.findall(r'^(pa32/[^:]+): ERROR:', (art / name).read_text(), re.M))

assert failures('entry-stage.log') == failures('stage-final.log') == set(binding['remaining_failures'])
assert len(binding['remaining_failures']) == 41
assert '178 / 219' in (art / 'stage-final.log').read_text()
assert '5178 / 5178' in (art / 'prior-final.log').read_text()
checks = {r['name']: r for r in json.loads((art / 'checks-final.json').read_text())}
for name in ['prior', 'file', 'local', 'dataflow', 'calls', 'trace']:
    assert checks[name]['exit_code'] == 0, name
assert checks['stage']['exit_code'] == checks['debug']['exit_code'] == 2
assert '119 execution cases' in (art / 'audit-reducers-final.log').read_text()
assert 'PASS (25/25)' in (art / 'debug-replay-final.log').read_text()
assert 'PASS' in (art / 'trace-final.log').read_text()
# Same-level images show that the audit corrections did not alter these hot
# workloads; timing fluctuations on identical images are not runtime claims.
for name in ['common-audit-o0.json', 'common-audit-o1.json']:
    data = json.loads((EVIDENCE / name).read_text())
    for images in data['images'].values():
        assert images['A']['object_sha256'] == images['B']['object_sha256']

if '--pre-record' not in sys.argv:
    plan = (ROOT / 'pa32/plan.md').read_text()
    audit = (ROOT / 'pa32/audit.md').read_text()
    tip = re.search(r'^Last reviewed commit: ([0-9a-f]{40})$', plan, re.M).group(1)
    assert 'Last reviewed commit: ' + tip in audit
    assert tip != binding['review_base']
    assert not git('diff', tip, '--', 'dev', 'student.tests/pa32')
    assert not git('status', '--porcelain')
print(json.dumps(dict(checkpoint_audit=True, stage_complete=False, inherited=history,
                      current_observations=current, unchanged_failures=41, total_tests=219)))

#!/usr/bin/env python3
"""Verify the PA16 implementation handoff against frozen artifacts and source."""
from pathlib import Path
import hashlib
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
RECORD = ROOT / 'student.tests/pa16/handoff.json'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def checked_file(record):
    path = Path(record['path'])
    if not path.is_absolute():
        path = ROOT / path
    assert sha(path) == record['sha256'], str(path)
    return path


def failures(path):
    return set(re.findall(r'(pa16/tests/[^:]+): ERROR:', path.read_text()))


record = json.loads(RECORD.read_text())
for source in record['source_files']:
    checked_file(source)
for item in record['evidence_files']:
    checked_file(item)
subprocess.run(['git', 'diff', '--exit-code', record['stage_base'], '--',
                'pa16/tests', 'pa16/scripts', 'pa16/README.md', 'pa16/Makefile',
                'pa16/pa16.gram', 'spec.md', 'TESTING_AND_REFERENCES.md'], cwd=ROOT, check=True)
assert len(list((ROOT / 'pa16/tests').rglob('*.t'))) == 154
logs = {name: checked_file(item) for name, item in record['logs'].items()}
before, after = failures(logs['baseline']), failures(logs['stage'])
assert len(before) == 109 and len(after) == 91 and not after - before
assert sorted(before - after) == record['fixed_tests']
assert 'TEST SUMMARY: 63 / 154 TESTS PASSED' in logs['stage'].read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)' in logs['prior'].read_text()
assert 'File audit passed for pa16' in logs['audit'].read_text()
assert '21 native and 10 rejection controls passed' in logs['scalar'].read_text()
assert '19 native and 13 rejection controls passed' in logs['floating'].read_text()
assert sha(ROOT / 'dev/cppgm++') == record['compiler_sha256']
expected_names = {f'{group}-{n}' for group in ('calls', 'template', 'memory-float', 'loops', 'floating', 'array-reads')
                  for n in (1000, 4000)} | {f'runtime-{group}' for group in ('calls', 'memory', 'floating', 'constant')}
observations = 0
for campaign in record['campaigns']:
    data = json.loads(checked_file(campaign).read_text())
    assert data['finished_utc'] and set(data['workloads']) == expected_names
    for binary in data['binaries']:
        checked_file(binary)
    checked_file(data['backend'])
    for name, workload in data['workloads'].items():
        assert sha(workload['source_path']) == workload['source_sha256']
        for output in workload['outputs']:
            checked_file(output)
            if 'native' in output:
                checked_file(output['native'])
        common = workload['comparison'] == 'exact'
        if common:
            assert len({output['sha256'] for output in workload['outputs']}) == 1
            if 'runtime' in workload:
                assert len({output['native']['sha256'] for output in workload['outputs']}) == 1
        else:
            assert workload['entry_probe']['exit_code'] != 0
        for phase in ('compiler', 'runtime'):
            if phase not in workload:
                continue
            samples = workload[phase]
            expected = [0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0] if common else [1] * 6
            assert [row['binary'] for row in samples['observations']] == expected
            assert [row['binary'] for row in samples['warmups']] == ([0, 1] if common else [1])
            for row in samples['warmups'] + samples['observations']:
                assert row['checked_exit'] == 0 and row['wall_s'] > 0 and row['rss_kib'] > 0
                observations += 1
plan = (ROOT / 'pa16/plan.md').read_text()
assert f"Stage base commit: `{record['stage_base']}`" in plan
assert f"Last reviewed commit: `{record['stage_base']}`" in plan
print(f'PASS: unchanged 154-fixture contract; 18 fixed failures; earlier 2112 checks; file audit; '
      f'63 personal controls; {len(record["campaigns"])} frozen campaigns / {observations} timing observations')

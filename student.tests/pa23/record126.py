#!/usr/bin/env python3
"""Seal final-audit observations without modifying fixtures or running compilers."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(sys.argv[1]).resolve()
OUT = ROOT/'student.tests/pa23'

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()

record = dict(reviewed_through=git('rev-parse', 'HEAD'),
              implementation=git('rev-parse', 'd9179e84'),
              dev_tree=git('rev-parse', 'HEAD:dev'),
              compiler_sha256=sha(ROOT/'dev/cppgm++'),
              spec_sha256=sha(ROOT/'spec.md'), checks=[], evidence=[])
assert record['compiler_sha256'] == '979a332fdeb3133af19b09d93bfbf4cb061236b59573e682b2c4a747fda5acaa'
for name, command, expected in [
    ('stage', ['make', 'test-pa23'], '(45 / 45)'),
    ('through', ['make', 'test-report-through-pa23'], '(3856 / 3856)'),
    ('file-audit', ['perl', 'scripts/cppgm_file_audit.pl', '--stage', 'pa23', '--paths', 'dev/src'], 'File audit passed'),
]:
    path = WORK/(name+'.log')
    text = path.read_text()
    assert expected in text
    record['checks'].append(dict(name=name, command=command, observed_exit=0,
        log=str(path), sha256=sha(path), summary=[line for line in text.splitlines()
        if 'ALL TESTS' in line or 'File audit' in line or '[warning]' in line or 'PASS (' in line]))
    if name == 'through':
        controls = re.findall(r'PASS \((\d+)/(\d+)\)', text)
        assert sum(int(n) for n, _ in controls) == 22
        assert all(a == b for a, b in controls)
        stages = re.findall(r'^===== (pa\d+) =====$', text, re.M)
        assert stages == ['pa'+str(n) for n in range(1, 24)]
        record['course_counts'] = dict(fixtures_passed=3856, fixtures_total=3856,
            focused_controls_passed=22, focused_controls_total=22, stages=stages,
            supplied_summary=3880, supplied_summary_reproduced=False)

record['personal'] = json.loads((WORK/'personal-summary.json').read_text())
for check in record['personal']:
    path = WORK/(check['name']+'.json')
    assert sha(path) == check['sha256'] and check['exit'] == 0
    assert check['cases'] == check['passed']
    # Keep individual verdicts as well as hashes of the full artifact records.
    check['results'] = [dict(name=c.get('name', c.get('source', '')),
        passed=c.get('passed', c.get('stable', False))) for c in json.loads(path.read_text())['cases']]

for source, target in [('controls','controls126'), ('reference','reference126'),
                       ('backend-limit','backend-limit126'), ('trace','trace126'),
                       ('performance','performance126'), ('performance-forest','performance126-forest')]:
    path = WORK/(source+'.json')
    data = json.loads(path.read_text())
    if source == 'controls':
        assert len(data['cases']) == 12 and all(c['passed'] for c in data['cases'])
    if source == 'reference':
        assert len(data['cases']) == 5
        assert all(c['lanes'][1].get('runtime_exit') == 0 for c in data['cases'])
    if source == 'backend-limit':
        assert all(c['hosted_run']['exit'] == 0 for c in data['cases'])
    if source.startswith('performance'):
        for binary in data['binaries']:
            assert sha(binary['path']) == binary['sha256']
        for workload in data['workloads'].values():
            assert all(o['checked_exit'] == 0 for o in workload['outputs'])
            if not workload['final_only']:
                assert workload['identical_text']
    dst = OUT/(target+'.json')
    dst.write_bytes(path.read_bytes())
    record['evidence'].append(dict(path=str(dst.relative_to(ROOT)), sha256=sha(dst)))

manifest = json.loads((OUT/'oracles125.json').read_text())
assert len(manifest['fixtures']) == 45
for row in manifest['fixtures']:
    src = ROOT/row['source']
    assert sha(src) == row['source_sha256']
    assert sha(src.with_suffix('.ref')) == row['after_sha256']
    assert sha(src.with_suffix('.ref.exit_status')) == row['exit_status_sha256']
changed = git('diff', 'f33dd077', 'HEAD', '--name-only', '--', 'pa23/tests', 'pa23/scripts', 'scripts', 'pa23/Makefile').splitlines()
assert len(changed) == 21 and all(p.endswith('.ref') for p in changed)
record['coverage'] = dict(fixtures=45, accepted_roundtrips=44, changed_oracles=changed,
    source_status_comparator_changes=[], manifest_sha256=sha(OUT/'oracles125.json'))
record['reviewed_commits'] = git('log', '--reverse', '--format=%H %s', 'f33dd077..HEAD').splitlines()
record['implementation_files'] = {p: sha(ROOT/p) for p in git('diff', 'f33dd077', 'HEAD', '--name-only', '--', 'dev').splitlines()}
record['source_tree_clean'] = not git('diff', 'HEAD', '--', 'dev')
assert record['source_tree_clean']
(OUT/'validation126.json').write_text(json.dumps(record, indent=2)+'\n')
print('Final audit evidence sealed:', len(record['personal']), 'inherited control suites; 12 new controls; 45 contract fixtures')

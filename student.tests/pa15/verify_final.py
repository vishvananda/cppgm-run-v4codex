#!/usr/bin/env python3
"""Check final PA15 evidence without rewriting historical handoffs or gates."""
from pathlib import Path
import hashlib
import json
import os
import re
import statistics
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
ART = Path(os.environ['RALPH_ARTIFACT_DIR'])
FINAL = ART / 'pa15-final-audit'
BASE = '8000f3c8ef4647d57f2c0775192585f14cab33d8'
ENTRY = 'd97445d4'
TIP = 'dff6a92b'
ORDER = [0] * 4 + [0, 1, 1, 0] * 2

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def read(path):
    return json.loads(Path(path).read_text())

# A historical experiment belongs to its actual harness, not the current file
# at the same pathname. Match content hashes against retained git blobs/files.
harnesses = {}
for line in git('rev-list', '--objects', '--all', '--', 'student.tests/pa15').splitlines():
    fields = line.split(' ', 1)
    if len(fields) == 2 and fields[1].endswith('.py'):
        data = subprocess.check_output(['git', 'cat-file', 'blob', fields[0]], cwd=ROOT)
        harnesses[hashlib.sha256(data).hexdigest()] = 'git:' + line
for directory in [ROOT / 'student.tests/pa15', *ART.glob('pa15-*'), Path('/tmp/pa15-loop34'), Path('/tmp/pa15-loop35')]:
    for path in directory.glob('*.py'):
        harnesses[sha(path)] = str(path)

checkpoint = read(ROOT / 'student.tests/pa15/checkpoint-evidence.json')
paths = [Path(c['path']) for c in checkpoint['performance_campaigns']]
for c in checkpoint['performance_campaigns']:
    assert sha(c['path']) == c['sha256'], c['path']
paths += [Path(p) for p in read(ROOT / 'student.tests/pa15/matching-handoff.json')['campaigns']]
paths += [ROOT / 'student.tests/pa15' / ('initialization-performance' + suffix + '.json')
          for suffix in ['-preliminary', '-address', '', '-repeat', '-repeat-instrumented']]
paths += [ROOT / 'student.tests/pa15/execution-performance.json', FINAL / 'stage-performance.json', FINAL / 'owner-performance.json']
paths += sorted(FINAL.glob('owner-repeat-performance.json'))
campaigns = []
observations = 0

def samples(m, common=True):
    rows = m['observations']
    assert [r['binary'] for r in rows] == (ORDER if common else [1] * 6)
    for r in rows + m['warmups']:
        assert r['checked_exit'] == 0 and r['wall_s'] > 0 and r['rss_kib'] >= 0
    if common:
        ratios = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1) /
                  statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in (4, 8)]
        assert ratios == m['paired_b_over_a']
        if 'aa_range_s' in m:
            assert m['aa_range_s'] == [min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])]
    return len(rows) + len(m['warmups'])

for path in paths:
    p = read(path)
    assert p['harness_sha256'] in harnesses, (path, 'unavailable frozen harness')
    if 'shared_harness_sha256' in p:
        assert sha(ROOT / 'student.tests/pa10/benchmark.py') == p['shared_harness_sha256']
    for b in p['binaries']:
        assert sha(b['path']) == b['sha256'], b['path']
    count = 0
    if 'backend' in p:
        assert p['finished_utc'] and sha(p['backend']['path']) == p['backend']['sha256']
    else:
        original = ROOT / 'student.tests/pa15/initialization-performance.json'
        assert sha(original) == p['campaign_sha256']
    for name, w in p['workloads'].items():
        if 'observations' in w:
            assert w['source_sha256'] == read(original)['workloads'][name]['source_sha256']
            count += samples(w)
            continue
        assert sha(w['source_path']) == w['source_sha256'], w['source_path']
        for output in w['outputs']:
            assert sha(output['path']) == output['sha256'], output['path']
            if 'native' in output:
                native = output['native']
                assert sha(native['path']) == native['sha256'] and native['checked_exit'] == 0
        common = w.get('common_correct', w.get('comparison') in ('exact', 'native-equivalent'))
        if common and w.get('comparison') != 'native-equivalent':
            assert w['outputs'][0]['sha256'] == w['outputs'][1]['sha256'], name
            if 'runtime' in w:
                assert w['outputs'][0]['native']['sha256'] == w['outputs'][1]['native']['sha256'], name
        for phase in ('compiler', 'runtime'):
            if phase in w:
                count += samples(w[phase], common)
    observations += count
    campaigns.append(dict(path=str(path), sha256=sha(path), invocations=count,
                          harness=harnesses[p['harness_sha256']]))

owners = read(FINAL / 'owner-performance.json')
for n in (1000, 4000):
    w = owners['workloads'][f'nested-packs-{n}']['outputs'][1]['telemetry'][0]
    assert w['semantic_pack_expansion_work'] == 18 and w['semantic_pack_expansion_lanes'] == 7*n
    assert w['template_body_transitions'] == 3*n and w['template_source_regions'] == 9
    w = owners['workloads'][f'calls-{n}']['outputs'][1]['telemetry'][0]
    assert w['semantic_constant_bodies'] == 1 and w['semantic_constant_activations'] == n
    assert w['semantic_constant_execution_steps'] == 5*n and w['semantic_constant_execution_hits'] == n
w = owners['workloads']['ordered-patterns-4000']['outputs'][1]['telemetry'][0]
assert w['class_pattern_ordering_work'] == 2 and w['class_pattern_ordering_hits'] == 7998
assert owners['workloads']['storage-4000']['outputs'][1]['telemetry'][0]['semantic_body_checks'] == 0

for p in [read(FINAL / 'stage-performance.json'), owners]:
    assert not p['source_diff']
    assert git('rev-parse', p['implementation_commit'] + ':dev') == git('rev-parse', TIP + ':dev')
    assert sha(ROOT / 'dev/cppgm++') == p['binaries'][1]['sha256']
assert git('rev-parse', 'HEAD:dev') == git('rev-parse', TIP + ':dev')
assert not git('diff', 'HEAD', '--', 'dev')

correction = read(ROOT / 'student.tests/pa15/aggregate-reference-correction.json')
assert sha(ROOT / correction['fixture']) == correction['source_sha256']
assert sha(ROOT / correction['reducer']) == correction['reducer_sha256']
ref = str(Path(correction['fixture']).with_suffix('.ref'))
assert sha(ROOT / ref) == correction['corrected_ref_sha256']
assert hashlib.sha256(subprocess.check_output(['git', 'show', BASE + ':' + ref], cwd=ROOT)).hexdigest() == correction['original_ref_sha256']
changed_fixtures = git('diff', '--name-only', BASE, 'HEAD', '--', *[f'pa{n}/tests' for n in range(1, 16)]).splitlines()
assert changed_fixtures == [ref], changed_fixtures
protected = ['scripts', 'Makefile', 'spec.md', 'TESTING_AND_REFERENCES.md', 'pa15/README.md', 'reference-binaries/manifest.tsv']
assert not git('diff', BASE, '--', *protected)
reference_recheck = read(FINAL / 'reference-recheck.json')
for r in reference_recheck:
    assert r['native_exit'] == (0 if r['compiler'] == 'student' else 1)
    assert sha(FINAL / (r['compiler'] + '-relocation.lowir')) == r['lowir_sha256']
    assert sha(FINAL / (r['compiler'] + '-relocation')) == r['native_sha256']
assert reference_recheck[1]['lowir_sha256'] == correction['reference_lowir_sha256']

checks = {}
for name, expected in {
    'through': 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)',
    'file-audit': 'File audit passed for pa15 with 3 warning(s).',
    'constants': '10 constant groups passed', 'value_arguments': '26 value argument groups passed',
    'specializations': '24 native controls; 13 rejection controls passed',
    'packs': '27 native controls; 8 rejections passed',
    'class_patterns': '15 native and 7 rejection class-pattern controls passed',
    'initialization': '3 global-address LowIR controls passed',
    'execution': '24 native and 18 rejection controls passed',
    'checkpoint_audit': '11 native and 7 rejection audit controls passed',
    'final_audit': '17 native and 11 rejection final-audit controls passed',
}.items():
    path = FINAL / ('final-' + name + '.log')
    assert expected in path.read_text(), name
    checks[name] = dict(path=str(path), sha256=sha(path), result=expected, exit_code=0)
assert set(re.findall(r'^===== (pa\d+) =====$', (FINAL / 'final-through.log').read_text(), re.M)) == {f'pa{n}' for n in range(1, 16)}
stage_log = FINAL / 'volatile-stage.log'
assert 'ALL TESTS PASSED SUCCESSFULLY! (177 / 177)' in stage_log.read_text()
checks['stage'] = dict(path=str(stage_log), sha256=sha(stage_log), result='177/177', exit_code=0)
for filename in ('plan.md', 'audit.md'):
    text = (ROOT / 'pa15' / filename).read_text()
    assert f'Last reviewed commit: `{git("rev-parse", TIP)}`' in text
    assert f'Stage base commit: `{BASE}`' in text

evidence = dict(stage='pa15 full-stage', stage_base_commit=BASE, audit_entry_commit=git('rev-parse', ENTRY),
                last_reviewed_commit=git('rev-parse', TIP), source_tree=git('rev-parse', TIP + ':dev'),
                reviewed_commits=git('rev-list', '--reverse', BASE + '..' + TIP).splitlines(),
                reviewed_source_files=git('diff', '--name-only', BASE, TIP, '--', 'dev').splitlines(),
                checks=checks, performance_campaigns=campaigns, performance_invocations=observations,
                reference_corrections=[correction], reference_recheck=reference_recheck,
                binary_sha256=sha(ROOT / 'dev/cppgm++'), verifier_sha256=sha(__file__),
                fixture_trees={f'pa{n}/tests': git('rev-parse', f'HEAD:pa{n}/tests') for n in range(1, 16)},
                required_remaining=[], status='full-stage audit complete; all 15 stages pass')
if '--write' in sys.argv:
    (ROOT / 'student.tests/pa15/final-evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
else:
    assert read(ROOT / 'student.tests/pa15/final-evidence.json') == evidence, 'stale final evidence ledger'
print(f'PASS: {len(campaigns)} frozen campaigns / {observations} invocations; current code, work counts, fixtures and final checks verified')

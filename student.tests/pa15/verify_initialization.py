#!/usr/bin/env python3
"""Verify the frozen PA15 initialization handoff artifacts; run explicitly."""
from pathlib import Path
import hashlib, json, re, statistics, subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('/tmp/pa15-loop34')
ENTRY = 'b8379f52f55da98614f5a03d499d34a062ca8fe0'
IMPLEMENTATION = '4f4bea42e7d5ca28810fb4fb40fb29a58066d12b'
ORDER = [0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0]
artifacts = {}

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).decode().strip()

def sha(path):
    path = Path(path)
    if not path.is_absolute(): path = ROOT / path
    value = hashlib.sha256(path.read_bytes()).hexdigest()
    artifacts[str(path)] = value
    return value

def checked(path, expected):
    assert sha(path) == expected, path

def load(path):
    sha(path)
    return json.loads(Path(path).read_text())

def sample_check(measurement, common=True):
    rows = measurement['observations']
    assert [r['binary'] for r in rows] == (ORDER if common else [1] * 6)
    for r in measurement['warmups'] + rows:
        assert r['checked_exit'] == 0 and r['wall_s'] > 0 and r['rss_kib'] >= 0
    if common:
        ratios = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1) /
                  statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in [4, 8]]
        assert ratios == measurement['paired_b_over_a']
    return len(rows) + len(measurement['warmups'])

base = ROOT / 'student.tests/pa15'
campaigns = [base / ('initialization-performance' + suffix + '.json')
             for suffix in ['-preliminary', '-address', '']]
invocations = 0
for path in campaigns:
    d = load(path)
    assert d['finished_utc'] and d['flags'] == ['--emit-lowir', '-O0']
    for b in d['binaries']: checked(b['path'], b['sha256'])
    checked(d['backend']['path'], d['backend']['sha256'])
    checked(ROOT / 'student.tests/pa10/benchmark.py', d['shared_harness_sha256'])
    for name, w in d['workloads'].items():
        checked(w['source_path'], w['source_sha256'])
        common = w['comparison'] in ['exact', 'native-equivalent']
        invocations += sample_check(w['compiler'], common)
        if 'runtime' in w: invocations += sample_check(w['runtime'])
        for output in w['outputs']:
            checked(output['path'], output['sha256'])
            assert Path(output['path']).stat().st_size == output['bytes']
            if 'native' in output:
                native = output['native']; checked(native['path'], native['sha256'])
                assert native['checked_exit'] == 0
                assert Path(native['path']).stat().st_size == native['file_bytes']
                assert native['text_bytes'] + native['data_bytes'] < native['file_bytes']
        if w['comparison'] == 'exact':
            assert w['outputs'][0]['sha256'] == w['outputs'][1]['sha256']
            if 'runtime' in w:
                assert w['outputs'][0]['native']['sha256'] == w['outputs'][1]['native']['sha256']

final = load(campaigns[-1])
checked(ROOT / 'dev/cppgm++', final['binaries'][1]['sha256'])
checked(base / 'initialization_benchmark.py', final['harness_sha256'])
for name in ['runtime-array-16', 'runtime-array-64']:
    outputs = final['workloads'][name]['outputs']
    assert outputs[0]['native']['sha256'] == outputs[1]['native']['sha256']
    assert outputs[1]['native']['data_bytes'] == 0
for suffix, harness in [('-repeat', 'initialization_repeat.py'),
                        ('-repeat-instrumented', 'initialization_repeat_instrumented.py')]:
    d = load(base / ('initialization-performance' + suffix + '.json'))
    checked(campaigns[-1], d['campaign_sha256']); checked(base / harness, d['harness_sha256'])
    for w in d['workloads'].values(): invocations += sample_check(w)

entry_log = WORK / 'entry-handoff.log'
if not entry_log.exists():
    entry_log.write_bytes(Path('/home/vishvananda/work/.ralph/v4codex-gpt-6-astra-xhigh/last-test.log').read_bytes())
checks = load(WORK / 'required-handoff.json')
checks['entry'] = dict(command='Ralph turn-entry PA15 report', exit=2, log=str(entry_log))
checks['stage'] = dict(command='make test-pa15', exit=2, log=str(WORK / 'stage-bounded.log'))
for name, expected in [('entry', (169,177)), ('stage', (173,177)),
                       ('prior-handoff', (1935,1935)), ('through-handoff', (2108,2112))]:
    record = checks[name]; text = Path(record['log']).read_text(); sha(record['log'])
    match = re.search(r'^===== (?:ALL TESTS PASSED SUCCESSFULLY! \(|TEST SUMMARY: )(\d+)\s*/\s*(\d+)', text, re.M)
    assert match and tuple(map(int, match.groups())) == expected, name
    record['passed'], record['total'] = expected
    record['failures'] = sorted(set(re.findall(r'^(pa\d+/tests/\S+\.t): ERROR:', text, re.M)))
    assert len(record['failures']) == expected[1] - expected[0]
assert set(checks['stage']['failures']) < set(checks['entry']['failures'])
assert checks['stage']['failures'] == checks['through-handoff']['failures']
audit = checks['file-audit-handoff']; sha(audit['log'])
assert audit['exit'] == 0 and 'File audit passed for pa15' in Path(audit['log']).read_text()
for name, script, expected in [
    ('initialization-bounded', 'initialization.py', '3 global-address LowIR controls passed'),
    ('values-final', 'value_arguments.py', '26 value argument groups passed'),
    ('constants-final', 'constants.py', '10 constant groups passed'),
    ('specializations-final', 'specializations.py', '24 native controls; 13 rejection controls passed'),
    ('packs-final', 'packs.py', '27 native controls; 8 rejections passed'),
    ('matching-final', 'class_patterns.py', '15 native and 7 rejection class-pattern controls passed'),
    ('checkpoint-final', 'checkpoint_audit.py', '11 native and 7 rejection audit controls passed')]:
    path = WORK / (name + '.log'); sha(path); sha(base / script)
    assert Path(path).read_text().rstrip().endswith(expected)
    checks[name] = dict(command='python3 student.tests/pa15/' + script, exit=0, log=str(path))

correction = load(base / 'aggregate-reference-correction.json')
fixture = correction['fixture']; ref = fixture[:-2] + '.ref'
checked(fixture, correction['source_sha256']); checked(ref, correction['corrected_ref_sha256'])
checked(correction['reducer'], correction['reducer_sha256'])
original = subprocess.check_output(['git', 'show', ENTRY + ':' + ref], cwd=ROOT)
assert hashlib.sha256(original).hexdigest() == correction['original_ref_sha256']
for label, compiler, expected in [('student', ROOT / 'dev/cppgm++', 0),
                                  ('reference', ROOT / 'dev/cppgm++-ref', 1)]:
    ir = WORK / (label + '-relocation-handoff.lowir'); exe = ir.with_suffix('.exe')
    subprocess.run([compiler, '--emit-lowir', '-O0', '-o', ir, ROOT / correction['reducer']], check=True)
    subprocess.run([ROOT / 'dev/lowir2native-ref', '-O0', '-o', exe, ir], check=True)
    assert subprocess.run([exe]).returncode == expected
    checked(ir, correction[label + '_lowir_sha256'])
    checked(exe, correction[label + '_executable_sha256'])

protected = ['scripts', 'reference-binaries', 'Makefile', 'spec.md', 'AGENTS.md',
             'TESTING_AND_REFERENCES.md']
for n in range(1,35): protected.extend([f'pa{n}/tests', f'pa{n}/Makefile', f'pa{n}/README.md'])
assert git('diff', '--name-only', ENTRY, '--', *protected).splitlines() == [ref]
assert len(git('ls-files', 'pa15/tests/*.t').splitlines()) == 177
assert not git('diff', IMPLEMENTATION, '--', 'dev')
markers = dict(re.findall(r'^(Stage base commit|Last reviewed commit): `([^`]+)`',
                         (ROOT / 'pa15/plan.md').read_text(), re.M))
old_markers = dict(re.findall(r'^(Stage base commit|Last reviewed commit): `([^`]+)`',
                             git('show', ENTRY + ':pa15/plan.md'), re.M))
assert markers == old_markers and len(markers) == 2
handoff = dict(status='validated incomplete implementation handoff; independent audit remains',
    entry_commit=ENTRY, implementation_commit=IMPLEMENTATION,
    dev_tree=git('rev-parse', IMPLEMENTATION + ':dev'), review_markers=markers,
    binary_sha256=final['binaries'][1]['sha256'], checks=checks,
    reference_changes=[ref], fixture_count=177, protected_paths=protected,
    protected_changes_only_proven_reference=True, measured_invocations=invocations,
    artifacts=artifacts, remaining_implementation=['constant execution (two required fixtures)',
        'member storage demand (one fixture)', 'ordinary body validation (one fixture)'],
    boundary='Completed initialization prerequisites, scalar constant-array backing and retained update queries; next owners require checked execution frames or separate validation/storage/emission dependency facts.')
(base / 'initialization-handoff.json').write_text(json.dumps(handoff, indent=2) + '\n')
print('Verified frozen evidence, reference reducer, unchanged coverage and 8 -> 4 failures')

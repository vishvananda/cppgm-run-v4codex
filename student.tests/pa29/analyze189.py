#!/usr/bin/env python3
"""Verify and bind the PA29 implementation189 handoff to authoritative artifacts."""
import hashlib
import json
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
scratch = pathlib.Path(sys.argv[1]).resolve()
out = root / 'student.tests/pa29/evidence189'
entry = '54ddacb1ced906e692ea94bde5110b0757e1b2af'
implementation = subprocess.check_output(['git', 'rev-parse', '383fae65'], cwd=root, text=True).strip()
entry_plan = subprocess.check_output(['git', 'show', entry + ':pa29/plan.md'], cwd=root, text=True)
current_plan = (root / 'pa29/plan.md').read_text()
for marker in ['Stage base commit', 'Last reviewed commit', 'Previous reviewed commit']:
    pattern = re.escape(marker) + r': `([^`]+)`'
    assert re.search(pattern, entry_plan).group(1) == re.search(pattern, current_plan).group(1)


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def save(name, value):
    (out / (name + '.json')).write_text(json.dumps(value, indent=2) + '\n')


def read(p):
    return json.loads(p.read_text())


datasets = {}
for dest, directory, count in [('common', 'common-handoff', 224), ('owner', 'owner-handoff', 336),
                               ('intermediate-common', 'common-final', 224),
                               ('intermediate-owner', 'owner-final', 336),
                               ('preliminary-common', 'common-performance', 224),
                               ('preliminary-owner', 'owner-performance', 168)]:
    d = read(scratch / directory / 'performance.json')
    assert len(d['runs']) == count
    assert all(r['status'] == 0 for r in d['runs'])
    for v in d['binaries'].values():
        assert sha(pathlib.Path(v['path'])) == v['sha256']
    for name, images in d['images'].items():
        assert images['A'] == images['B'], (dest, name)
        for mode in ['compile', 'runtime']:
            rows = [r for r in d['runs'] if r['workload'] == name and r['mode'] == mode]
            assert ''.join(r['label'] for r in rows) == 'AAAA' + 'ABBA' * 6
    if 'owner' in dest:
        for v in d['inputs'].values():
            assert hashlib.sha256(v['source'].encode()).hexdigest() == v['sha256']
        assert len(d['launchers']) == 8
    datasets[dest] = d
    save(dest + '-performance', d)
assert datasets['common']['binaries'] == datasets['owner']['binaries']
assert sha(root / 'dev/cppgm++') == datasets['common']['binaries']['B']['sha256']
assert datasets['common']['binaries']['A']['sha256'] == read(root / 'student.tests/pa29/evidence188/source-binding.json')['binaries']['B']['sha256']
assert datasets['common']['images'] == datasets['preliminary-common']['images']
assert datasets['common']['images'] == datasets['intermediate-common']['images']
assert datasets['owner']['images'] == datasets['intermediate-owner']['images']
for name, images in datasets['preliminary-owner']['images'].items():
    assert images == datasets['owner']['images'][name]

# Work counters, not wall-time ratios, establish the bounded per-size work.
scaling = {}
for kind in ['demand', 'contexts']:
    points = []
    for n in [600, 1200, 2400]:
        rows = [r for r in datasets['owner']['runs'] if r['workload'] == kind + str(n)
                and r['mode'] == 'compile' and r['label'] == 'B']
        counters = [{k: v for phase in r['counters'] for k, v in phase.items()} for r in rows]
        keys = ['tokens', 'parsed_nodes', 'nodes', 'semantic_specializations',
                'template_body_transitions', 'semantic_candidate_work', 'instructions', 'native_functions']
        values = {k: counters[0][k] for k in keys}
        assert all(all(c[k] == v for k, v in values.items()) for c in counters)
        points.append(dict(N=n, counters=values, samples=len(rows)))
    equations = {}
    for k in points[0]['counters']:
        a, b, c = [p['counters'][k] for p in points]
        assert c - b == 2 * (b - a), (kind, k, a, b, c)
        slope = (b - a) / 600
        equations[k] = dict(slope=slope, intercept=a - slope * 600)
    scaling[kind] = dict(points=points, linear_equations=equations)
save('scaling', scaling)

checks = read(scratch / 'controls-handoff-final3/checks.json')
assert len(checks) == 221
assert all((r['status'] == 0) == r['expected_success'] for r in checks)
save('controls', checks)
artifacts = {str(p.relative_to(scratch)): dict(sha256=sha(p), bytes=p.stat().st_size)
             for p in sorted((scratch / 'controls-handoff-final3').iterdir())
             if p.suffix in ['.lowir', '.canonical']}
assert len(artifacts) == 14
save('inspection', dict(artifacts=artifacts, controls=221,
                       dormant_bodies_not_emitted=True, assertion_conversions_not_emitted=True,
                       sources={str(p.relative_to(root)): sha(p)
                                for p in sorted((root / 'student.tests/pa29/controls189').iterdir())}))

validation = read(scratch / 'validation-handoff/validation.json')
assert [r['status'] for r in validation] == [0, 2, 2, 0]
texts = []
for r in validation:
    p = pathlib.Path(r['path'])
    assert sha(p) == r['sha256']
    texts.append(p.read_text())
prior, stage, through, audit = texts
assert 'ALL TESTS PASSED SUCCESSFULLY! (4538 / 4538)' in prior
assert 'TEST SUMMARY: 398 / 403 TESTS PASSED' in stage
assert 'TEST SUMMARY: 4936 / 4941 TESTS PASSED' in through
assert 'File audit passed for pa29 with 4 warning(s).' in audit
failures = re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$', stage, re.M)
assert len(failures) == 5
assert failures == re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$', through, re.M)
old = read(root / 'student.tests/pa29/evidence188/remaining.json')
by_name = {r['test']: r for r in old['failures']}
corrected = [f'pa29/tests/compile/{name}.t' for name in [
    '600-hosted-nothrow-default-constructible-shorthand',
    '700-hosted-char-traits-primary-conversion-shims',
    '700-hosted-nothrow-invocable-cache-default']]
assert set(by_name) - {r[0] for r in failures} == set(corrected)
remaining = []
for name, diagnostic in failures:
    r = by_name[name]
    r['diagnostic'] = [diagnostic]
    r['diagnostic_provenance'] = 'Implementation189 final PA29 and root-through reports.'
    remaining.append(r)
save('remaining', dict(total=403, passed=398, failed=5, entry_failures=8,
                       corrected_reference_cases=corrected, unfinished_implementation=4,
                       independent_contract_questions=1, failures=remaining))

coverage = read(root / 'student.tests/pa29/evidence186/coverage.json')
assert len(coverage['files']) == 1707
allowed = {p[:-2] + '.ref.exit_status' for p in corrected}
changes = {}
for path, expected in coverage['files'].items():
    actual = sha(root / path)
    if actual != expected:
        assert path in allowed, path
        assert (root / path).read_text() == 'EXIT_FAILURE\n'
        original = subprocess.check_output(['git', 'show', entry + ':' + path], cwd=root)
        assert original == b'EXIT_SUCCESS\n'
        changes[path] = dict(before=expected, after=actual)
assert set(changes) == allowed
paths = subprocess.check_output(['git', 'ls-files', 'pa29/tests'], cwd=root, text=True).splitlines()
assert sum(p.endswith('.t') for p in paths) == 403
changed = subprocess.check_output(['git', 'diff', '--name-only', entry, '--', 'pa29/tests',
                                   'pa29/scripts', 'pa29/Makefile', 'TESTING_AND_REFERENCES.md'], cwd=root, text=True).splitlines()
assert set(changed) == allowed
assert not subprocess.check_output(['git', 'diff', entry, '--', *['pa' + str(n) for n in range(1,29)]], cwd=root)
save('coverage', dict(entry=entry, total_stage_inputs=403, compared_contract_harness_paths=1707,
                      unchanged_paths=1704, original_inputs_unchanged=True,
                      discovery_and_comparison_unchanged=True, changes=changes,
                      proof='pa29/reference-corrections189.md', correction_id='pa29-189-template-demand'))

sources = subprocess.check_output(['git', 'diff', '--name-only', entry, implementation, '--', 'dev'], cwd=root, text=True).splitlines()
assert not subprocess.check_output(['git', 'diff', implementation, '--', 'dev'], cwd=root)
for p in sources:
    assert hashlib.sha256(subprocess.check_output(['git', 'show', implementation + ':' + p], cwd=root)).hexdigest() == sha(root / p)
save('source-binding', dict(entry=entry, implementation=implementation,
                            binaries=datasets['common']['binaries'], files={p: sha(root / p) for p in sources}))
save('validation', dict(checks=validation, priorThroughTests='pass:4538/4538', fileAudit='pass',
                        stageProgress='pass:8 -> 5 original failures; documented reference corrections; 403 inputs retained',
                        stageTests='398/403; exit 2', rootThrough='4936/4941; only PA29 fails'))
sizes = {}
for label, path in [('entry', scratch/'cppgm-A'), ('initial', scratch/'cppgm-B'), ('access', scratch/'cppgm-final'), ('final', scratch/'cppgm-handoff')]:
    words = subprocess.check_output(['size', str(path)], text=True).splitlines()[1].split()
    sizes[label] = dict(text=int(words[0]), data=int(words[1]), bss=int(words[2]), sha256=sha(path))
save('performance-summary', dict(compiler_sizes=sizes, summaries={k:d['summary'] for k,d in datasets.items()}))

records = [root / p for p in ['pa29/plan.md', 'pa29/handoff189.md', 'pa29/performance189.md',
                             'pa29/reference-corrections189.md', 'student.tests/pa29/check189.py',
                             'student.tests/pa29/performance189.py', 'student.tests/pa29/analyze189.py']]
if all(p.exists() for p in records):
    save('manifest', dict(entry=entry, implementation=implementation,
                         stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',
                         last_reviewed='2df00585bd10d4e2e068934394dffc8adb0a47ed',
                         observations=1512, launchers=24,
                         checks=dict(priorThroughTests='pass', fileAudit='pass', stageProgress='pass',
                                     stageTests='398/403; five failures remain'),
                         files={str(p.relative_to(root)):sha(p) for p in [*sorted(out.glob('*.json')), *records]
                                if p.name != 'manifest.json'}))
print('Verified 1512 observations, 221 controls, 403 inputs and three proven reference corrections; failures 8 -> 5.')

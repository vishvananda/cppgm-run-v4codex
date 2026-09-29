#!/usr/bin/env python3
"""Bind final audit evidence to the current checkout: WORK artifact directory."""
from pathlib import Path
import hashlib, json, subprocess, sys

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(sys.argv[1]).resolve()
VALIDATION = WORK/'validation-reviewed'
BASE = '94dcb8ad21664137e87d574e878c14a4a047348a'
ENTRY = 'f5c942ff'
def sha(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()
def read(p):
    return json.loads(Path(p).read_text())
def artifact(p):
    return dict(path=str(p), sha256=sha(p))

assert not git('diff', 'HEAD', '--', 'dev')
checks = read(VALIDATION/'checks.json')
assert len(checks) == 71 and all(c['exit'] == 0 for c in checks.values())
for c in checks.values():
    assert sha(c['path']) == c['sha256']
assert '420 / 420' in (VALIDATION/'stage.log').read_text()
assert '2609 / 2609' in (VALIDATION/'prior.log').read_text()
assert '3029 / 3029' in (VALIDATION/'through.log').read_text()
assert 'File audit passed' in (VALIDATION/'file-audit.log').read_text()
compiler = sha(ROOT/'dev/cppgm++')
assert compiler == read(VALIDATION/'stage-progress.json')['compiler_sha256']
before = read(WORK/'entry-controls-final/results.json')
after = read(VALIDATION/'audit90_controls/results.json')
assert [(r['name'], r['source_sha256'], r['expected']) for r in before] == [
    (r['name'], r['source_sha256'], r['expected']) for r in after]
assert len(after) == 49 and sum(r['passed'] for r in before) == 13
assert all(r['passed'] for r in after)
personal = {}
for p in sorted(VALIDATION.rglob('results.json')):
    rows = read(p)
    if isinstance(rows, list) and rows and all(isinstance(r, dict) and 'passed' in r for r in rows):
        assert all(r['passed'] for r in rows)
        personal[str(p.relative_to(VALIDATION))] = dict(count=len(rows), **artifact(p))
assert len(personal) == 37 and sum(r['count'] for r in personal.values()) == 1605
suites = [f'pa{i}/tests' for i in range(1,19)]
paths = git('ls-files', '--', *suites).splitlines()
assert paths == git('ls-tree', '-r', '--name-only', BASE, '--', *suites).splitlines()
assert not git('diff', ENTRY, '--', *suites, 'scripts', 'Makefile', 'spec.md', 'AGENTS.md')
old = read(ROOT/'student.tests/pa18/loop89-evidence.json')
changed = git('diff', '--name-only', BASE, '--', *suites).splitlines()
assert len(changed) == 29 and set(changed) == set(old['reference_corrections'])
performance = read(ROOT/'student.tests/pa18/loop90-performance.json')
assert performance.get('finished_utc') and len(performance['workloads']) == 99
assert performance['binaries'][1]['sha256'] == compiler
assert performance['harness_sha256'] == sha(ROOT/'student.tests/pa18/benchmark90.py')
assert sum(w['comparison'] == 'exact' for w in performance['workloads'].values()) == 89
traces = []
for name in ('signature79','scalar80','result81','audit82','array83','object84','discard85','audit86','audit88','audit90'):
    traces.append(dict(name=name, source=artifact(ROOT/f'student.tests/pa18/{name}_trace.cpp'),
        lowir=artifact(VALIDATION/(name+'.lowir')), executable=artifact(VALIDATION/(name+'.exe')),
        telemetry=[json.loads(s) for s in (VALIDATION/(name+'-trace.log')).read_text().splitlines()],
        execution_exit=checks[name+'-execution']['exit']))
result = dict(stage_base=BASE, entry=git('rev-parse', ENTRY), reviewed_code=git('rev-parse','HEAD'),
    compiler_sha256=compiler, checks=checks, personal=personal, course=old['course'],
    new_controls=dict(before=artifact(WORK/'entry-controls-final/results.json'),
        after=artifact(VALIDATION/'audit90_controls/results.json'), entry_passes=13, final_passes=49, count=49),
    architecture_trace=traces, reference_corrections=old['reference_corrections'],
    fixture_paths=len(paths), fixture_path_list_sha256=hashlib.sha256('\n'.join(paths).encode()).hexdigest(),
    preserved_entry_audit_sha256=sha(ROOT/'pa18/audit89.md'),
    reviewed_source_sha256={p:sha(ROOT/p) for p in git('ls-files','dev').splitlines() if (ROOT/p).is_file()},
    performance={name:artifact(ROOT/'student.tests/pa18'/name) for name in
        ['loop90-performance-initial.json','loop90-performance.json'] +
        [p.name for p in sorted((ROOT/'student.tests/pa18').glob('loop90-repeat-*.json'))]},
    initial_harness=artifact(WORK/'benchmark90-initial.py'),
    earlier_validation={name:artifact(WORK/(name+'.log')) for name in
        ('validation','validation-final','validation-complete')},
    disposition='PA18/O0 full-stage aligned; no remaining PA18 audit finding; no PA19 advancement')
(ROOT/'student.tests/pa18/loop90-evidence.json').write_text(json.dumps(result,indent=2)+'\n')
print('Bound 71 passing checks, 1605 personal cases, 10 traces and 99 performance workloads.')

#!/usr/bin/env python3
"""Verify and bind the complete audit range, checks, coverage and measurements.

Usage: evidence86.py WORK OUT [CODE_TIP]; run after the reviewed code is committed.
"""
from pathlib import Path
import hashlib, json, re, subprocess, sys

ROOT = Path(__file__).resolve().parents[2]
W = Path(sys.argv[1]).resolve()
OUT = Path(sys.argv[2])
V = W/'validation'
ENTRY = '890f810b'
REVIEW = 'ecc308bc'
BASE = '94dcb8ad21664137e87d574e878c14a4a047348a'
CODE = sys.argv[3] if len(sys.argv)>3 else 'HEAD'
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p): return json.loads(Path(p).read_text())
def git(*args): return subprocess.check_output(['git',*args],cwd=ROOT)
def full(rev): return git('rev-parse',rev).decode().strip()
def artifact(p): return dict(path=str(p),sha256=sha(p))

assert not git('diff',CODE,'--','dev')
checks = read(V/'checks.json')
assert checks['stage']['exit'] == 2
assert all(c['exit'] == 0 for k,c in checks.items() if k != 'stage')
for c in checks.values(): assert sha(c['path']) == c['sha256'], c
assert '417 / 420' in (V/'stage.log').read_text()
assert '2609 / 2609' in (V/'prior.log').read_text()
assert re.findall(r'^===== (pa\d+) =====$',(V/'prior.log').read_text(),re.M) == ['pa'+str(i) for i in range(1,18)]
assert 'File audit passed for pa18' in (V/'file-audit.log').read_text()
progress = read(V/'stage-progress.json')
assert len(progress['entry_failures']) == len(progress['final_failures']) == 3
assert progress['entry_failures'] == progress['final_failures'] and not progress['new_failures']
assert progress['inputs'] == 420 and progress['files'] == 1686
assert len(progress['reference_revisions']) == 23
for p,h in progress['fixture_sha256'].items(): assert sha(ROOT/p) == h
assert progress['compiler_sha256'] == sha(ROOT/'dev/cppgm++') == sha(W/'final-cppgm')

controls = {}
for name,c in read(V/'validate73_controls/results.json').items():
    if 'count' in c: controls[name] = V/'validate73_controls'/name/'results.json'
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls',
             'lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls',
             'signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls',
             'array83_controls','object84_controls','discard85_controls','storage85_controls',
             'audit86_controls','course81','course84'):
    controls[name] = V/name/'results.json'
total = 0
for name,p in list(controls.items()):
    rows = read(p)
    assert all(r['passed'] for r in rows), name
    total += len(rows)
    controls[name] = dict(count=len(rows),**artifact(p))
new = read(V/'audit86_controls/results.json')
entry_controls = read(W/'audit-entry-complete/results.json')
assert len(entry_controls) == len(new)
assert {r['name']:r['source_sha256'] for r in entry_controls} == {r['name']:r['source_sha256'] for r in new}
assert total == 1355 + len(new)

history = {}
for number in (83,84,85):
    p = ROOT/f'student.tests/pa18/loop{number}-evidence.json'
    old = read(p)
    assert old['last_reviewed_commit'] == full(REVIEW)
    for c in old['checks'].values(): assert sha(c['path']) == c['sha256']
    history[number] = dict(**artifact(p),checks=len(old['checks']),controls=old['personal_control_count'])

def inspect_performance(path):
    data = read(path)
    assert data.get('finished_utc')
    for b in data['binaries']: assert sha(b['path']) == b['sha256']
    observations = 0
    for item in [dict(compiler=data['startup']),*data['workloads'].values()]:
        for phase in ('compiler','runtime'):
            if phase not in item: continue
            m = item[phase]
            assert [r['binary'] for r in m['observations']] in ([0]*4+[0,1,1,0]*4,[1]*6)
            rows = m['warmups'] + m['observations']
            assert all(r['checked_exit']==0 and r['wall_s']>0 for r in rows)
            observations += len(rows)
    for name,item in data['workloads'].items():
        assert hashlib.sha256(item['source'].encode()).hexdigest() == item['source_sha256']
        for output in item['outputs']:
            if 'native' in output: assert output['native']['checked_exit'] == 0
    return dict(**artifact(path),observations=observations,workloads=len(data['workloads']))

historical_performance = {}
for record in ('loop83-performance.json','loop84-performance-initial.json','loop84-performance.json',
               'loop85-performance-initial.json','loop85-performance.json'):
    historical_performance[record] = inspect_performance(ROOT/'student.tests/pa18'/record)
perfpath = ROOT/'student.tests/pa18/loop86-performance.json'
performance = inspect_performance(perfpath)
perf = read(perfpath)
assert perf['commits'] == [full(REVIEW),full(CODE)]
assert perf['binaries'][0]['sha256'] == read(ROOT/'student.tests/pa18/loop82-performance.json')['binaries'][1]['sha256']
assert perf['binaries'][1]['sha256'] == progress['compiler_sha256']
assert perf['harness_sha256'] == sha(ROOT/'student.tests/pa18/benchmark86.py')
for number in (83,84,85):
    for name,item in read(ROOT/f'student.tests/pa18/loop{number}-performance.json')['workloads'].items():
        assert perf['workloads'][name]['source_sha256'] == item['source_sha256']

traces = {}
for name in ('signature79','scalar80','result81','audit82','array83','object84','discard85','audit86'):
    traces[name] = dict(source_sha256=sha(ROOT/'student.tests/pa18'/(name+'_trace.cpp')),
                       lowir_sha256=sha(V/(name+'.lowir')),native_sha256=sha(V/(name+'.exe')),
                       telemetry=[json.loads(x) for x in (V/(name+'-trace.log')).read_text().splitlines()])
commits = []
for commit in git('rev-list','--reverse',REVIEW+'..'+CODE).decode().splitlines():
    commits.append(dict(commit=commit,subject=git('show','-s','--format=%s',commit).decode().strip(),
                        patch_sha256=hashlib.sha256(git('show','--format=fuller','--binary',commit)).hexdigest(),
                        paths=git('diff-tree','--no-commit-id','--name-only','-r',commit).decode().splitlines()))
paths = git('diff','--name-only',REVIEW,CODE,'--','dev').decode().splitlines()
result = dict(stage_base_commit=BASE,previous_last_reviewed_commit=full(REVIEW),entry_commit=full(ENTRY),
              last_reviewed_commit=full(CODE),commits=commits,
              combined_patch_sha256=hashlib.sha256(git('diff','--binary',REVIEW,CODE)).hexdigest(),
              implementation_sha256={p:sha(ROOT/p) for p in paths},checks=checks,stage_progress=progress,
              compiler_sha256=progress['compiler_sha256'],controls=controls,personal_control_count=total,
              audit_controls=new,entry_controls=entry_controls,history=history,
              historical_performance=historical_performance,performance=performance,traces=traces,
              disposition='Accumulated checkpoint audit passes; PA18 remains unfinished at 417/420.',
              remaining_work=['Class-result ABI consistency across definitions/calls/indirect signatures',
                              'Inherited class-ellipsis representation at the scalar variadic LowIR boundary'])
OUT.write_text(json.dumps(result,indent=2)+'\n')
print('Verified complete audit:',len(commits),'commits;',total,'controls;',performance['observations'],'performance observations.')

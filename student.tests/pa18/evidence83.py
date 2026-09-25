#!/usr/bin/env python3
"""Verify PA18 array handoff against final authoritative artifacts: WORK OUT."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();OUT=Path(sys.argv[2]);V=W/'validation-complete'
ENTRY='48c864abc19c24c4e9f5706a86109c023e2da7f7'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
checks=read(V/'checks.json');progress=read(V/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for k,c in checks.items() if k!='stage')
for c in checks.values():assert sha(c['path'])==c['sha256']
assert '411 / 420' in (V/'stage.log').read_text() and '2609 / 2609' in (V/'prior.log').read_text()
assert 'File audit passed for pa18' in (V/'file-audit.log').read_text()
assert len(progress['entry_failures'])==24 and len(progress['final_failures'])==9 and not progress['new_failures']
assert len(progress['resolved_failures'])==15 and progress['inputs']==420 and progress['files']==1686
for p,h in progress['fixture_sha256'].items():assert sha(ROOT/p)==h
assert not git('diff','HEAD','--','dev')
plan=(ROOT/'pa18/plan.md').read_text();old=git('show',ENTRY+':pa18/plan.md').decode()
for label in ('Stage base commit','Last reviewed commit'):
 assert re.search(label+r': `([^`]+)`',plan)[1]==re.search(label+r': `([^`]+)`',old)[1]
subprocess.run(['python3',ROOT/'student.tests/pa18/reference83.py'],cwd=ROOT,check=True)
revisions=read(ROOT/'student.tests/pa18/reference83-revisions.json');assert len(revisions['revisions'])==17
course_diff=git('diff','--name-only',ENTRY,'--',*[f'pa{i}/tests' for i in range(1,19)]).decode().splitlines()
assert set(course_diff)=={r['path'] for r in revisions['revisions']}
controls={};total=0
for name,c in read(V/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=V/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','array83_controls','course81'):
 p=V/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
 if name=='array83_controls':controls[name]['results']=rows
assert controls['array83_controls']['count']==61 and total==1164+61
storage=read(V/'array83_controls/storage.json');assert len(storage)==7 and all(r['passed'] for r in storage)
pa16=read(V/'pa16-array-controls/results.json');assert all(r['passed'] for r in pa16['rows'])
reference=read(V/'reference-observations/results.json');assert len(reference)==46 and all(r['passed'] for r in reference)
inspection=read(V/'result81_inspection/results.json');assert len(inspection)==10 and all(r['native_exit']==0 for r in inspection)
perfpath=ROOT/'student.tests/pa18/loop83-performance.json';perf=read(perfpath)
assert perf.get('finished_utc') and len(perf['workloads'])==16
assert perf['commits'][0]==ENTRY
assert not git('diff',perf['commits'][1],'--','dev'),'Measured implementation differs from handoff.'
assert perf['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark83.py')
assert perf['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')==sha(W/'final-cppgm')==progress['compiler_sha256']
for b in perf['binaries']:assert sha(b['path'])==b['sha256']
observations=0
for item in [dict(compiler=perf['startup']),*perf['workloads'].values()]:
 for phase in ('compiler','runtime'):
  if phase not in item:continue
  m=item[phase];rows=m['observations'];order=[r['binary'] for r in rows]
  assert order in ([0]*4+[0,1,1,0]*4,[1]*6)
  assert all(r['checked_exit']==0 and r['wall_s']>0 for r in rows+m['warmups'])
  observations+=len(rows)+len(m['warmups'])
for name,item in perf['workloads'].items():
 assert hashlib.sha256(item['source'].encode()).hexdigest()==item['source_sha256']
 assert len(item['outputs'])==(1 if item['comparison']=='new-behavior' else 2)
 for output in item['outputs']:
  if 'native' in output:assert output['native']['checked_exit']==0
traces={}
for name in ('signature79','scalar80','result81','audit82','array83'):
 traces[name]=dict(source_sha256=sha(ROOT/'student.tests/pa18'/(name+'_trace.cpp')),lowir_sha256=sha(V/(name+'.lowir')),native_sha256=sha(V/(name+'.exe')))
paths=git('diff','--name-only',ENTRY,'--','dev').decode().splitlines()
result=dict(entry_commit=ENTRY,measured_commit=perf['commits'][1],implementation_sha256={p:sha(ROOT/p) for p in paths},compiler_sha256=sha(ROOT/'dev/cppgm++'),
 stage_base_commit=re.search(r'Stage base commit: `([^`]+)`',plan)[1],last_reviewed_commit=re.search(r'Last reviewed commit: `([^`]+)`',plan)[1],
 checks=checks,stage_progress=progress,controls=controls,personal_control_count=total,array_storage_checks=storage,pa16_array_controls=pa16,
 reference_observations=reference,reference_revisions=revisions,reference_proof_sha256=sha(ROOT/'pa18/reference-correction83.md'),
 entry_controls=read(W/'controls-entry-complete/results.json'),entry_timeout=dict(log=str(W/'controls-entry-final.log'),sha256=sha(W/'controls-entry-final.log'),case='empty_aggregate_template_scalar',seconds=30),
 performance=dict(path=str(perfpath),sha256=sha(perfpath),observations=observations),inspection=inspection,traces=traces,
 disposition='Validated array/constant-materialization implementation handoff; PA18 remains unfinished with nine course failures. Independent review is not waived.',
 unfinished_implementation=progress['final_failures'],review_questions=['Source bound completion versus retained signatures','Array query/frame identity','Static-array bound compatibility','Constant-array materialization proof boundaries'])
OUT.write_text(json.dumps(result,indent=2)+'\n');print('Verified handoff:',total,'controls; PA18 411/420, earlier 2609/2609; 17 proved oracle revisions;',observations,'performance observations.')

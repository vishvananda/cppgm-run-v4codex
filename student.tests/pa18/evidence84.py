#!/usr/bin/env python3
"""Bind completed PA18 constructor/value handoff checks and frozen observations."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();OUT=Path(sys.argv[2]);V=W/'validation'
ENTRY='09a77fca41efaa8eefc7913bc49898e7cd81e660'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
checks=read(V/'checks.json');progress=read(V/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for k,c in checks.items() if k!='stage')
for c in checks.values():assert sha(c['path'])==c['sha256']
assert '414 / 420' in (V/'stage.log').read_text() and '2609 / 2609' in (V/'prior.log').read_text()
assert 'File audit passed for pa18' in (V/'file-audit.log').read_text()
assert len(progress['entry_failures'])==9 and len(progress['final_failures'])==6 and not progress['new_failures']
assert len(progress['resolved_failures'])==3 and progress['inputs']==420 and progress['files']==1686
for p,h in progress['fixture_sha256'].items():assert sha(ROOT/p)==h
assert not git('diff','HEAD','--','dev')
plan=(ROOT/'pa18/plan.md').read_text();old=git('show',ENTRY+':pa18/plan.md').decode()
for label in ('Stage base commit','Last reviewed commit'):
 assert re.search(label+r': `([^`]+)`',plan)[1]==re.search(label+r': `([^`]+)`',old)[1]
subprocess.run(['python3',ROOT/'student.tests/pa18/reference84.py'],cwd=ROOT,check=True)
revisions=read(ROOT/'student.tests/pa18/reference84-revisions.json');assert len(revisions['revisions'])==3
course_diff=git('diff','--name-only',ENTRY,'--',*[f'pa{i}/tests' for i in range(1,19)]).decode().splitlines()
assert set(course_diff)=={r['path'] for r in revisions['revisions']}
controls={};total=0
for name,c in read(V/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=V/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','array83_controls','object84_controls','course81','course84'):
 p=V/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
 if name=='object84_controls':controls[name]['results']=rows
assert controls['object84_controls']['count']==51 and total==1225+51+3
inspection=read(V/'object84_controls/inspection.json');assert len(inspection)==21 and all(r['passed'] for r in inspection)
reference=read(V/'reference-observations/results.json');assert len(reference)==9 and all(r['passed'] for r in reference)
array_reference=read(V/'array-reference-observations/results.json');assert len(array_reference)==46 and all(r['passed'] for r in array_reference)
perfpath=ROOT/'student.tests/pa18/loop84-performance.json';perf=read(perfpath)
assert perf.get('finished_utc') and len(perf['workloads'])==13
initial_path=ROOT/'student.tests/pa18/loop84-performance-initial.json';initial=read(initial_path)
assert initial.get('finished_utc') and initial['binaries']==perf['binaries'] and initial['commits']==perf['commits']
assert initial['harness_sha256']==perf['harness_sha256'] and initial['flags']==perf['flags']
assert {k:v['source_sha256'] for k,v in initial['workloads'].items()}=={k:v['source_sha256'] for k,v in perf['workloads'].items()}
assert perf['commits'][0]==ENTRY and not git('diff',perf['commits'][1],'--','dev')
assert perf['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark84.py')
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
for name in ('signature79','scalar80','result81','audit82','array83','object84'):
 traces[name]=dict(source_sha256=sha(ROOT/'student.tests/pa18'/(name+'_trace.cpp')),lowir_sha256=sha(V/(name+'.lowir')),native_sha256=sha(V/(name+'.exe')))
paths=git('diff','--name-only',ENTRY,'--','dev').decode().splitlines()
result=dict(entry_commit=ENTRY,measured_commit=perf['commits'][1],implementation_sha256={p:sha(ROOT/p) for p in paths},compiler_sha256=sha(ROOT/'dev/cppgm++'),
 stage_base_commit=re.search(r'Stage base commit: `([^`]+)`',plan)[1],last_reviewed_commit=re.search(r'Last reviewed commit: `([^`]+)`',plan)[1],checks=checks,stage_progress=progress,
 controls=controls,personal_control_count=total,object_inspection=inspection,reference_observations=reference,reference_revisions=revisions,reference_proof_sha256=sha(ROOT/'pa18/reference-correction84.md'),
 entry_controls=read(W/'controls-entry-final/results.json'),performance=dict(path=str(perfpath),sha256=sha(perfpath),observations=observations,initial_path=str(initial_path),initial_sha256=sha(initial_path)),traces=traces,
 disposition='Validated constructor/empty-value implementation handoff; six PA18 failures remain. Whole-stage independent review is not waived.',
 unfinished_implementation=progress['final_failures']+['Inherited class_ellipsis_pending.cpp remains unsupported by scalar-only variadic LowIR'],
 review_questions=['Delegation-entry worklist completeness and base/complete/polymorphic propagation','Empty-transfer fast-path legality and shared source recipe validity','Local-type specialization root retention','Zero-init oracle proof and scalar conversion preservation'])
OUT.write_text(json.dumps(result,indent=2)+'\n');print('Verified handoff:',total,'controls; PA18 414/420, earlier 2609/2609; three proved oracle revisions;',observations,'performance observations.')

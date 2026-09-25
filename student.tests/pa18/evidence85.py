#!/usr/bin/env python3
"""Bind completed PA18 demand/discard handoff checks and frozen observations."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();OUT=Path(sys.argv[2]);V=W/'validation-shared'
ENTRY='f953e42a875bc8f7e3b9882a324db843f0c4191a'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
checks=read(V/'checks.json');progress=read(V/'stage-progress.json')
assert checks['stage']['exit']==2 and all(c['exit']==0 for k,c in checks.items() if k!='stage')
for c in checks.values():assert sha(c['path'])==c['sha256']
assert '417 / 420' in (V/'stage.log').read_text() and '2609 / 2609' in (V/'prior.log').read_text()
assert re.findall(r'^===== (pa\d+) =====$',(V/'prior.log').read_text(),re.M)==['pa'+str(i) for i in range(1,18)]
assert 'File audit passed for pa18' in (V/'file-audit.log').read_text()
assert len(progress['entry_failures'])==6 and len(progress['final_failures'])==3 and not progress['new_failures']
assert len(progress['resolved_failures'])==3 and progress['inputs']==420 and progress['files']==1686
for p,h in progress['fixture_sha256'].items():assert sha(ROOT/p)==h
assert not git('diff','HEAD','--','dev')
plan=(ROOT/'pa18/plan.md').read_text();old=git('show',ENTRY+':pa18/plan.md').decode()
for label in ('Stage base commit','Last reviewed commit'):
 assert re.search(label+r': `([^`]+)`',plan)[1]==re.search(label+r': `([^`]+)`',old)[1]
subprocess.run(['python3',ROOT/'student.tests/pa18/reference85.py'],cwd=ROOT,check=True)
revisions=read(ROOT/'student.tests/pa18/reference85-revisions.json');assert len(revisions['revisions'])==3
course_diff=git('diff','--name-only',ENTRY,'--',*[f'pa{i}/tests' for i in range(1,19)]).decode().splitlines()
assert set(course_diff)=={r['path'] for r in revisions['revisions']}
controls={};total=0
for name,c in read(V/'validate73_controls/results.json').items():
 if 'count' not in c:continue
 p=V/'validate73_controls'/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
for name in ('audit74_controls','list75_controls','inherit76_controls','nested77_controls','lookup77_controls','audit78_controls','audit78_declarations','syntax79_controls','signature79_controls','course79','scalar80_controls','result81_controls','audit82_controls','array83_controls','object84_controls','discard85_controls','storage85_controls','course81','course84'):
 p=V/name/'results.json';rows=read(p);assert all(r['passed'] for r in rows)
 controls[name]=dict(count=len(rows),sha256=sha(p),path=str(p));total+=len(rows)
 if name in ('discard85_controls','storage85_controls'):controls[name]['results']=rows
assert controls['discard85_controls']['count']==60 and controls['storage85_controls']['count']==16 and total==1279+76
inspection=read(V/'discard85_controls/inspections.json');assert len(inspection)==16 and all(r['passed'] for r in inspection)
reference=read(V/'reference-observations/observations.json');assert len(reference['rows'])==11
for row in reference['rows']:
 if row.get('label')=='student':assert row['exit']==0 and row['native_exit']==0
array_reference=read(V/'array-reference-observations/results.json');assert len(array_reference)==46 and all(r['passed'] for r in array_reference)
perfpath=ROOT/'student.tests/pa18/loop85-performance.json';perf=read(perfpath)
assert perf.get('finished_utc') and len(perf['workloads'])==14
initial_path=ROOT/'student.tests/pa18/loop85-performance-initial.json';initial=read(initial_path)
assert initial.get('finished_utc') and initial['flags']==perf['flags'] and initial['harness_sha256']==perf['harness_sha256']
assert {k:v['source_sha256'] for k,v in initial['workloads'].items()}=={k:v['source_sha256'] for k,v in perf['workloads'].items()}
for b in initial['binaries']:assert sha(b['path'])==b['sha256']
initial_observations=sum(len(m['observations'])+len(m['warmups']) for item in [dict(compiler=initial['startup']),*initial['workloads'].values()] for phase,m in item.items() if phase in ('compiler','runtime'))
sharing=read(V/'discard85_scaling/results.json');assert len(sharing)==15 and all(r['passed'] for r in sharing)
assert perf['commits'][0]==ENTRY and not git('diff',perf['commits'][1],'--','dev')
assert perf['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark85.py')
assert perf['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')==sha(W/'final-shared-cppgm')==progress['compiler_sha256']
for b in perf['binaries']:assert sha(b['path'])==b['sha256']
observations=0
for item in [dict(compiler=perf['startup']),*perf['workloads'].values()]:
 for phase in ('compiler','runtime'):
  if phase not in item:continue
  m=item[phase];rows=m['observations'];order=[r['binary'] for r in rows]
  assert order in ([0]*4+[0,1,1,0]*4,[1]*6)
  assert all(r['checked_exit']==0 and r['wall_s']>0 for r in rows+m['warmups'])
  observations+=len(rows)+len(m['warmups'])
for n in (150,600):
 stats={k:v for row in perf['workloads']['discard-class-'+str(n)]['outputs'][0]['telemetry'] for k,v in row.items()}
 assert stats['semantic_discard_selections']==1 and stats['semantic_discard_recipe_uses']==n and stats['semantic_discard_materializations']==n
for name,item in perf['workloads'].items():
 assert hashlib.sha256(item['source'].encode()).hexdigest()==item['source_sha256']
 assert len(item['outputs'])==(1 if item['comparison']=='new-behavior' else 2)
 assert item['comparison'] in ('exact','new-behavior')
 for output in item['outputs']:
  if 'native' in output:assert output['native']['checked_exit']==0
traces={}
for name in ('signature79','scalar80','result81','audit82','array83','object84','discard85'):
 traces[name]=dict(source_sha256=sha(ROOT/'student.tests/pa18'/(name+'_trace.cpp')),lowir_sha256=sha(V/(name+'.lowir')),native_sha256=sha(V/(name+'.exe')))
paths=git('diff','--name-only',ENTRY,'--','dev').decode().splitlines()
result=dict(entry_commit=ENTRY,measured_commit=perf['commits'][1],implementation_sha256={p:sha(ROOT/p) for p in paths},compiler_sha256=sha(ROOT/'dev/cppgm++'),
 stage_base_commit=re.search(r'Stage base commit: `([^`]+)`',plan)[1],last_reviewed_commit=re.search(r'Last reviewed commit: `([^`]+)`',plan)[1],checks=checks,stage_progress=progress,
 controls=controls,personal_control_count=total,discard_inspection=inspection,sharing_inspection=sharing,reference_observations=reference,reference_revisions=revisions,reference_proof_sha256=sha(ROOT/'pa18/reference-correction85.md'),
 entry_controls=read(W/'discard-entry-expanded/results.json'),entry_inspections=read(W/'discard-entry-expanded/inspections.json'),record_sizes=read(W/'sizes/sizes.json'),performance=dict(path=str(perfpath),sha256=sha(perfpath),observations=observations,initial_path=str(initial_path),initial_sha256=sha(initial_path),initial_observations=initial_observations),traces=traces,
 disposition='Validated demand/discard implementation handoff; three PA18 failures remain. Whole-stage independent review is not waived.',
 unfinished_implementation=progress['final_failures']+['Inherited class_ellipsis_pending.cpp remains unsupported by scalar-only variadic LowIR'],
 review_questions=['Built-in source-form fidelity across overload selection and fixed/dependent query substitution','Complete keys and shared volatile-copy recipes without unrelated body demand','Single temporary activation and full-expression normal/exceptional cleanup','Dormant-static and discarded-reference oracle proofs'])
OUT.write_text(json.dumps(result,indent=2)+'\n');print('Verified handoff:',total,'controls; PA18 417/420, earlier 2609/2609; three proved oracle revisions;',observations,'performance observations.')

#!/usr/bin/env python3
"""Verify the initialization handoff's code, unchanged coverage, revisions and evidence."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(*args):return subprocess.check_output(args,cwd=ROOT,text=True).strip()
def tree(paths):
 files=run('git','ls-files',*paths).splitlines();h=hashlib.sha256()
 for f in files:h.update(f.encode()+b'\0'+bytes.fromhex(sha(ROOT/f)))
 return len(files),h.hexdigest()
record=json.loads((ROOT/'student.tests/pa16/initialization-checkpoint.json').read_text())
assert tree(['dev/src','dev/frontend_source_sets.mk'])==tuple(record['implementation_tree'])
assert sha(ROOT/'dev/cppgm++')==record['compiler_sha256']
entry=record['entry_commit']
old_plan=run('git','show',entry+':pa16/plan.md');plan=(ROOT/'pa16/plan.md').read_text()
for label in ['Stage base commit','Last reviewed commit']:
 assert re.search(label+r': `([^`]+)`',old_plan)[1]==re.search(label+r': `([^`]+)`',plan)[1]
suites=[f'pa{n}/tests' for n in range(1,17)]
changes=set(run('git','diff','--name-only',entry,'--',*suites).splitlines())
revisions=json.loads((ROOT/'student.tests/pa16/initialization-reference-revisions.json').read_text())
assert len(revisions)==24 and changes=={r['path'] for r in revisions}
for r in revisions:
 old=subprocess.check_output(['git','show',entry+':'+r['path']],cwd=ROOT)
 assert hashlib.sha256(old).hexdigest()==r['before_sha256']
 assert sha(ROOT/r['path'])==r['after_sha256']
protected=['AGENTS.md','spec.md','TESTING_AND_REFERENCES.md','Makefile','dev/Makefile','scripts']
for n in range(1,17):protected += [f'pa{n}/Makefile',f'pa{n}/README.md',f'pa{n}/scripts',f'pa{n}/controls']
assert not run('git','diff','--name-only',entry,'--',*protected)
assert tree(suites)==tuple(record['course_tree'])
for a in record['artifacts']:
 assert sha(a['path'])==a['sha256'],a['path']
for p,h in record['harnesses'].items():assert sha(ROOT/p)==h,p
checks=record['checks'];prior=Path(checks['priorThroughTests']['log']).read_text()
assert checks['priorThroughTests']['exit_code']==0 and '2112 / 2112' in prior and 'ALL TESTS PASSED SUCCESSFULLY' in prior
stage=Path(checks['stageTests']['log']).read_text()
assert checks['stageTests']['exit_code']==2 and '153 / 154' in stage
names=re.findall(r'^(pa16/\S+\.t): ERROR:',stage,re.M)
assert names==record['remaining_course_failures'] and len(names)==1
assert checks['stageProgress']==dict(baseline_failures=8,current_failures=1,resolved_entry_failures=7,new_failures=0,coverage_preserved=True,passed=True)
assert checks['fileAudit']['exit_code']==0 and 'File audit passed for pa16' in Path(checks['fileAudit']['log']).read_text()
controls=json.loads(Path(record['controls']).read_text())
assert all(r['passed'] for r in controls['rows'])
assert sum(r.get('native_exit')==0 for r in controls['rows'])==32
assert sum(r.get('expected_reference_failure',False) for r in controls['rows'])==7
assert sum(r['compile_exit']!=0 for r in controls['rows'])==2
pending=json.loads(Path(record['pending_evidence']).read_text())
assert pending['required_exit']==0 and pending['observed_exit']!=0 and pending['status']=='unfinished implementation'
assert pending['compiler_sha256']==record['compiler_sha256']
assert sha(ROOT/'student.tests/pa16/initialization-performance.json')==record['performance_sha256']
perf=json.loads((ROOT/'student.tests/pa16/initialization-performance.json').read_text())
assert perf['flags']==['--emit-lowir','-O0']
assert perf['harness_sha256']==sha(ROOT/'student.tests/pa16/initialization_benchmark.py')
assert perf['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert all(sha(b['path'])==b['sha256'] for b in perf['binaries'])
assert perf['binaries'][1]['sha256']==record['compiler_sha256']
count=0
for name,w in perf['workloads'].items():
 assert sha(w['source_path'])==w['source_sha256']
 for o in w['outputs']:
  assert sha(o['path'])==o['sha256']
  if 'native' in o:assert sha(o['native']['path'])==o['native']['sha256'] and o['native']['checked_exit']==0
 if w['comparison']=='exact':
  assert len({o['sha256'] for o in w['outputs']})==1
  if 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
 for phase in ['compiler','runtime']:
  if phase not in w:continue
  m=w[phase];assert [r['binary'] for r in m['warmups']]==[0,1]
  assert [r['binary'] for r in m['observations']]==[0,0,0,0,0,1,1,0,0,1,1,0]
  assert all(r['wall_s']>0 and r['checked_exit']==0 for r in m['observations']+m['warmups'])
  count+=len(m['observations'])+len(m['warmups'])
assert count==364
for path,expected in record['performance_artifacts'].items():assert sha(ROOT/path)==expected
initial=json.loads((ROOT/'student.tests/pa16/initialization-performance-initial.json').read_text())
for b in initial['binaries']:assert sha(b['path'])==b['sha256']
for name,w in initial['workloads'].items():
 assert w['outputs'][1]['sha256']==perf['workloads'][name]['outputs'][1]['sha256']
 for o in w['outputs']:
  assert sha(o['path'])==o['sha256']
  if 'native' in o:assert sha(o['native']['path'])==o['native']['sha256']
 for phase in ['compiler','runtime']:
  if phase not in w:continue
  m=w[phase];assert [r['binary'] for r in m['observations']]==[0,0,0,0,0,1,1,0,0,1,1,0]
  count+=len(m['warmups'])+len(m['observations'])
noise=json.loads((ROOT/'student.tests/pa16/initialization-noise.json').read_text())
assert noise['campaign_sha256']==record['performance_sha256']
assert noise['harness_sha256']==sha(ROOT/'student.tests/pa16/initialization_noise.py')
for w in noise['workloads'].values():
 assert sha(w['source_path'])==w['source_sha256']
 assert [r['binary'] for r in w['observations']]==[0]*4+[0,1,1,0]*4
 for batch in w['warmups']+w['observations']:
  assert len(batch['invocations'])==w['repeats']
  assert all(i['checked_exit']==0 and i['wall_s']>0 for i in batch['invocations'])
  count+=len(batch['invocations'])
lifecycle=json.loads((ROOT/'student.tests/pa16/lifecycle-performance.json').read_text())
assert lifecycle['binaries'][1]['sha256']==record['compiler_sha256']
assert lifecycle['harness_sha256']==sha(ROOT/'student.tests/pa16/lifecycle_benchmark.py')
for n,w in lifecycle['workloads'].items():
 assert w['entry_exit']!=0 and 'duplicate singleton role' in w['entry_diagnostic']
 assert all(sha(src['path'])==src['sha256'] for src in w['sources'])
 assert sha(w['output']['path'])==w['output']['sha256']
 assert sha(w['native']['path'])==w['native']['sha256'] and w['native']['checked_exit']==0
 assert w['output']['telemetry']['initializer_units']==int(n)
 for phase in ['compiler','runtime']:
  m=w[phase];assert len(m['warmups'])==1 and len(m['observations'])==6
  assert all(i['checked_exit']==0 and i['wall_s']>0 for i in m['warmups']+m['observations'])
  count+=7
assert count==1482
print('PA16 initialization handoff verified: 153/154; prior 2112/2112; 7 fewer failures; unchanged coverage; 24 proved revisions; 1482 observations. Incomplete implementation remains recorded.')

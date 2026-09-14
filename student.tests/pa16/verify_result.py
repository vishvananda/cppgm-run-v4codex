#!/usr/bin/env python3
"""Verify the PA16 implementation-43 handoff without changing any fixtures."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(*args):return subprocess.check_output(args,cwd=ROOT,text=True).strip()
def tree(paths):
 files=run('git','ls-files',*paths).splitlines();h=hashlib.sha256()
 for f in files:h.update(f.encode()+b'\0'+bytes.fromhex(sha(ROOT/f)))
 return [len(files),h.hexdigest()]
record=json.loads((ROOT/'student.tests/pa16/result-checkpoint.json').read_text())
assert tree(['dev/src','dev/frontend_source_sets.mk'])==record['implementation_tree']
assert sha(ROOT/'dev/cppgm++')==record['compiler_sha256']
entry=record['entry_commit'];plan=(ROOT/'pa16/plan.md').read_text();old=run('git','show',entry+':pa16/plan.md')
for label in ['Stage base commit','Last reviewed commit']:
 assert re.search(label+r': `([^`]+)`',plan)[1]==re.search(label+r': `([^`]+)`',old)[1]
suites=[f'pa{n}/tests' for n in range(1,17)]
assert run('git','ls-files',*suites)==run('git','ls-tree','-r','--name-only',entry,'--',*suites)
revision=json.loads((ROOT/'student.tests/pa16/result-reference-revision.json').read_text())
assert run('git','diff','--name-only',entry,'--',*suites)==revision['path']
assert sha(ROOT/revision['path'])==revision['new_sha256']
assert hashlib.sha256(subprocess.check_output(['git','show',entry+':'+revision['path']],cwd=ROOT)).hexdigest()==revision['old_sha256']
assert sha((ROOT/revision['path']).with_suffix('.t'))==revision['source_sha256']
for file,digest in record['artifacts'].items():assert sha(ROOT/file)==digest,file
for check in record['checks']:
 assert check['exit_code']==0,check
 assert sha(check['log'])==check['sha256'],check['name']
 assert check['success'] in Path(check['log']).read_text(),check['name']
for suite in record['controls']:
 assert suite['exit_code']==0,suite['name']
 assert sha(suite['log'])==suite['sha256'],suite['name']
for suite in record['new_control_results']:
 assert suite['compiler_sha256']==record['compiler_sha256']
 assert all(r.get('passed',r.get('native_exit')==0) for r in suite['rows'])
perf=json.loads((ROOT/'student.tests/pa16/result-performance.json').read_text())
assert perf['finished_utc'] and perf['binaries'][1]['sha256']==record['compiler_sha256']
assert perf['continuation_harness_sha256']==sha(ROOT/'student.tests/pa16/result_benchmark.py')
assert hashlib.sha256(perf['initial_producer_source'].encode()).hexdigest()==perf['harness_sha256']
for p in perf['continuation_producers']:assert hashlib.sha256(p['source'].encode()).hexdigest()==p['sha256']
count=0
for name,w in perf['workloads'].items():
 assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256'],name
 for phase in ['compiler','runtime']:
  if phase not in w:continue
  x=w[phase];rows=x['observations'];count+=len(rows)+len(x['warmups'])
  assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>=0 for r in rows+x['warmups'])
  if w['comparison']=='entry-incorrect':
   assert {r['binary'] for r in rows}=={1} and w['entry_behavior']['compile_exit']!=0
  else:
   assert [r['binary'] for r in rows[:4]]==[0]*4
   assert [r['binary'] for r in rows[4:]]==[0,1,1,0]*((len(rows)-4)//4)
 if w['comparison']=='exact':
  assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256']
  if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256']
 for output in w['outputs']:
  assert sha(output['path'])==output['sha256']
  if 'native' in output:assert output['native']['checked_exit']==0 and sha(output['native']['path'])==output['native']['sha256']
assert count==record['performance_observations']==302
print('PA16 implementation-43 handoff verified: 2266/2266 course; 246 native/83 rejection controls; 302 performance observations; preserved review markers and coverage.')

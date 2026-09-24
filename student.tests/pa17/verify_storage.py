#!/usr/bin/env python3
"""Verify the loop 59 implementation boundary; this does not replace stage audit."""
from pathlib import Path
import hashlib,json,re,subprocess
from verify_checkpoint56 import measurement
from storage_reference_corrections import corrected
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def digest():
 h=hashlib.sha256()
 for name in sorted(git('ls-files','dev').splitlines()):
  h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
 return h.hexdigest()
def failures(s):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',s,re.M))
def verify():
 e=json.loads((ROOT/'student.tests/pa17/storage-evidence.json').read_text())
 assert digest()==e['source_digest'] and not git('diff',e['code_commit'],'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',e['code_commit'],'HEAD'],cwd=ROOT)
 plan=(ROOT/'pa17/plan.md').read_text()
 for label,key in [('Stage base commit','stage_base'),('Last reviewed commit','last_reviewed')]:
  assert f'{label}: `{e[key]}`' in plan
 assert 'unfinished implementation' in plan and 'Independent review remains open' in plan
 logs={}
 for name,row in e['logs'].items():
  assert sha(row['path'])==row['sha256'];logs[name]=Path(row['path']).read_text()
 entry,final=failures(logs['entry']),failures(logs['stage'])
 assert len(entry)==10 and len(final)==3 and final<entry
 assert sorted(entry-final)==e['closed_failures'] and sorted(final)==e['final_failures']
 assert 'TEST SUMMARY: 340 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior'] and not failures(logs['prior']) and e['logs']['prior']['exit_code']==0
 assert 'TEST SUMMARY: 2606 / 2609 TESTS PASSED' in logs['through'] and failures(logs['through'])==final
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file-audit'] and e['logs']['file-audit']['exit_code']==0
 assert 'PASS' in logs['stage-progress'] and e['logs']['stage-progress']['exit_code']==0
 assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))==e['course_tests']
 assert len(e['course_tests'])==343 and not git('diff',e['entry_commit'],'--',*e['course_tests'])
 assert not git('diff',e['entry_commit'],'--',*e['protected_paths'])
 expected=corrected()
 paths=sorted('pa17/tests/'+name+'.ref' for name in expected)
 assert git('diff','--name-only',e['entry_commit'],'--',':(glob)pa*/tests/**').splitlines()==paths
 assert paths==e['reference_corrections']
 for name,text in expected.items():assert (ROOT/'pa17/tests'/(name+'.ref')).read_text()==text
 observations=json.loads((ROOT/'student.tests/pa17/storage-reference-observations.json').read_text())
 assert observations['compiler_sha256']==sha(ROOT/'reference-binaries/cppgm++')
 reducers={r['name']:r for r in observations['reducers']}
 for r in reducers.values():
  assert r['compiler_exit']==r['backend_exit']==0
  assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
  assert hashlib.sha256(r['lowir'].encode()).hexdigest()==r['lowir_sha256']
 assert reducers['reference']['native_exit']==reducers['unused_member']['native_exit']==1
 assert 'role=init' in reducers['constexpr_function_address']['lowir']
 assert '__guard' in reducers['local_function_address']['lowir']
 assert 'zeroinit' not in reducers['empty_value']['lowir']
 controls=json.loads((ROOT/'student.tests/pa17/storage-controls.json').read_text())
 old=json.loads((ROOT/'student.tests/pa17/query-controls.json').read_text())
 assert set(controls)==set(old)|{'storage'} and sum(map(len,controls.values()))==567
 for name,rows in controls.items():
  if name!='storage':assert [(r['name'],r['source_sha256']) for r in rows]==[(r['name'],r['source_sha256']) for r in old[name]]
  for r in rows:
   assert r['passed'] and hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
   if r.get('expected')=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
   if r.get('expected')=='reject':assert r['compiler_exit']!=0
 before=json.loads((ROOT/'student.tests/pa17/storage-entry-controls.json').read_text())
 assert len(before)==37 and sum(not r['passed'] for r in before)==e['entry_control_failures']
 assert [(r['name'],r['source_sha256']) for r in before]==[(r['name'],r['source_sha256']) for r in controls['storage']]
 p=json.loads((ROOT/'student.tests/pa17/storage-performance.json').read_text())
 assert p['finished_utc'] and not p['source_diff'] and p['source_commit']==e['campaign_commit']
 assert hashlib.sha256(subprocess.check_output(['git','show',p['source_commit']+':student.tests/pa17/storage_benchmark.py'],cwd=ROOT)).hexdigest()==p['harness_sha256']
 assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
 assert p['flags']==['--emit-lowir','-O0'] and len(p['workloads'])==14
 for row in p['binaries']+[p['backend']]:assert sha(row['path'])==row['sha256']
 assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
 for w in p['workloads'].values():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  common=w['comparison']!='entry-rejected'
  if not common:assert w['entry_rejection']['exit_code']!=0
  assert len(w['outputs'])==(2 if common else 1)
  for row in w['outputs']:
   assert sha(row['path'])==row['sha256']
   stats={k:v for block in row['telemetry'] for k,v in block.items()};assert stats['semantic_entity_bytes']==120
   if 'native' in row:
    n=row['native'];assert n['checked_exit']==0 and sha(n['path'])==n['sha256']
    assert n['code_and_alignment_bytes']+n['global_data_bytes']==n['executable_payload_bytes']
  if w['comparison']=='exact':
   assert len({r['sha256'] for r in w['outputs']})==1
   if 'native' in w['outputs'][0]:assert len({r['native']['sha256'] for r in w['outputs']})==1
  for phase in ('compiler','runtime'):
   if phase in w:measurement(w[phase],common)
 for row in e['evidence_files']:assert sha(ROOT/row['path'])==row['sha256']
 for key in ('source','lowir','native','telemetry'):assert sha(e['trace'][key]['path'])==e['trace'][key]['sha256']
 assert e['trace']['native_exit']==0 and not e['waivers']
 assert {f for group in e['remaining_groups'].values() for f in group['failures']}==final
 print('PA17 storage handoff verified: 340/343, seven original failures closed; prior 2266/2266; 567 controls; six proved reference corrections; file audit and frozen performance pass. Three implementation cases and independent review remain.')
if __name__=='__main__':verify()

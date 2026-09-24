#!/usr/bin/env python3
"""Verify the PA17 loop 57 implementation handoff, without certifying PA17."""
from pathlib import Path
import hashlib,json,re,subprocess
from verify_checkpoint56 import measurement
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def fails(s):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',s,re.M))
def digest():
 h=hashlib.sha256()
 for p in sorted(git('ls-files','dev').splitlines()):h.update(p.encode()+b'\0');h.update((ROOT/p).read_bytes());h.update(b'\0')
 return h.hexdigest()
def performance(path,finished):
 p=json.loads(path.read_text());assert bool(p.get('finished_utc'))==finished and not p['source_diff']
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 if finished:assert p['harness_sha256']==sha(ROOT/'student.tests/pa17/transfer_benchmark.py')
 for w in p['workloads'].values():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  for o in w['outputs']:
   assert sha(o['path'])==o['sha256']
   if 'native' in o:
    n=o['native'];assert sha(n['path'])==n['sha256'] and n['checked_exit']==0
    assert n['code_and_alignment_bytes']+n['global_data_bytes']==n['executable_payload_bytes']
  if w['comparison']=='exact':
   assert len({o['sha256'] for o in w['outputs']})==1
   if 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
  if finished:assert 'compiler' in w
  for phase in ['compiler','runtime']:
   if phase in w:measurement(w[phase],True)
 return p

def verify():
 e=json.loads((ROOT/'student.tests/pa17/transfer-evidence.json').read_text());tip=e['code_commit']
 assert digest()==e['source_digest'] and not git('diff',tip,'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
 assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
 plan=(ROOT/'pa17/plan.md').read_text()
 assert f'Stage base commit: `{e["stage_base"]}`' in plan and f'Last reviewed commit: `{e["last_reviewed"]}`' in plan
 logs={}
 for k,v in e['logs'].items():assert sha(v['path'])==v['sha256'];logs[k]=Path(v['path']).read_text()
 entry,final=fails(logs['entry']),fails(logs['stage'])
 assert len(entry)==19 and len(final)==13 and final<entry
 assert sorted(entry-final)==e['closed_failures'] and sorted(final)==e['final_failures']
 assert 'TEST SUMMARY: 330 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert not fails(logs['prior']) and 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior'] and e['logs']['prior']['exit_code']==0
 assert fails(logs['through'])==final and 'TEST SUMMARY: 2596 / 2609 TESTS PASSED' in logs['through']
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file-audit'] and e['logs']['file-audit']['exit_code']==0
 assert 'PASS' in logs['stage-progress'] and e['logs']['stage-progress']['exit_code']==0
 course=sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))
 assert course==e['course_tests'] and len(course)==343
 grouped=[p for g in e['remaining_groups'].values() for p in g['failures']]
 assert len(grouped)==len(set(grouped)) and set(grouped)==final
 controls=json.loads((ROOT/'student.tests/pa17/transfer-controls.json').read_text())
 old=json.loads((ROOT/'student.tests/pa17/checkpoint56-controls.json').read_text())
 assert set(controls)==set(old)|{'transfer'} and sum(map(len,controls.values()))==493
 for name,rows in controls.items():
  if name!='transfer':assert [(r['name'],r['source_sha256']) for r in rows]==[(r['name'],r['source_sha256']) for r in old[name]]
  for r in rows:
   assert r['passed'] and hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
   if r.get('expected')=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
   if r.get('expected')=='reject':assert r['compiler_exit']!=0
 before=json.loads((ROOT/'student.tests/pa17/transfer-entry-controls.json').read_text())
 assert len(before)==28 and sum(not r['passed'] for r in before)==2
 assert [(r['name'],r['source_sha256']) for r in before]==[(r['name'],r['source_sha256']) for r in controls['transfer']]
 p=performance(ROOT/'student.tests/pa17/transfer-performance.json',True)
 assert p['source_commit']==tip and p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
 assert len(p['workloads'])==18
 performance(ROOT/'student.tests/pa17/transfer-performance-before-fixed-receiver.json',False)
 for f in e['evidence_files']:assert sha(ROOT/f['path'])==f['sha256']
 for k in ['source','lowir','native']:assert sha(e['trace'][k]['path'])==e['trace'][k]['sha256']
 assert e['trace']['native_exit']==0 and e['trace']['telemetry']
 assert not e['reference_corrections'] and not e['waivers']
 print('PA17 handoff verified: 330/343; six original failures closed; earlier 2266/2266; 493 personal controls; file audit and frozen performance evidence. Independent review and 13 implementation failures remain.')
if __name__=='__main__':verify()

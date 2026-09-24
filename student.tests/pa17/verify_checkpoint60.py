#!/usr/bin/env python3
"""Verify scope, preserved coverage, frozen evidence and review markers."""
from pathlib import Path
import hashlib,json,subprocess
from record_checkpoint60 import ROOT,git,sha,patch,digest,failures
from verify_checkpoint56 import measurement
from storage_reference_corrections import corrected
def verify():
 e=json.loads((ROOT/'student.tests/pa17/checkpoint60-evidence.json').read_text());tip=e['code_commit']
 assert digest()==e['source_digest'] and not git('diff',tip,'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
 for name in ('plan.md','audit.md'):
  s=(ROOT/'pa17'/name).read_text()
  assert f'Last reviewed commit: `{tip}`' in s and f'Stage base commit: `{e["stage_base"]}`' in s
 r=json.loads((ROOT/'student.tests/pa17/checkpoint60-range.json').read_text())
 assert r['reviewed_tip']==tip and r['review_start']==e['review_start']
 assert [c['commit'] for c in r['commits']]==git('rev-list','--reverse',e['review_start']+'..'+tip).splitlines()
 assert r['combined_implementation_paths']==git('diff','--name-only',e['review_start'],tip,'--','dev').splitlines()
 assert r['combined_patch_sha256']==patch('diff',e['review_start'],tip,'--','dev')
 for c in r['commits']:assert c['implementation_patch_sha256']==patch('show','--format=',c['commit'],'--','dev')
 for f in e['evidence_files']+e['historical_evidence']:assert sha(ROOT/f['path'])==f['sha256']
 logs={}
 for name,row in e['logs'].items():
  assert sha(row['path'])==row['sha256'];logs[name]=Path(row['path']).read_text()
 entry,stage=failures(e['logs']['entry']['path']),failures(e['logs']['stage']['path'])
 assert entry==stage==e['entry_failures']==e['final_failures'] and len(stage)==3
 assert failures(e['logs']['baseline']['path'])==stage and failures(e['logs']['through']['path'])==stage
 assert 'TEST SUMMARY: 340 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert 'TEST SUMMARY: 2606 / 2609 TESTS PASSED' in logs['through'] and e['logs']['through']['exit_code']==2
 assert 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior'] and not failures(e['logs']['prior']['path']) and e['logs']['prior']['exit_code']==0
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file-audit'] and e['logs']['file-audit']['exit_code']==0
 assert 'PASS' in logs['stage-progress'] and e['logs']['stage-progress']['exit_code']==0
 assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))==e['course_tests'] and len(e['course_tests'])==343
 assert not git('diff',e['review_start'],'--',*e['protected_paths'])
 edits=corrected();paths=sorted('pa17/tests/'+name+'.ref' for name in edits)
 assert paths==e['reference_corrections']==git('diff','--name-only',e['review_start'],'--',':(glob)pa*/tests/**').splitlines()
 assert not git('diff',e['entry_commit'],'--',':(glob)pa*/tests/**')
 for name,text in edits.items():assert (ROOT/'pa17/tests'/(name+'.ref')).read_text()==text
 inherited=json.loads((ROOT/'student.tests/pa17/storage-controls.json').read_text())
 controls=json.loads((ROOT/'student.tests/pa17/checkpoint60-controls.json').read_text())
 assert set(controls)==set(inherited)|{'checkpoint60'} and sum(map(len,controls.values()))==e['control_count']==610
 for group,rows in controls.items():
  if group in inherited:assert [(r['name'],r['source_sha256'],r['expected']) for r in rows]==[(r['name'],r['source_sha256'],r['expected']) for r in inherited[group]]
  for row in rows:
   assert row['passed'] and hashlib.sha256(row['source'].encode()).hexdigest()==row['source_sha256']
   if row['expected']=='native':assert row['compiler_exit']==row['backend_exit']==row['native_exit']==0
   if row['expected']=='reject':assert row['compiler_exit']>0
 entry=json.loads((ROOT/'student.tests/pa17/checkpoint60-entry-controls.json').read_text())
 assert len(entry)==e['new_control_count']==43 and sum(not r['passed'] for r in entry)==e['entry_control_failures']
 assert [(r['name'],r['source_sha256']) for r in entry]==[(r['name'],r['source_sha256']) for r in controls['checkpoint60']]
 for filename in e['performance']:
  p=json.loads((ROOT/filename).read_text());assert p['finished_utc'] and not p['source_diff'] and p['source_commit']==tip
  assert p['harness_sha256']==sha(ROOT/'student.tests/pa17/checkpoint60_benchmark.py') and p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
  for b in p['binaries']+[p['backend']]:assert sha(b['path'])==b['sha256']
  assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++') and p['flags']==['--emit-lowir','-O0']
  for w in p['workloads'].values():
   assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
   common=len(w['outputs'])==2
   if not common:assert w.get('entry_rejection',{}).get('exit_code',0)!=0 or w['entry_incorrect']['native_exit']!=0
   for o in w['outputs']:
    assert sha(o['path'])==o['sha256']
    if 'native' in o:
     n=o['native'];assert n['checked_exit']==0 and sha(n['path'])==n['sha256']
     assert n['code_and_alignment_bytes']+n['global_data_bytes']==n['executable_payload_bytes']
   if w['comparison']=='exact':
    assert len({o['sha256'] for o in w['outputs']})==1
    if 'native' in w['outputs'][0]:assert len({o['native']['sha256'] for o in w['outputs']})==1
   for phase in ('compiler','runtime'):
    if phase in w:measurement(w[phase],common)
 t=json.loads((ROOT/'student.tests/pa17/checkpoint60-trace.json').read_text())
 assert t['code_commit']==tip and t['native_exit']==0 and t['telemetry'] and t['disassembly']
 for key in ('source','lowir','native','compiler'):assert sha(t[key]['path'])==t[key]['sha256']
 assert e['record_sizes']=={'entry':'120 48','final':'120 48'}
 assert sorted(p for group in e['remaining_groups'].values() for p in group)==stage and not e['waivers']
 print('PA17 loop-60 audit verified: full accumulated range; 340/343, same three failures; prior 2266/2266; 610 controls; six proved prior reference corrections; file audit, trace and frozen performance evidence pass.')
if __name__=='__main__':verify()

#!/usr/bin/env python3
"""Verify current PA16 storage handoff and preserve the historical scalar evidence."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def checked(item):
 p=Path(item['path']);p=p if p.is_absolute() else ROOT/p
 assert sha(p)==item['sha256'],str(p)
 return p
r=json.loads((ROOT/'student.tests/pa16/storage-handoff.json').read_text())
for p in r['source_files']:checked(p)
assert sha(ROOT/'dev/cppgm++')==r['compiler_sha256']
subprocess.run(['git','diff','--exit-code',r['stage_base'],'--','pa16/tests','pa16/scripts','pa16/Makefile','pa16/README.md','pa16/pa16.gram','spec.md','TESTING_AND_REFERENCES.md','scripts/compare_results_common.pl'],cwd=ROOT,check=True)
assert len(list((ROOT/'pa16/tests').rglob('*.t')))==154
logs={n:checked(x) for n,x in r['logs'].items()}
f=lambda p:set(re.findall(r'(pa16/tests/[^:]+): ERROR:',p.read_text()))
before,after=f(logs['baseline']),f(logs['stage'])
assert len(before)==91 and len(after)==76 and not after-before
assert sorted(before-after)==r['fixed_tests']
assert 'TEST SUMMARY: 78 / 154 TESTS PASSED' in logs['stage'].read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)' in logs['prior'].read_text()
assert 'File audit passed for pa16' in logs['audit'].read_text()
assert '27 native storage controls passed' in logs['storage'].read_text()
assert '21 native and 10 rejection controls passed' in logs['scalar'].read_text()
assert '19 native and 13 rejection controls passed' in logs['floating'].read_text()
assert '17 native and 11 rejection final-audit controls passed' in logs['prior_audit'].read_text()
for e in r['retained_evidence']:checked(e)
observations=0
names={f'{g}-{n}' for g in ['template','memory-float','array-sharing','array-copy','static'] for n in [1000,4000]}|{f'runtime-{g}' for g in ['calls','memory','floating','sharing','copy','static']}
for campaign in r['campaigns']:
 d=json.loads(checked(campaign).read_text());assert d['finished_utc'] and set(d['workloads'])==names
 for b in d['binaries']:checked(b)
 checked(d['backend'])
 assert sha(ROOT/'student.tests/pa16/storage_benchmark.py')==d['harness_sha256']
 for name,w in d['workloads'].items():
  assert sha(w['source_path'])==w['source_sha256']
  for o in w['outputs']:
   checked(o)
   if 'native' in o:
    checked(o['native']);assert o['native']['checked_exit']==0
    assert o['native']['code_and_alignment_bytes']+o['native']['global_data_bytes']==o['native']['executable_payload_bytes']
  if w['comparison']=='exact':
   assert len({o['sha256'] for o in w['outputs']})==1
   if 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
  if w['comparison']=='entry-incorrect' and 'runtime' in w:assert w['entry_behavior']['exit_code']!=0
  for phase in ['compiler','runtime']:
   if phase not in w:continue
   a=w[phase];common=w['comparison']!='entry-incorrect'
   assert [o['binary'] for o in a['observations']]==([0,0,0,0,0,1,1,0,0,1,1,0] if common else [1]*6)
   assert [o['binary'] for o in a['warmups']]==([0,1] if common else [1])
   for o in a['warmups']+a['observations']:
    assert o['checked_exit']==0 and o['wall_s']>0 and o['rss_kib']>0
    observations+=1
 # Instrumented ownership scales with emitted items, not unrelated declarations.
 for n in [1000,4000]:
  t=d['workloads'][f'array-sharing-{n}']['outputs'][1]['telemetry'][0]
  assert t['lower_constant_data_records']==1 and t['lower_constant_data_hits']==2*n-1
  assert t['lower_constant_data_work']==28*n-7
  t=d['workloads'][f'array-copy-{n}']['outputs'][1]['telemetry'][0]
  assert t['lower_constant_data_records']==1 and t['lower_constant_data_work']==8*n-4
  assert t['semantic_static_plan_work']==n
  t=d['workloads'][f'static-{n}']['outputs'][0]['telemetry'][0]
  assert t['lower_local_statics']==n and t['semantic_static_initialization_work']==n
noise=json.loads(checked(r['noise']).read_text())
assert noise['finished_utc'] and noise['campaign_sha256']==sha(ROOT/'student.tests/pa16/storage-performance.json')
assert noise['harness_sha256']==sha(ROOT/'student.tests/pa16/storage_noise.py')
noise_observations=0
for w in noise['workloads'].values():
 assert sha(w['source_path'])==w['source_sha256']
 assert [x['binary'] for x in w['observations']]==[0]*4+[0,1,1,0]*4
 assert [x['binary'] for x in w['warmups']]==[0,1]
 for row in w['warmups']+w['observations']:
  assert len(row['invocations'])==w['repeats']
  for x in row['invocations']:
   assert x['checked_exit']==0 and x['wall_s']>0 and x['rss_kib']>0
   noise_observations+=1
assert noise_observations==726
# The original manifest is historical: its source hashes name the loop-37 tree,
# not current files. Check that tree and every retained historical artifact.
h=json.loads(checked(r['historical_manifest']).read_text())
for x in h['source_files']:
 data=subprocess.check_output(['git','show',r['entry_commit']+':'+x['path']],cwd=ROOT)
 assert hashlib.sha256(data).hexdigest()==x['sha256'],x['path']
for x in h['evidence_files']+list(h['logs'].values()):checked(x)
for c in h['campaigns']:
 d=json.loads(checked(c).read_text())
 for b in d['binaries']:checked(b)
 checked(d['backend'])
 for w in d['workloads'].values():
  assert sha(w['source_path'])==w['source_sha256']
  for o in w['outputs']:
   checked(o)
   if 'native' in o:checked(o['native'])
p=(ROOT/'pa16/plan.md').read_text()
assert f"Stage base commit: `{r['stage_base']}`" in p
assert f"Last reviewed commit: `{r['last_reviewed_commit']}`" in p
assert 'Remaining implementation' in p and 'independent review' in p.lower()
print(f'PASS: 15 entry failures fixed; 154 unchanged fixtures; prior 2112/2112; file audit; '
      f'27 native storage controls; 40 native/23 rejection inherited PA16 controls; '
      f'{len(r["campaigns"])} storage campaigns / {observations} observations + {noise_observations} noise observations; historical scalar evidence preserved')

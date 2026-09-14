#!/usr/bin/env python3
"""Verify the frozen loop-33 handoff; --live additionally checks current code."""
from pathlib import Path
import argparse, hashlib, json, re, subprocess
ROOT=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser();p.add_argument('--live',action='store_true');args=p.parse_args()
e=json.loads((ROOT/'student.tests/pa15/matching-handoff.json').read_text())
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
for item in e['artifacts']:
 assert sha(item['path'])==item['sha256'],item['path']
for name,tree in e['fixtures'].items():
 assert git('rev-parse',e['entry_commit']+':'+name)==tree,name
 assert git('rev-parse',e['implementation_commit']+':'+name)==tree,name
 if args.live:assert git('rev-parse','HEAD:'+name)==tree,name
entry=Path(e['checks']['entry']['log']).read_text();stage=Path(e['checks']['stage']['log']).read_text()
def failures(log):return sorted(set(re.findall(r'(pa15/\S+\.t): ERROR:',log)))
assert failures(entry)==e['entry_failures'] and failures(stage)==e['remaining_failures']
assert len(e['entry_failures'])==11 and len(e['remaining_failures'])==8
assert set(e['remaining_failures']) < set(e['entry_failures'])
assert '166 / 177' in entry and '169 / 177' in stage
assert '1935 / 1935' in Path(e['checks']['prior']['log']).read_text()
assert 'File audit passed for pa15' in Path(e['checks']['file_audit']['log']).read_text()
assert e['checks']['stage']['exit']==2 and e['checks']['prior']['exit']==0 and e['checks']['file_audit']['exit']==0
observations=0
for campaign in e['campaigns']:
 data=json.loads(Path(campaign).read_text())
 assert data['source_diff']==''
 for binary in data['binaries']:assert sha(binary['path'])==binary['sha256']
 assert sha(data['backend']['path'])==data['backend']['sha256']
 assert data['harness_sha256'] in [x['sha256'] for x in e['artifacts']]
 for name,w in data['workloads'].items():
  assert sha(w['source_path'])==w['source_sha256']
  for out in w['outputs']:
   assert sha(out['path'])==out['sha256']
   if 'native' in out:assert sha(out['native']['path'])==out['native']['sha256']
  if w['common_correct']:
   assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256'],name
   if 'native' in w['outputs'][0]:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256'],name
  else:assert w['entry_probe']['exit']!=0
  for kind in ['compiler','runtime']:
   if kind not in w:continue
   m=w[kind];rows=m['observations'];order=[r['binary'] for r in rows]
   assert order==([0]*4+[0,1,1,0]*2 if w['common_correct'] else [1]*6),(name,kind)
   for r in rows+m['warmups']:assert r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0
   observations+=len(rows)+len(m['warmups'])
 if data['profile']=='matching-reuse':
  for n in [1000,4000]:
   w=data['workloads'][f'ordered-patterns-{n}']
   sem=w['outputs'][1]['telemetry'][0]
   assert sem['class_pattern_ordering_work']==2
   assert sem['class_pattern_ordering_hits']==2*n-2
   assert sem['semantic_candidate_work']==2*n
   assert sem['template_body_transitions']==0
assert observations==e['measured_invocations'],observations
if args.live:
 assert sha(ROOT/'dev/cppgm++')==e['binary_sha256']
 assert git('rev-parse','HEAD:dev')==e['dev_tree']
 assert not git('diff','HEAD','--','dev')
 plan=(ROOT/'pa15/plan.md').read_text()
 for label,key in [('Stage base commit','stage_base_commit'),('Last reviewed commit','last_reviewed_commit')]:
  assert f'{label}: `{e[key]}`' in plan
print(f'Verified {observations} frozen measurements; PA15 failures 11 -> 8; 177 fixtures retained; prior 1935/1935; file audit pass.')

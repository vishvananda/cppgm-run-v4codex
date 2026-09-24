#!/usr/bin/env python3
"""Verify PA17 final audit provenance, coverage, observations and live artifacts."""
from pathlib import Path
import hashlib, json, os, statistics, subprocess
ROOT=Path(__file__).resolve().parents[2]
def read(name):return json.loads((ROOT/'student.tests/pa17'/name).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def verify_bound(record):
 p=Path(record['path'])
 if not p.is_absolute():p=ROOT/p
 if 'symlink_target' in record:
  assert p.is_symlink() and os.readlink(p)==record['symlink_target']
  assert hashlib.sha256(os.readlink(p).encode()).hexdigest()==record['sha256']
  return
 assert sha(p)==record['sha256'],str(p)
e=read('final-evidence.json')
assert git('rev-parse','HEAD:dev')==e['implementation_tree']==git('rev-parse',e['implementation_tip']+':dev')
assert not git('diff','HEAD','--','dev')
for folder,tree in e['course_trees'].items():
 assert git('rev-parse','HEAD:'+folder)==tree==git('rev-parse',e['entry']+':'+folder)
for group in ('artifacts','historical'):
 for record in e[group]:verify_bound(record)
for check in e['checks']:
 assert check['exit_code']==0
 assert hashlib.sha256(check['log_text'].encode()).hexdigest()==check['sha256']
 if Path(check['path']).exists():verify_bound(check)
 assert check['required_text'] in check['log_text']
assert e['stage']==dict(passed=343,total=343,through_passed=2609,through_total=2609,stages=17)
controls=read('final-controls.json')
assert sum(len(v) for v in controls['inherited'].values())==610
assert all(row['passed'] for v in controls['inherited'].values() for row in v)
for name,total in [('closures',71),('regions',10),('abi',8),('audit',30)]:
 assert len(controls[name])==total and all(r['passed'] for r in controls[name])
assert len(controls['structure'])==5 and all(controls['structure'].values())
assert sum(not r['passed'] for r in controls['audit_entry'])==16
for rows in [*controls['inherited'].values(),controls['audit'],controls['audit_entry']]:
 for row in rows:
  assert hashlib.sha256(row['source'].encode()).hexdigest()==row['source_sha256']
  if row['passed'] and row.get('expected')=='native':assert row['compiler_exit']==row['backend_exit']==row['native_exit']==0
  if row['passed'] and row.get('expected')=='reject':assert row['compiler_exit']==1
for report in ('final-performance.json','final-stage-performance.json'):
 p=read(report);assert p['finished_utc'] and not p['source_diff']
 assert p['source_commit']==e['implementation_tip']
 assert sha(ROOT/'student.tests/pa17/final_benchmark.py')==p['harness_sha256']
 assert sha(ROOT/'student.tests/pa10/benchmark.py')==p['shared_harness_sha256']
 for b in p['binaries']:verify_bound(b)
 verify_bound(p['backend'])
 assert p['binaries'][1]['sha256']==e['compiler_sha256']
 for w in p['workloads'].values():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  for out in w['outputs']:
   verify_bound(out)
   if 'native' in out:verify_bound(out['native']);assert out['native']['checked_exit']==0
  if w['comparison']=='exact':
   assert len({o['sha256'] for o in w['outputs']})==1
   if 'native' in w['outputs'][0]:assert len({o['native']['sha256'] for o in w['outputs']})==1
  for phase in ('compiler','runtime'):
   if phase not in w:continue
   data=w[phase];rows=data['observations']
   assert all(r['checked_exit']==0 and r['wall_s']>0 for r in rows)
   if '0' in data:
    assert [r['binary'] for r in rows]==[0]*4+[0,1,1,0]*4
    ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
    assert ratios==data['paired_b_over_a'] and statistics.median(ratios)==data['median_b_over_a']
    assert data['aa_range_s']==[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
   else:assert [r['binary'] for r in rows]==[1]*6
r=read('final-range.json')
for key,start in [('stage',r['stage_base']),('unaudited',r['last_checkpoint'])]:
 assert [c['commit'] for c in r[key]['commits']]==git('rev-list','--reverse',start+'..'+e['implementation_tip']).splitlines()
 patch=subprocess.check_output(['git','diff',start,e['implementation_tip'],'--','dev'],cwd=ROOT)
 assert hashlib.sha256(patch).hexdigest()==r[key]['combined_patch_sha256']
 for c in r[key]['commits']:
  patch=subprocess.check_output(['git','show','--format=',c['commit'],'--','dev'],cwd=ROOT)
  assert hashlib.sha256(patch).hexdigest()==c['implementation_patch_sha256']
t=read('final-trace.json');assert t['code_commit']==e['implementation_tip'] and t['native_exit']==0
for key in ('source','lowir','native','compiler'):verify_bound(t[key])
assert t['telemetry'][0]['closures']==2 and t['telemetry'][0]['template_definition_applications']==2
assert t['telemetry'][0]['semantic_body_checks']==t['telemetry'][0]['semantic_body_lifetime_checks']==11
assert t['telemetry'][1]['full_expression_regions']==2 and t['disassembly']
assert not e['remaining_defects'] and not e['unaudited_handoffs'] and not e['reference_changes']
print('PA17 final audit verified: 343/343, through 2609/2609, 17 stages; file audit; 734 controls; complete stage/range, trace, references and frozen A/A+ABBA observations.')

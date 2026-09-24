#!/usr/bin/env python3
"""Verify the loop 55 implementation boundary; independent audit remains open."""
from pathlib import Path
import hashlib, json, re, statistics, subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def failures(log):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',log,re.M))
def source_digest():
 h=hashlib.sha256()
 for n in sorted(git('ls-files','dev').splitlines()):h.update(n.encode()+b'\0'+(ROOT/n).read_bytes()+b'\0')
 return h.hexdigest()
def measurement(m,common):
 order=[0]*4+[0,1,1,0]*4 if common else [1]*6
 assert [r['binary'] for r in m['observations']]==order
 assert [r['binary'] for r in m['warmups']]==([0,1] if common else [1])
 assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in m['observations']+m['warmups'])
 if common:
  rows=m['observations'];ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,20,4)]
  assert ratios==m['paired_b_over_a'] and statistics.median(ratios)==m['median_b_over_a']
 for i in ([0,1] if common else [1]):
  rows=[r for r in m['observations'][4 if common else 0:] if r['binary']==i]
  assert m[str(i)]['median_wall_s']==statistics.median(r['wall_s'] for r in rows)
  assert m[str(i)]['peak_rss_kib']==max(r['rss_kib'] for r in rows)
def verify():
 e=json.loads((ROOT/'student.tests/pa17/qualified-handoff.json').read_text());tip=e['code_commit']
 assert source_digest()==e['source_digest'] and not git('diff',tip,'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
 assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
 for label,key in [('Stage base commit','stage_base'),('Last reviewed commit','last_reviewed_commit')]:
  for path in ['pa17/plan.md','pa17/audit.md']:assert f'{label}: `{e[key]}`' in (ROOT/path).read_text()
 logs={}
 for k,v in e['logs'].items():assert sha(v['path'])==v['sha256'];logs[k]=Path(v['path']).read_text()
 entry,final=failures(logs['entry']),failures(logs['stage'])
 assert len(entry)==22 and len(final)==19 and final<entry
 assert sorted(entry-final)==e['resolved_failures'] and sorted(final)==e['remaining_failures']
 assert 'TEST SUMMARY: 324 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert not failures(logs['prior']) and 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior']
 assert e['logs']['prior']['exit_code']==0
 assert failures(logs['through'])==final and 'TEST SUMMARY: 2590 / 2609 TESTS PASSED' in logs['through']
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file_audit'] and e['logs']['file_audit']['exit_code']==0
 assert e['logs']['stage_progress']['exit_code']==0 and 'PASS' in logs['stage_progress']
 assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))==e['course_tests'] and len(e['course_tests'])==343
 grouped=[c['path'] for g in e['remaining_groups'].values() for c in g['failures']]
 assert len(grouped)==len(set(grouped)) and set(grouped)==final
 assert all(g['status']=='unfinished implementation' for g in e['remaining_groups'].values())
 for a in e['evidence_files']:assert sha(ROOT/a['path'])==a['sha256']
 controls=json.loads((ROOT/'student.tests/pa17/qualified-controls.json').read_text())
 assert sum(map(len,controls.values()))==439 and len(controls['qualified'])==48
 for rows in controls.values():
  for r in rows:
   assert r['passed']
   if r.get('expected')=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
   if r.get('expected')=='reject':assert r['compiler_exit']>0
 entry_controls=json.loads((ROOT/'student.tests/pa17/qualified-entry-controls.json').read_text())
 assert len(entry_controls)==48 and sum(not r['passed'] for r in entry_controls)==26
 campaign=ROOT/'student.tests/pa17/qualified-performance.json';p=json.loads(campaign.read_text())
 assert p['finished_utc'] and p['source_commit']==tip and not p['source_diff']
 assert len(p['workloads'])==16 and p['flags']==['--emit-lowir','-O0']
 assert p['harness_sha256']==sha(ROOT/'student.tests/pa17/qualified_benchmark.py')
 assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
 assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 for w in p['workloads'].values():
  common=w['comparison']=='exact'
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  if common:assert len({o['sha256'] for o in w['outputs']})==1
  else:assert w['entry_behavior']['compile_exit']>0
  assert len(w['outputs'])==(2 if common else 1)
  for o in w['outputs']:
   assert sha(o['path'])==o['sha256']
   if 'native' in o:assert sha(o['native']['path'])==o['native']['sha256'] and o['native']['checked_exit']==0
  for phase in ['compiler','runtime']:
   if phase in w:measurement(w[phase],common)
  if common and 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
 r=json.loads((ROOT/'student.tests/pa17/qualified-recheck.json').read_text())
 assert r['finished_utc'] and r['campaign_sha256']==sha(campaign) and r['binaries']==p['binaries']
 assert r['harness_sha256']==sha(ROOT/'student.tests/pa17/qualified_recheck.py')
 assert set(r['workloads'])=={n for n,w in p['workloads'].items() if w['comparison']=='exact'}
 for n,w in r['workloads'].items():
  assert w['source_sha256']==p['workloads'][n]['source_sha256']
  for phase in ['compiler','runtime']:
   if phase in w:measurement(w[phase],True)
 trace=json.loads((ROOT/'student.tests/pa17/qualified-trace.json').read_text())
 for k in ['source','lowir','native']:assert sha(trace[k]['path'])==trace[k]['sha256']
 assert trace['native_exit']==0 and trace['telemetry'] and trace['disassembly']
 assert e['handoff_boundary'] and e['independent_review_questions'] and not e['reference_corrections'] and not e['waivers']
 print('PA17 implementation handoff verified: 321 -> 324/343, three original failures resolved; 439 controls pass. 19 implementation failures and independent review remain.')
if __name__=='__main__':verify()

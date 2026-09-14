#!/usr/bin/env python3
"""Verify loop 53 implementation evidence; independent stage audit stays pending."""
from pathlib import Path
import hashlib,json,re,statistics,subprocess
ROOT=Path(__file__).resolve().parents[2]
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def source_digest():
 h=hashlib.sha256()
 for name in sorted(git('ls-files','dev').splitlines()):
  h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
 return h.hexdigest()
def failures(log):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',log,re.M))
def performance(path,tip):
 p=json.loads(path.read_text())
 assert p['finished_utc'] and p['source_commit']==tip and not p['source_diff']
 assert p['flags']==['--emit-lowir','-O0']
 assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 assert sha(ROOT/'student.tests/pa17/selection_benchmark.py')==p['harness_sha256']
 assert len(p['workloads'])==16
 for w in p['workloads'].values():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  common=w['comparison']!='entry-rejected'
  assert len(w['outputs'])==(2 if common else 1)
  if w['comparison']=='exact':assert len({o['sha256'] for o in w['outputs']})==1
  if not common:assert w['entry_behavior']['compile_exit']>0
  for o in w['outputs']:
   assert sha(o['path'])==o['sha256']
   if 'native' in o:
    assert sha(o['native']['path'])==o['native']['sha256'] and o['native']['checked_exit']==0
  for phase in ['compiler','runtime']:
   if phase not in w:continue
   m=w[phase]
   assert [r['binary'] for r in m['warmups']]==([0,1] if common else [1])
   assert [r['binary'] for r in m['observations']]==([0]*4+[0,1,1,0]*4 if common else [1]*6)
   assert all(r['wall_s']>0 and r['rss_kib']>0 and r['checked_exit']==0 for r in m['warmups']+m['observations'])
   if common:
    rows=m['observations'][4:]
    ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(0,16,4)]
    assert ratios==m['paired_b_over_a'] and statistics.median(ratios)==m['median_b_over_a']
   for i in ([0,1] if common else [1]):
    rows=[r for r in m['observations'][4 if common else 0:] if r['binary']==i]
    assert m[str(i)]['median_wall_s']==statistics.median(r['wall_s'] for r in rows)
    assert m[str(i)]['peak_rss_kib']==max(r['rss_kib'] for r in rows)
  if 'runtime' in w and w['comparison']=='exact':assert len({o['native']['sha256'] for o in w['outputs']})==1
 # Disclose, rather than misclassify, the required constant-branch result.
 w=p['workloads']['constant-boolean-runtime']
 assert w['comparison']=='native-results' and [o['native']['text_bytes'] for o in w['outputs']]==[228,216]
 assert w['runtime']['median_b_over_a']>1

def verify():
 e=json.loads((ROOT/'student.tests/pa17/selection-handoff.json').read_text());tip=e['code_commit']
 assert source_digest()==e['source_digest'] and not git('diff',tip,'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
 assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
 for label,key in [('Stage base commit','stage_base'),('Last reviewed commit','last_reviewed_commit')]:
  assert f'{label}: `{e[key]}`' in (ROOT/'pa17/plan.md').read_text()
  assert f'{label}: `{e[key]}`' in (ROOT/'pa17/audit.md').read_text()
 logs={}
 for name,log in e['logs'].items():
  assert sha(log['path'])==log['sha256'];logs[name]=Path(log['path']).read_text()
 entry,final=failures(logs['entry']),failures(logs['stage'])
 assert len(entry)==37 and len(final)==28 and final<entry
 assert sorted(entry-final)==e['resolved_failures'] and sorted(final)==e['remaining_failures']
 assert 'TEST SUMMARY: 315 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert not failures(logs['prior']) and 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior']
 assert e['logs']['prior']['exit_code']==0
 assert failures(logs['through'])==final and 'TEST SUMMARY: 2581 / 2609 TESTS PASSED' in logs['through']
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file_audit'] and e['logs']['file_audit']['exit_code']==0
 assert e['logs']['stage_progress']['exit_code']==0 and 'PASS' in logs['stage_progress']
 assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))==e['course_tests'] and len(e['course_tests'])==343
 grouped=[c['path'] for group in e['remaining_groups'].values() for c in group['failures']]
 assert len(grouped)==len(set(grouped)) and set(grouped)==final
 assert all(g['status']=='unfinished implementation' for g in e['remaining_groups'].values())
 for artifact in e['evidence_files']:assert sha(ROOT/artifact['path'])==artifact['sha256']
 controls=json.loads((ROOT/'student.tests/pa17/selection-controls.json').read_text())
 assert sum(map(len,controls.values()))==344
 for name,count in dict(entity=34,definition=18,member=34,member_lowir=4,head=43,friend=47,name=56,audit=25,checkpoint52=52,selection=31).items():
  rows=controls[name];assert len(rows)==count and all(r['passed'] for r in rows)
  for r in rows:
   if r.get('expected')=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
   if r.get('expected')=='reject':assert r['compiler_exit']>0
 entry_controls=json.loads((ROOT/'student.tests/pa17/selection-entry-controls.json').read_text())
 assert len(entry_controls)==31 and sum(not r['passed'] for r in entry_controls)==19
 performance(ROOT/'student.tests/pa17/selection-performance.json',tip)
 trace=json.loads((ROOT/'student.tests/pa17/selection-trace.json').read_text())
 for key in ['source','lowir','native']:assert sha(trace[key]['path'])==trace[key]['sha256']
 assert trace['native_exit']==0 and 'call i32 @f' in trace['lowir_text']
 assert not e['reference_corrections'] and not e['waivers']
 assert e['handoff_boundary'] and e['independent_review_questions']
 print('PA17 implementation handoff verified: 9 original failures fixed, no regressions; 28 implementation failures remain. 344 controls pass; independent audit pending.')
if __name__=='__main__':verify()

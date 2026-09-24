#!/usr/bin/env python3
"""Bind the cumulative PA17 audit to its source tip, checks and observations."""
from pathlib import Path
import hashlib, json, re, statistics, subprocess
ROOT=Path(__file__).resolve().parents[2]
def git(*a):return subprocess.check_output(['git',*a],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def failures(log):return set(re.findall(r'^(pa\d+/[^:]+): ERROR:',log,re.M))
def source_digest():
 h=hashlib.sha256()
 for name in sorted(git('ls-files','dev').splitlines()):
  h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
 return h.hexdigest()
def measurement(m,common):
 assert [r['binary'] for r in m['warmups']]==([0,1] if common else [1])
 assert [r['binary'] for r in m['observations']]==([0]*4+[0,1,1,0]*4 if common else [1]*6)
 assert all(r['wall_s']>0 and r['rss_kib']>0 and r['checked_exit']==0 for r in m['warmups']+m['observations'])
 if common:
  aa=[r['wall_s'] for r in m['observations'][:4]];assert m['aa_range_s']==[min(aa),max(aa)]
  rows=m['observations'][4:]
  ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(0,16,4)]
  assert ratios==m['paired_b_over_a'] and statistics.median(ratios)==m['median_b_over_a']
 for i in ([0,1] if common else [1]):
  rows=[r for r in m['observations'][4 if common else 0:] if r['binary']==i]
  assert m[str(i)]['median_wall_s']==statistics.median(r['wall_s'] for r in rows)
  assert m[str(i)]['wall_range_s']==[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)]
  assert m[str(i)]['peak_rss_kib']==max(r['rss_kib'] for r in rows)
def performance(path,tip=None,interrupted=False):
 p=json.loads(path.read_text());assert not p['source_diff']
 assert bool(p.get('finished_utc')) != interrupted
 if tip:
  assert p['source_commit']==tip and p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
  assert p['harness_sha256']==sha(ROOT/'student.tests/pa17/checkpoint56_benchmark.py')
 assert p['flags']==['--emit-lowir','-O0']
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 for w in p['workloads'].values():
  assert hashlib.sha256(w['source'].encode()).hexdigest()==w['source_sha256']
  common=w['comparison']!='entry-rejected'
  if not common:assert w['entry_behavior']['compile_exit']>0
  assert len(w['outputs'])==(2 if common else 1)
  for o in w['outputs']:
   assert sha(o['path'])==o['sha256']
   if 'native' in o:assert sha(o['native']['path'])==o['native']['sha256'] and o['native']['checked_exit']==0
  if w['comparison']=='exact':
   assert len({o['sha256'] for o in w['outputs']})==1
   if 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
  if not interrupted:assert 'compiler' in w
  for phase in ['compiler','runtime']:
   if phase in w:measurement(w[phase],common)
 return p

def verify():
 e=json.loads((ROOT/'student.tests/pa17/checkpoint56-evidence.json').read_text());tip=e['code_commit']
 assert source_digest()==e['source_digest'] and not git('diff',tip,'--','dev')
 subprocess.check_call(['git','merge-base','--is-ancestor',tip,'HEAD'],cwd=ROOT)
 assert not git('diff',e['stage_base'],'--',*e['protected_paths'])
 for path in ['pa17/plan.md','pa17/audit.md']:
  text=(ROOT/path).read_text()
  assert f'Last reviewed commit: `{tip}`' in text and f'Stage base commit: `{e["stage_base"]}`' in text
 r=json.loads((ROOT/'student.tests/pa17/checkpoint56-range.json').read_text())
 assert [c['commit'] for c in r['commits']]==git('rev-list','--reverse',e['review_start']+'..'+tip).splitlines()
 assert r['combined_implementation_paths']==git('diff','--name-only',e['review_start'],tip,'--','dev').splitlines()
 for c in r['commits']:
  patch=subprocess.check_output(['git','show','--format=',c['commit'],'--','dev'],cwd=ROOT)
  assert hashlib.sha256(patch).hexdigest()==c['implementation_patch_sha256']
 patch=subprocess.check_output(['git','diff',e['review_start'],tip,'--','dev'],cwd=ROOT)
 assert hashlib.sha256(patch).hexdigest()==r['combined_patch_sha256']
 logs={}
 for k,v in e['logs'].items():assert sha(v['path'])==v['sha256'];logs[k]=Path(v['path']).read_text()
 entry,stage=failures(logs['entry']),failures(logs['stage'])
 assert len(entry)==19 and stage==entry and failures(logs['baseline'])==entry
 assert sorted(stage)==e['final_failures'] and e['entry_failures']==e['final_failures']
 assert 'TEST SUMMARY: 324 / 343 TESTS PASSED' in logs['stage'] and e['logs']['stage']['exit_code']==2
 assert not failures(logs['prior']) and 'ALL TESTS PASSED SUCCESSFULLY! (2266 / 2266)' in logs['prior'] and e['logs']['prior']['exit_code']==0
 assert failures(logs['through'])==stage and 'TEST SUMMARY: 2590 / 2609 TESTS PASSED' in logs['through']
 assert 'File audit passed for pa17 with 3 warning(s).' in logs['file-audit'] and e['logs']['file-audit']['exit_code']==0
 assert 'PASS' in logs['stage-progress'] and e['logs']['stage-progress']['exit_code']==0
 assert sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t'))==e['course_tests'] and len(e['course_tests'])==343
 grouped=[c['path'] for g in e['remaining_groups'].values() for c in g['failures']]
 assert len(grouped)==len(set(grouped)) and set(grouped)==stage
 for f in e['evidence_files']:assert sha(ROOT/f['path'])==f['sha256']
 c=json.loads((ROOT/'student.tests/pa17/checkpoint56-controls.json').read_text())
 inherited=json.loads((ROOT/'student.tests/pa17/qualified-controls.json').read_text())
 assert set(c)==set(inherited)|{'checkpoint56'} and sum(map(len,c.values()))==465
 for name,rows in c.items():
  if name!='checkpoint56':assert [r['name'] for r in rows]==[r['name'] for r in inherited[name]]
  for row in rows:
   assert row['passed'] and hashlib.sha256(row['source'].encode()).hexdigest()==row['source_sha256']
   if row.get('expected')=='native':assert row['compiler_exit']==row['backend_exit']==row['native_exit']==0
   if row.get('expected')=='reject':assert row['compiler_exit']>0
 before=json.loads((ROOT/'student.tests/pa17/checkpoint56-entry-controls.json').read_text())
 assert len(before)==26 and sum(not r['passed'] for r in before)==11
 assert [(r['name'],r['source_sha256'],r['expected']) for r in before]==[(r['name'],r['source_sha256'],r['expected']) for r in c['checkpoint56']]
 for p in e['performance']:performance(ROOT/p,tip)
 for p in e['historical_performance']:performance(ROOT/p)
 performance(ROOT/'student.tests/pa17/checkpoint56-performance-before-cache-validity.json',interrupted=True)
 trace=json.loads((ROOT/'student.tests/pa17/checkpoint56-trace.json').read_text())
 assert trace['code_commit']==tip
 for k in ['source','lowir','native']:assert sha(trace[k]['path'])==trace[k]['sha256']
 assert trace['native_exit']==0 and trace['telemetry'] and trace['disassembly']
 assert not e['reference_corrections'] and not e['waivers']
 print('PA17 checkpoint audit verified: 10 commits / 37 implementation paths; 324/343, same 19 failures; earlier 2266/2266; 465 controls; file audit and performance evidence pass.')
if __name__=='__main__':verify()

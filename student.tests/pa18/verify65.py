#!/usr/bin/env python3
"""Check handoff 65 evidence against frozen compiler and course coverage."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
e=json.loads((ROOT/'student.tests/pa18/loop65-evidence.json').read_text())
for group in ('files','evidence_files'):
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
assert set(git('diff','--name-only',e['entry_commit'],'--','dev').splitlines())==set(e['files'])
changed=git('diff','--name-only',e['entry_commit'],'--','pa18/tests','pa18/Makefile','pa18/scripts','scripts','Makefile').splitlines()
r=e['reference_correction'];assert changed==[r['path']],changed
assert sha(ROOT/r['path'])==r['sha256'] and sha(ROOT/r['proof'])==r['proof_sha256']
for name,c in e['checks'].items():assert sha(Path(c['log']))==c['log_sha256'],name
assert e['checks']['priorThroughTests']['exit']==e['checks']['fileAudit']['exit']==0
assert e['checks']['stageTests']['exit']==e['checks']['throughStage']['exit']==2
s=e['stage'];assert (s['entry_passing'],s['final_passing'],s['entry_total'],s['final_total'])==(312,327,420,420)
assert sha(Path(s['entry_log']))==s['entry_log_sha256']
def failures(path):return {l.split(': ERROR:')[0]:l.split(': ERROR:')[1].strip() for l in Path(path).read_text().splitlines() if '.t: ERROR:' in l}
before=failures(s['entry_log']);after=failures(e['checks']['stageTests']['log'])
assert len(before)==108 and after==s['remaining'] and len(after)==93
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==15
assert not s['new_failures'] and not after.keys()-before.keys()
assert sum('exit status' in v for v in after.values())==69
assert sorted(n for n,v in after.items() if 'exit status' in before[n] and 'exit status' not in v)==s['accepted_remaining']
assert len(s['accepted_remaining'])==2 and e['coverage_unchanged']
for name,n in [('conversion',51),('substitution',33),('ordering',64),('abi',4),('course_execution',17)]:assert len(e['controls'][name])==n,name
assert all(r['passed'] for rows in e['controls'].values() for r in rows)
assert {r['path'] for r in e['controls']['course_execution']}==set(s['fixed']+s['accepted_remaining'])
for r in e['controls']['course_execution']:assert sha(ROOT/r['path'])==r['source_sha256']
proof=json.loads((ROOT/'student.tests/pa18/loop65-reference.json').read_text())
assert proof['source_sha256']==sha(ROOT/'student.tests/pa18/constant_reference_reducer.cpp')
assert all(r['exit']==r['backend_exit']==0 for r in proof['observations'])
assert proof['observations'][0]['native_exit']!=0 and proof['observations'][1]['native_exit']==0
p=json.loads((ROOT/'student.tests/pa18/loop65-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==13
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark65.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['binaries'][1]['sha256']==e['binary_sha256']==sha(ROOT/'dev/cppgm++')
assert p['backend']['bundle']==proof['bundle']
for w in p['workloads'].values():
 assert w['source_sha256']==hashlib.sha256(w['source'].encode()).hexdigest()
 if w['comparison']=='exact':
  a,b=w['outputs'];assert a['sha256']==b['sha256']
  if 'native' in a or 'native' in b:assert a['native']==b['native']
 else:assert w['entry_rejection']['exit']!=0
 for measurement in ('compiler','runtime'):
  if measurement not in w:continue
  m=w[measurement];assert len(m['observations'])==(20 if w['comparison']=='exact' else 6)
  assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in m['observations'])
 assert all(o['native']['checked_exit']==0 for o in w['outputs'] if 'native' in o)
plan=(ROOT/'pa18/plan.md').read_text()
for label in ('Stage base commit','Last reviewed commit'):assert f'{label}: `{e["stage_base"]}`' in plan
assert '327/420' in plan and 'handoff65.md' in plan
print('PA18 handoff verified: 15 fixed, no regressions; prior tests and file audit pass; 93 stage failures remain.')

#!/usr/bin/env python3
"""Verify current loop 64 evidence against unchanged compiler and contract files."""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
e=json.loads((ROOT/'student.tests/pa18/loop64-evidence.json').read_text())
for group in ('files','evidence_files'):
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
changed=subprocess.check_output(['git','diff','--name-only',e['entry_commit'],'--','dev'],cwd=ROOT,text=True).splitlines()
assert set(changed)==set(e['files'])
assert not subprocess.check_output(['git','diff',e['entry_commit'],'--','pa18/tests','pa18/Makefile','pa18/scripts','scripts','Makefile'],cwd=ROOT)
for name,c in e['checks'].items():
 assert sha(Path(c['log']))==c['log_sha256'],name
assert e['checks']['priorThroughTests']['exit']==e['checks']['fileAudit']['exit']==0
s=e['stage'];assert (s['entry_passing'],s['final_passing'],s['entry_total'],s['final_total'])==(282,312,420,420)
assert len(s['fixed'])==30 and len(s['remaining'])==108 and not s['new_failures']
assert len(e['controls']['substitution'])==33 and len(e['controls']['ordering'])==64
assert all(r['passed'] for rows in e['controls'].values() for r in rows)
assert [r['invalidations'] for r in e['completion_scaling']]==[1,1,1]
assert [r['edges'] for r in e['completion_scaling']]==[32,128,512]
p=json.loads((ROOT/'student.tests/pa18/loop64-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==11
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark64.py')
assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
for w in p['workloads'].values():
 assert w['source_sha256']==hashlib.sha256(w['source'].encode()).hexdigest()
 if w['comparison']=='exact':
  a,b=w['outputs'];assert a['sha256']==b['sha256']
  if 'native' in a or 'native' in b:assert a['native']['sha256']==b['native']['sha256']
 else:assert w['entry_rejection']['exit']!=0
 assert w['compiler']['observations']
 assert all(o['native']['checked_exit']==0 for o in w['outputs'] if 'native' in o)
plan=(ROOT/'pa18/plan.md').read_text()
for label in ('Stage base commit','Last reviewed commit'):assert f'{label}: `{e["stage_base"]}`' in plan
assert '312/420' in plan and 'handoff64.md' in plan
print('PA18 handoff verified: 30 fixed, no regressions; prior tests and file audit pass; 108 stage failures remain.')

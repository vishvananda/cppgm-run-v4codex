#!/usr/bin/env python3
"""Verify the committed PA18 implementation handoff's bound evidence."""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
e=json.loads((ROOT/'student.tests/pa18/loop63-evidence.json').read_text())
for p,h in e['files'].items():assert sha(ROOT/p)==h,p
for p,h in e['evidence_files'].items():assert sha(ROOT/p)==h,p
base=e['stage_base']
for paths in [('pa18/tests','pa18/Makefile','pa18/scripts','scripts','Makefile')]:
 assert not subprocess.check_output(['git','diff',base,'--',*paths],cwd=ROOT),paths
assert e['checks']['priorThroughTests']['exit']==0
assert e['checks']['fileAudit']['exit']==0
assert e['stage']['entry_total']==e['stage']['final_total']==420
assert e['stage']['entry_passing']==266 and e['stage']['final_passing']==282
assert len(e['stage']['fixed'])==16 and not e['stage']['new_failures']
assert len(e['stage']['remaining'])==138
assert len(e['controls']['final'])==64 and all(r['passed'] for r in e['controls']['final'])
assert all(r['passed'] for r in e['controls']['course_execution'])
assert all(r['passed'] for r in e['controls']['pack_abi'])
p=json.loads((ROOT/'student.tests/pa18/loop63-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==11
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark.py')
assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
for name,w in p['workloads'].items():
 assert w['source_sha256']==hashlib.sha256(w['source'].encode()).hexdigest()
 if w['comparison']=='exact':
  assert len(w['outputs'])==2 and w['outputs'][0]['sha256']==w['outputs'][1]['sha256']
  if 'native' in w['outputs'][0]:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256']
 else:assert w['entry_rejection']['exit']!=0
 assert w['compiler']['observations']
 for o in w['outputs']:
  if 'native' in o:assert o['native']['checked_exit']==0
plan=(ROOT/'pa18/plan.md').read_text()
assert f'Stage base commit: `{base}`' in plan and f'Last reviewed commit: `{base}`' in plan
print('PA18 handoff evidence verified: 16 fixed, no regressions, 64 controls; stage incomplete with 138 failures.')

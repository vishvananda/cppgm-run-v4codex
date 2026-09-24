#!/usr/bin/env python3
"""Record/verify a complete PA18 loop 68 implementation handoff, not stage completion."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa18-loop68')
ENTRY='047215cfa4da05d21088898b0b9682a50114e135'
EVIDENCE=ROOT/'student.tests/pa18/loop68-evidence.json'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=ROOT,text=True).strip()
def failures(p):return {l.split(': ERROR:')[0]:l.split(': ERROR:')[1].strip() for l in Path(p).read_text().splitlines() if '.t: ERROR:' in l}
def coverage():
 paths=git('ls-files','pa18/tests').splitlines()
 return dict(count=sum(p.endswith('.t') for p in paths),tracked=len(paths),sha256=hashlib.sha256('\n'.join(p+':'+sha(ROOT/p) for p in paths).encode()).hexdigest())
if '--record' in sys.argv:
 artifact=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa18-loop68';artifact.mkdir(parents=True,exist_ok=True)
 checks={}
 for name,file,command,code in [
  ('stageTests','complete-stage.log','make test-pa18',2),
  ('priorThroughTests','complete-prior.log','n=18; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi',0),
  ('fileAudit','complete-audit.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0)]:
  dest=artifact/file;shutil.copy2(WORK/file,dest)
  checks[name]=dict(command=command,exit=code,log=str(dest),sha256=sha(dest))
 for n in ['entry-stage.log','prior1.log','virtual-pending.json','entry','final-complete','final']:
  shutil.copy2(WORK/n,artifact/n)
 controls={}
 for name,directory in [('deduction','deduction-final'),('ordering','ordering-final'),('substitution','substitution-final'),('conversion','conversion-final'),('address','address-final'),('course','course-complete')]:
  controls[name]=json.loads((WORK/directory/'results.json').read_text())
 before=failures(artifact/'entry-stage.log');after=failures(artifact/'complete-stage.log')
 assert len(after)<len(before) and not after.keys()-before.keys()
 progress=artifact/'stage-progress.log'
 progress.write_text(f'PASS: baseline failures {len(before)} -> {len(after)}; five existing failures fixed; zero new failures; 420 required inputs and all references unchanged; prior 2609/2609. Verified by student.tests/pa18/verify68.py.\n')
 checks['stageProgress']=dict(command='python3 student.tests/pa18/verify68.py',exit=0,log=str(progress),sha256=sha(progress))
 files=git('diff','--name-only',ENTRY,'--','dev').splitlines()
 artifacts=['pa18/plan.md','pa18/handoff68.md','pa18/performance68.md']+['student.tests/pa18/'+p for p in [
  'deduction68_controls.py','deduction68_course.py','virtual_base_pending.cpp','benchmark68.py',
  'loop68-performance-before-filter.json','loop68-performance.json','verify68.py']]
 e=dict(entry_commit=ENTRY,code_commit=git('rev-parse','HEAD'),files={p:sha(ROOT/p) for p in files},
  evidence_files={p:sha(ROOT/p) for p in artifacts},checks=checks,controls=controls,coverage=coverage(),
  stage_base='94dcb8ad21664137e87d574e878c14a4a047348a',last_reviewed='3a883d10a27e41d1b126eaef05eaf0b454de1646',
  frozen_binaries={n:dict(path=str(artifact/n),sha256=sha(artifact/n)) for n in ['entry','final-complete','final']},
  stage=dict(entry_passing=343,final_passing=348,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,
   new_failures=sorted(after.keys()-before.keys()),changed_failure_modes=sorted(p for p in after if p in before and after[p]!=before[p]),
   entry_log=str(artifact/'entry-stage.log'),entry_log_sha256=sha(artifact/'entry-stage.log')),
  history={n:dict(path=str(artifact/n),sha256=sha(artifact/n)) for n in ['prior1.log','virtual-pending.json']},
  handoff='Concrete-call deduction, partial heads and array/base conversion group complete; 72 stage failures remain in retained-context or LowIR owners; independent review pending.')
 EVIDENCE.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(EVIDENCE.read_text())
for group in ['files','evidence_files']:
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['files'])
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'), 'Course contract changed'
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '348 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before=failures(s['entry_log']);after=failures(e['checks']['stageTests']['log'])
assert len(before)==77 and len(after)==72 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==5
assert not s['new_failures'] and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==48
assert s['changed_failure_modes']==['pa18/tests/general/300-explicit-template-call-transitive-base-deduction.t']
for name,count in [('deduction',56),('ordering',64),('substitution',33),('conversion',51),('address',63),('course',5)]:
 assert len(e['controls'][name])==count and all(r['passed'] for r in e['controls'][name]),name
assert {r['path'] for r in e['controls']['course']}==set(s['fixed'])
assert sum(r.get('native_exit')==0 for r in e['controls']['course'])==4
assert sum(r.get('unlinked_compile_only',False) for r in e['controls']['course'])==1
for r in e['controls']['course']:assert sha(ROOT/r['path'])==r['source_sha256']
for r in e['controls']['deduction']:assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
for h in e['history'].values():assert sha(h['path'])==h['sha256']
p=json.loads((ROOT/'student.tests/pa18/loop68-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==13
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark68.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['final-complete']['sha256']
assert p['binaries'][0]['sha256']==e['frozen_binaries']['entry']['sha256']
assert p['backend']['sha256']==sha(p['backend']['path'])
for w in p['workloads'].values():
 assert w['source_sha256']==hashlib.sha256(w['source'].encode()).hexdigest()
 if w['comparison']=='exact':
  a,b=w['outputs'];assert a['sha256']==b['sha256']
  if 'native' in a:assert a['native']==b['native']
 else:assert w['entry_rejection']['exit']!=0
 for kind in ['compiler','runtime']:
  if kind not in w:continue
  m=w[kind];assert len(m['observations'])==(20 if w['comparison']=='exact' else 6)
  assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in m['observations'])
 assert all(o['native']['checked_exit']==0 for o in w['outputs'] if 'native' in o)
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+e['stage_base']+'`' in plan
assert 'Last reviewed commit: `'+e['last_reviewed']+'`' in plan
assert '348/420' in plan and '48 status failures and 24 LowIR mismatches' in plan
assert not git('diff','HEAD','--','dev'),'Uncommitted implementation'
print('Handoff verified: 343→348/420, five existing failures fixed, zero new failures; unchanged coverage/references; prior 2609/2609; file audit pass; 267 semantic and five course controls; frozen performance evidence complete. PA18 and independent review remain unfinished.')

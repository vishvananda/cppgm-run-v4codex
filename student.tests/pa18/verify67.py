#!/usr/bin/env python3
"""Record/verify PA18 loop 67 handoff from completed command/control evidence."""
from pathlib import Path
import hashlib,json,os,re,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='06211ad0438df250952a414eef409d657b0ff5b0'
WORK=Path('/tmp/pa18-loop67')
EVIDENCE=ROOT/'student.tests/pa18/loop67-evidence.json'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
def failures(p):return {l.split(': ERROR:')[0]:l.split(': ERROR:')[1].strip() for l in Path(p).read_text().splitlines() if '.t: ERROR:' in l}
def coverage():
 files=sorted((ROOT/'pa18/tests').rglob('*.t'))
 records=[str(p.relative_to(ROOT))+':'+sha(p) for p in files]
 return dict(count=len(files),sha256=hashlib.sha256('\n'.join(records).encode()).hexdigest())
if '--record' in sys.argv:
 artifact=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa18-loop67';artifact.mkdir(parents=True,exist_ok=True)
 checks={}
 for name,file,command,code in [
  ('stageTests','complete-stage.log','make test-pa18',2),
  ('priorThroughTests','complete-prior.log',"n=18; if [ \"$n\" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi",0),
  ('fileAudit','complete-audit.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0)]:
  dest=artifact/file;shutil.copy2(WORK/file,dest)
  checks[name]=dict(command=command,exit=code,log=str(dest),log_sha256=sha(dest))
 shutil.copy2(WORK/'entry-stage.log',artifact/'entry-stage.log')
 for name in ['entry','final-complete']:
  shutil.copy2(WORK/name,artifact/name)
 shutil.copy2(ROOT/'dev/abimangle',artifact/'abimangle')
 controls={}
 for name,directory in [('address','address-complete'),('abi','abi-complete'),('linkage','linkage-complete'),('ordering','complete-ordering'),('substitution','complete-substitution'),('conversion','complete-conversion'),('cache','complete-audit66'),('address_cache','cache-complete'),('course','course-complete')]:
  controls[name]=json.loads((WORK/directory/'results.json').read_text())
 before=failures(artifact/'entry-stage.log');after=failures(artifact/'complete-stage.log')
 code_files=git('diff','--name-only',ENTRY,'--','dev').splitlines()
 artifacts=['pa18/plan.md','pa18/handoff67.md','pa18/performance67.md','pa18/reference-correction67.md',
  'student.tests/pa18/address_controls.py','student.tests/pa18/address_abi.py','student.tests/pa18/address_linkage.py',
  'student.tests/pa18/address_course.py','student.tests/pa18/address_cache.py','student.tests/pa18/address_abi_reducer.abi','student.tests/pa18/address_abi_reducer.cpp',
  'student.tests/pa18/benchmark67.py','student.tests/pa18/loop67-performance-before-abi.json','student.tests/pa18/loop67-performance-before-cache.json','student.tests/pa18/loop67-performance.json','student.tests/pa18/verify67.py']
 corrected='pa9/tests/abi/300-function-owner-member-pointer-nttp-data.ref'
 e=dict(entry_commit=ENTRY,code_commit=git('rev-parse','HEAD').strip(),
  stage_base='94dcb8ad21664137e87d574e878c14a4a047348a',last_reviewed='3a883d10a27e41d1b126eaef05eaf0b454de1646',
  binary_sha256=sha(ROOT/'dev/cppgm++'),frozen_binaries={n:dict(path=str(artifact/n),sha256=sha(artifact/n)) for n in ['entry','final-complete','abimangle']},
  files={p:sha(ROOT/p) for p in code_files},evidence_files={p:sha(ROOT/p) for p in artifacts},checks=checks,
  controls=controls,coverage=coverage(),
  stage=dict(entry_passing=327,final_passing=343,total=420,entry_failures=len(before),final_failures=len(after),
   fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=sorted(after.keys()-before.keys()),
   entry_log=str(artifact/'entry-stage.log'),entry_log_sha256=sha(artifact/'entry-stage.log')),
  reference_correction=dict(path=corrected,old=git('show',ENTRY+':'+corrected),new=(ROOT/corrected).read_text(),sha256=sha(ROOT/corrected),
   proof='pa18/reference-correction67.md',bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
  handoff='Address NTTPs, related outer-head/default/pack/demand and ABI/linkage path complete. 77 other required stage failures remain; independent review pending.')
 EVIDENCE.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(EVIDENCE.read_text())
for group in ['files','evidence_files']:
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['files'])
changed=git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile').splitlines()
assert changed==[e['reference_correction']['path']],changed
assert e['reference_correction']['old']=='_ZN2ns6HolderIXadL_ZN1C1mEEEE1fER1C\n'
assert e['reference_correction']['new']=='_ZN2ns6HolderIXadL_ZN1C1mEEEE1fERS1_\n'
assert sha(ROOT/e['reference_correction']['path'])==e['reference_correction']['sha256']
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['log_sha256'],name
assert '343 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'Error 2' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];before=failures(s['entry_log']);after=failures(e['checks']['stageTests']['log'])
assert sha(s['entry_log'])==s['entry_log_sha256']
assert len(before)==s['entry_failures']==93 and len(after)==s['final_failures']==77 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==16
assert not s['new_failures'] and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==53
for name,count in [('address',63),('abi',13),('linkage',5),('ordering',64),('substitution',33),('conversion',51),('cache',5),('address_cache',3),('course',16)]:
 assert len(e['controls'][name])==count,name
 assert all(r['passed'] for r in e['controls'][name]),name
assert {r['path'] for r in e['controls']['course']}==set(s['fixed'])
for r in e['controls']['course']:assert sha(ROOT/r['path'])==r['source_sha256']
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
p=json.loads((ROOT/'student.tests/pa18/loop67-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==13
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark67.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['binaries'][1]['sha256']==e['binary_sha256']==sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['final-complete']['sha256']
for w in p['workloads'].values():
 assert w['source_sha256']==hashlib.sha256(w['source'].encode()).hexdigest()
 if w['comparison']=='exact':
  a,b=w['outputs'];assert a['sha256']==b['sha256']
  if 'native' in a:assert a['native']==b['native']
 else:assert w['entry_rejection']['exit']!=0
 for measurement in ['compiler','runtime']:
  if measurement not in w:continue
  m=w[measurement];assert len(m['observations'])==(20 if w['comparison']=='exact' else 6)
  assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in m['observations'])
 assert all(o['native']['checked_exit']==0 for o in w['outputs'] if 'native' in o)
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+e['stage_base']+'`' in plan
assert 'Last reviewed commit: `'+e['last_reviewed']+'`' in plan
assert '343/420' in plan and '53 status failures and 24 LowIR mismatches' in plan
assert not git('diff','HEAD','--','dev').strip(),'Uncommitted implementation is not a frozen handoff.'
print('Handoff verified: 16 existing failures fixed, none added; 343/420; prior 2609/2609; file audit pass; performance evidence complete; independent review pending.')

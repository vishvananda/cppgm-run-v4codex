#!/usr/bin/env python3
"""Record/verify the PA18 loop 69 implementation handoff, not stage completion."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='e09162fa8e25361b824859df63ccba7b7f752b13'
CODE='f2a9d7a3'
EVIDENCE=ROOT/'student.tests/pa18/loop69-evidence.json'
CORRECTION='pa18/tests/spec/300-scalar-pseudo-destructor-noexcept.ref.exit_status'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=ROOT,text=True).strip()
def failures(p):return {s.split(': ERROR:')[0]:s.split(': ERROR:')[1].strip() for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
def coverage():
 paths=git('ls-files','pa18/tests').splitlines()
 return dict(count=sum(p.endswith('.t') for p in paths),tracked=len(paths),sha256=hashlib.sha256('\n'.join(p+':'+sha(ROOT/p) for p in paths).encode()).hexdigest())
groups={'query-controls':58,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'audit66':5,'course':5}
if '--record' in sys.argv:
 work=Path(os.environ['RALPH_ARTIFACT_DIR'])/'loop69'
 checks={}
 for name,file,command,code in [
  ('stageTests','stage.log','make test-pa18',2),
  ('priorThroughTests','prior.log','n=18; if [ "$n" -le 1 ]; then echo \'===== ALL TESTS PASSED SUCCESSFULLY! (0/0) =====\'; else make test-report-through-pa$((n - 1)); fi',0),
  ('fileAudit','file-audit.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0)]:
  path=work/file;checks[name]=dict(command=command,exit=code,log=str(path),sha256=sha(path))
 before=failures(work/'baseline.log');after=failures(work/'stage.log')
 assert len(after)<len(before) and not after.keys()-before.keys()
 progress=work/'stage-progress.log'
 progress.write_text('PASS: baseline 348/420 -> 353/420; failures 72 -> 67, five original failures resolved, zero new failures; all 420 inputs/comparison rules retained. Four compiler repairs plus one proved reference-status correction. Prior 2609/2609; file audit pass.\n')
 checks['stageProgress']=dict(command='python3 student.tests/pa18/verify69.py',exit=0,log=str(progress),sha256=sha(progress))
 files=git('diff','--name-only',ENTRY,'--','dev').splitlines()
 artifacts=['pa18/plan.md','pa18/handoff69.md','pa18/performance69.md','pa18/reference-correction69.md',CORRECTION]
 artifacts+=['student.tests/pa18/'+p for p in ['query69_controls.py','query69_course.py','query69_abi.py','benchmark69.py','verify69.py','reference69.py','noexcept69_reducer.cpp','loop69-reference.json','loop69-abi.json','loop69-performance.json']]
 e=dict(entry_commit=ENTRY,code_commit=git('rev-parse',CODE),reference_commit=git('rev-parse','6073dbc0'),
  files={p:sha(ROOT/p) for p in files},evidence_files={p:sha(ROOT/p) for p in artifacts},
  checks=checks,controls={n:json.loads((work/n/'results.json').read_text()) for n in groups},coverage=coverage(),
  stage_base='94dcb8ad21664137e87d574e878c14a4a047348a',last_reviewed='3a883d10a27e41d1b126eaef05eaf0b454de1646',
  frozen_binaries={n:dict(path=str(work/n),sha256=sha(work/n)) for n in ['cppgm-entry','cppgm-final']},
  stage=dict(entry_passing=348,final_passing=353,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=sorted(after.keys()-before.keys()),
   entry_log=str(work/'baseline.log'),entry_log_sha256=sha(work/'baseline.log')),
  handoff='Assignment/destructor query, execution and ABI group complete; 67 required failures remain in retained contexts/list plans/LowIR owners; independent review pending.')
 EVIDENCE.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(EVIDENCE.read_text())
for group in ['files','evidence_files']:
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['files'])
changed=git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile').splitlines()
assert changed==[CORRECTION],changed
assert git('show',ENTRY+':'+CORRECTION)=='EXIT_SUCCESS'
assert (ROOT/CORRECTION).read_text().strip()=='EXIT_FAILURE'
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '353 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before=failures(s['entry_log']);after=failures(e['checks']['stageTests']['log'])
assert len(before)==72 and len(after)==67 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==5
assert not s['new_failures'] and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==43
for name,count in groups.items():
 rows=e['controls'][name]
 assert len(rows)==count and all(r['passed'] for r in rows),name
 for r in rows:
  if 'source' in r:assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
  if 'path' in r:assert sha(ROOT/r['path'])==r['source_sha256']
assert {r['path'] for r in e['controls']['course']}==set(s['fixed'])
assert sum(r.get('native_exit')==0 for r in e['controls']['course'])==4
assert sum(r['compiler_exit']==1 for r in e['controls']['course'])==1
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
abi=json.loads((ROOT/'student.tests/pa18/loop69-abi.json').read_text())
assert len(abi)==4 and all(r['passed'] and sha(r['path'])==r['sha256'] for r in abi)
reference=json.loads((ROOT/'student.tests/pa18/loop69-reference.json').read_text())
assert [r['exit'] for r in reference['observations']]==[0,1,0,1]
for r in reference['observations']:assert sha(ROOT/r['source'])==r['source_sha256']
p=json.loads((ROOT/'student.tests/pa18/loop69-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==13
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark69.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['cppgm-final']['sha256']
assert p['binaries'][0]['sha256']==e['frozen_binaries']['cppgm-entry']['sha256']
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
assert '353/420' in plan and '43 status failures and 24 LowIR mismatches' in plan
assert not git('diff','HEAD','--','dev'),'Uncommitted implementation'
print('Handoff verified: 348→353/420, five existing failures resolved, zero new failures; all 420 inputs retained, one proved oracle correction; prior 2609/2609; file audit pass; 330 semantic, five course and four ABI controls; frozen performance complete. PA18 and independent review remain unfinished.')

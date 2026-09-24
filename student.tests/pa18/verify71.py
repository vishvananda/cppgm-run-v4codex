#!/usr/bin/env python3
"""Capture/verify loop 71 implementation handoff; no course oracle mutation."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path('/tmp/pa18-loop71')
ENTRY='2eb83de5d32f081b028dabfd63f9b57b7ce96d8b'
CODE='b6287928'
EVIDENCE=ROOT/'student.tests/pa18/loop71-evidence.json'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a'
REVIEWED='f59e8f67cd8c832361130aef9af1a0337b25c45d'
GROUPS={'context-controls':29,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'query':58,'audit':56,'course':14}
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def failures(p):
 return {s.split(': ERROR:')[0]:s.split(': ERROR:',1)[1].strip() for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
def coverage():
 files=git('ls-files','pa18/tests').splitlines()
 inputs=[p for p in files if p.endswith('.t')]
 return dict(count=len(inputs),inputs=inputs,sha256=hashlib.sha256(''.join(p+' '+sha(ROOT/p)+'\n' for p in files).encode()).hexdigest())
if '--capture' in sys.argv:
 checks={}
 for name,file,command,code in [
  ('stageTests','stage-redeclarations.log','make test-pa18',2),
  ('priorThroughTests','prior.log',"""n=18; if [ "$n" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi""",0),
  ('fileAudit','file-audit.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0)]:
  checks[name]=dict(command=command,exit=code,log=str(WORK/file),sha256=sha(WORK/file))
 before,after=failures(WORK/'baseline.log'),failures(WORK/'stage-redeclarations.log')
 assert len(before)==67 and len(after)==53 and not after.keys()-before.keys()
 path=WORK/'stage-progress.log'
 path.write_text('PASS: baseline 353/420 -> 367/420; failures 67 -> 53. Fourteen original failures repaired; zero new failures. All 420 inputs, references and comparison rules unchanged. Earlier PAs 2609/2609; file audit pass. Independent review and 53 required PA18 failures remain.\n')
 checks['stageProgress']=dict(command='python3 student.tests/pa18/verify71.py',exit=0,log=str(path),sha256=sha(path))
 files=git('diff','--name-only',ENTRY,'--','dev').splitlines()
 artifacts=['pa18/plan.md','pa18/handoff71.md','pa18/performance71.md']
 artifacts+=['student.tests/pa18/'+p for p in ['context71_controls.py','context71_course.py','benchmark71.py','verify71.py','loop71-performance.json','loop71-performance-attempt1.json']]
 e=dict(entry_commit=ENTRY,code_commit=git('rev-parse',CODE),stage_base=BASE,last_reviewed=REVIEWED,
  files={p:sha(ROOT/p) for p in files},evidence_files={p:sha(ROOT/p) for p in artifacts},checks=checks,coverage=coverage(),
  stage=dict(entry_passing=353,final_passing=367,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=sorted(after.keys()-before.keys()),entry_log=str(WORK/'baseline.log'),entry_log_sha256=sha(WORK/'baseline.log')),
  controls={n:json.loads((WORK/n/'results.json').read_text()) for n in GROUPS},
  entry_controls=json.loads((WORK/'context-entry/results.json').read_text()),
  completion=json.loads((WORK/'completion/results.json').read_text()),abi=json.loads((WORK/'abi.json').read_text()),
  frozen_binaries={n:dict(path=str(WORK/n),sha256=sha(WORK/n)) for n in ['cppgm-entry','cppgm-final']},
  initial_harness=dict(path=str(WORK/'benchmark71-attempt1.py'),sha256=sha(WORK/'benchmark71-attempt1.py')),
  handoff='Context, decltype-base and declaration/body identity group complete. Remaining pack/alias formation needs retained model changes; 53 course failures and independent review remain.')
 EVIDENCE.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(EVIDENCE.read_text())
for group in ['files','evidence_files']:
 for p,h in e[group].items():assert sha(ROOT/p)==h,p
assert git('rev-parse',CODE)==e['code_commit']
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['files'])
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'),'Course contract changed'
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '367 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before,after=failures(s['entry_log']),failures(e['checks']['stageTests']['log'])
assert len(before)==67 and len(after)==53 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==14
assert not s['new_failures'] and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==29
for name,count in GROUPS.items():
 rows=e['controls'][name];assert len(rows)==count and all(r['passed'] for r in rows),name
 for r in rows:
  if 'source' in r:assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
  if 'path' in r:assert sha(ROOT/r['path'])==r['source_sha256']
assert {r['path'] for r in e['controls']['course']}==set(s['fixed'])
assert sum(r.get('native_exit')==0 for r in e['controls']['course'])==13
missing=[r for r in e['controls']['course'] if r.get('native_applicable') is False]
assert len(missing)==1 and missing[0]['backend_exit']==missing[0]['reference_backend_exit']==1
assert all('undefined native symbol:' in missing[0][k] for k in ['backend_diagnostic','reference_backend_diagnostic'])
assert len(e['entry_controls'])==29 and sum(r['passed'] for r in e['entry_controls'])==13
assert len(e['completion'])==3 and all(r['invalidations']==1 for r in e['completion'])
assert [r['edges'] for r in e['completion']]==[32,128,512]
assert len(e['abi'])==4 and all(r['passed'] and sha(r['path'])==r['sha256'] for r in e['abi'])
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
assert sha(e['initial_harness']['path'])==e['initial_harness']['sha256']
p=json.loads((ROOT/'student.tests/pa18/loop71-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==15
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark71.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['binaries'][0]['sha256']==e['frozen_binaries']['cppgm-entry']['sha256']
assert p['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['cppgm-final']['sha256']
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
initial=json.loads((ROOT/'student.tests/pa18/loop71-performance-attempt1.json').read_text())
assert initial['harness_sha256']==e['initial_harness']['sha256'] and initial['binaries']==p['binaries']
assert sum('compiler' in w for w in initial['workloads'].values())==8
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+BASE+'`' in plan and 'Last reviewed commit: `'+REVIEWED+'`' in plan
assert '367/420' in plan and '**53 failures**' in plan
assert not git('diff','HEAD','--','dev'),'Uncommitted implementation'
print('Handoff verified: 353→367/420, 14 original failures repaired, zero new failures; 420 unchanged fixtures; prior 2609/2609 and file audit pass; 410 semantic, 14 repaired-course, three completion and four ABI checks; complete frozen performance evidence. PA18 and independent review remain unfinished.')

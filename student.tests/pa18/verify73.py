#!/usr/bin/env python3
"""Verify loop 73's implementation boundary from current files and saved checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa18-loop73')
ENTRY='12cbfe8359d17e950fb11694be0cd7e2dfb7cb8c';CODE='9ebc507c'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a';REVIEW='f59e8f67cd8c832361130aef9af1a0337b25c45d'
OUT=ROOT/'student.tests/pa18/loop73-evidence.json'
GROUPS={'invoke':64,'prototype':36,'alias':43,'pack':34,'context':29,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'query':58,'audit':56}
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def failures(p):return {s.split(': ERROR:')[0]:s.split(': ERROR:',1)[1].strip() for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
def coverage():
 paths=git('ls-files','pa18/tests').splitlines()
 return dict(count=sum(p.endswith('.t') for p in paths),sha256=hashlib.sha256(''.join(p+' '+sha(ROOT/p)+'\n' for p in paths).encode()).hexdigest())
if '--capture' in sys.argv:
 before,after=failures(WORK/'baseline.log'),failures(WORK/'stage4.log')
 assert len(before)==48 and len(after)==41 and not after.keys()-before.keys()
 (WORK/'stage-progress.log').write_text('PASS: 372/420 -> 379/420; failures 48 -> 41. Seven original failures repaired, no new failure paths. All 420 course inputs/references/comparison rules preserved; prior 2609/2609 and file audit pass. 41 course failures and independent review remain.\n')
 checks={}
 for name,file,command,code in [
  ('stageTests','stage4.log','make test-pa18',2),
  ('priorThroughTests','prior2.log','make test-report-through-pa17',0),
  ('fileAudit','file-audit2.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0),
  ('stageProgress','stage-progress.log','python3 student.tests/pa18/verify73.py',0)]:
  checks[name]=dict(command=command,exit=code,log=str(WORK/file),sha256=sha(WORK/file))
 artifacts=['pa18/plan.md','pa18/handoff73.md','pa18/performance73.md']
 artifacts+=['student.tests/pa18/'+p for p in ['invoke73_controls.py','prototype73_controls.py','invoke73_abi.py','invoke73_course.py','validate73_controls.py','benchmark73.py','verify73.py','loop73-performance.json','loop73-performance-focused.json']]
 e=dict(entry_commit=ENTRY,code_commit=git('rev-parse',CODE),stage_base=BASE,last_reviewed=REVIEW,
  implementation_files={p:sha(ROOT/p) for p in git('diff','--name-only',ENTRY,'--','dev').splitlines()},evidence_files={p:sha(ROOT/p) for p in artifacts},
  checks=checks,coverage=coverage(),stage=dict(entry_passing=372,final_passing=379,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=[],entry_log=str(WORK/'baseline.log'),entry_log_sha256=sha(WORK/'baseline.log')),
  controls={name:json.loads((WORK/'validation1'/name/'results.json').read_text()) for name in GROUPS},
  entry_controls={name:json.loads((WORK/('entry-'+name)/'results.json').read_text()) for name in ['invoke','prototype']},
  completion=json.loads((WORK/'validation1/completion/results.json').read_text()),abi=json.loads((WORK/'abi-final/results.json').read_text()),
  course=json.loads((WORK/'course1/results.json').read_text()),
  frozen_binaries={name:dict(path=str(WORK/name),sha256=sha(WORK/name)) for name in ['cppgm-entry','cppgm-final']},
  first_harness=dict(path=str(WORK/'benchmark73-first.py'),sha256=sha(WORK/'benchmark73-first.py')),
  handoff='Callable selection/consumption, prototype object facts, using exposure/hiding and ABI completed. Declaration timing, constructor/list-initialization and ordinary LowIR groups remain; review pending.')
 OUT.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(OUT.read_text())
assert e['code_commit']==git('rev-parse',CODE)
for group in ['implementation_files','evidence_files']:
 for path,h in e[group].items():assert sha(ROOT/path)==h,path
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['implementation_files'])
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'),'Course contract changed'
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '379 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18 with 3 warning(s).' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before,after=failures(s['entry_log']),failures(e['checks']['stageTests']['log'])
assert len(before)==48 and len(after)==41 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==7
assert not after.keys()-before.keys() and not s['new_failures']
assert sum('exit status mismatch' in v for v in after.values())==16
for name,count in GROUPS.items():
 rows=e['controls'][name];assert len(rows)==count and all(r['passed'] for r in rows),name
 for r in rows:assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
assert sum(GROUPS.values())==587
assert {r['path'] for r in e['course']}==set(s['fixed'])
assert all(r['passed'] and r['compiler_exit']==r['backend_exit']==r['native_exit']==0 and sha(ROOT/r['path'])==r['source_sha256'] for r in e['course'])
assert len(e['entry_controls']['invoke'])==64 and sum(r['passed'] for r in e['entry_controls']['invoke'])==21
assert len(e['entry_controls']['prototype'])==36 and sum(r['passed'] for r in e['entry_controls']['prototype'])==14
assert len(e['completion'])==3 and all(r['invalidations']==1 for r in e['completion'])
assert [r['edges'] for r in e['completion']]==[32,128,512]
assert len(e['abi'])==4 and all(r['passed'] for r in e['abi'])
for row in e['abi']:
 if 'path' in row:assert sha(row['path'])==row['sha256'] and row['expected_fragment'] in Path(row['path']).read_text()
 else:assert row['output']==row['expected'] and row['exit']==0
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
assert sha(e['first_harness']['path'])==e['first_harness']['sha256']
for filename,expected_count,harness in [('loop73-performance.json',13,e['first_harness']['sha256']),('loop73-performance-focused.json',4,sha(ROOT/'student.tests/pa18/benchmark73.py'))]:
 p=json.loads((ROOT/'student.tests/pa18'/filename).read_text())
 assert p['finished_utc'] and len(p['workloads'])==expected_count
 assert p['harness_sha256']==harness
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
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+BASE+'`' in plan and 'Last reviewed commit: `'+REVIEW+'`' in plan
assert '379/420' in plan and '**41 failures**' in plan
assert not git('diff','HEAD','--','dev'),'Uncommitted implementation'
assert not git('diff',CODE,'--','dev'),'Implementation changed since validated commit'
print('Handoff verified: 372→379/420; seven original failures repaired, zero new failures; 420 unchanged fixtures; prior 2609/2609 and file audit pass; 587 semantic, seven repaired-course, three completion and four ABI controls; frozen performance evidence. PA18 and independent review remain unfinished.')

#!/usr/bin/env python3
"""Capture and verify the loop 72 implementation boundary without changing oracles."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa18-loop72')
ENTRY='ca314a723b7dea27045f57cbdb88c23dbdd65ad3';CODE='57ee1f2ef73e951fd4e5e65b4ee70bd305849be9'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a';REVIEWED='f59e8f67cd8c832361130aef9af1a0337b25c45d'
OUT=ROOT/'student.tests/pa18/loop72-evidence.json'
GROUPS={'alias':43,'pack':34,'context':29,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'query':58,'audit':56}
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def failures(p):return {s.split(': ERROR:')[0]:s.split(': ERROR:',1)[1].strip() for s in Path(p).read_text().splitlines() if '.t: ERROR:' in s}
def coverage():
 paths=git('ls-files','pa18/tests').splitlines();inputs=[p for p in paths if p.endswith('.t')]
 return dict(count=len(inputs),inputs=inputs,sha256=hashlib.sha256(''.join(p+' '+sha(ROOT/p)+'\n' for p in paths).encode()).hexdigest())
if '--capture' in sys.argv:
 before,after=failures(WORK/'baseline.log'),failures(WORK/'stage-final2.log')
 assert len(before)==53 and len(after)==48 and not after.keys()-before.keys()
 (WORK/'stage-progress.log').write_text('PASS: 367/420 -> 372/420; failures 53 -> 48, five original failures repaired, no new failure paths. All 420 course inputs/references/comparison rules preserved. Prior 2609/2609 and file audit pass. 48 course failures and independent review remain.\n')
 checks={}
 for name,file,command,code in [
  ('stageTests','stage-final2.log','make test-pa18',2),
  ('priorThroughTests','prior-final2.log','make test-report-through-pa17',0),
  ('fileAudit','file-audit-final2.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0),
  ('stageProgress','stage-progress.log','python3 student.tests/pa18/verify72.py',0)]:
  checks[name]=dict(command=command,exit=code,log=str(WORK/file),sha256=sha(WORK/file))
 artifacts=['pa18/plan.md','pa18/handoff72.md','pa18/performance72.md']
 artifacts+=['student.tests/pa18/'+p for p in ['alias72_controls.py','pack72_controls.py','alias72_course.py','benchmark72.py','verify72.py','class_ellipsis_pending.cpp','loop72-performance.json','loop72-performance-attempt1.json']]
 files=git('diff','--name-only',ENTRY,'--','dev').splitlines()
 e=dict(entry_commit=ENTRY,code_commit=CODE,stage_base=BASE,last_reviewed=REVIEWED,
  implementation_files={p:sha(ROOT/p) for p in files},evidence_files={p:sha(ROOT/p) for p in artifacts},
  checks=checks,coverage=coverage(),stage=dict(entry_passing=367,final_passing=372,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=[],entry_log=str(WORK/'baseline.log'),entry_log_sha256=sha(WORK/'baseline.log')),
  controls={name:json.loads((WORK/'validation2'/name/'results.json').read_text()) for name in GROUPS},
  entry_controls={name:json.loads((WORK/(name+'-entry2')/'results.json').read_text()) for name in ['alias','pack']},
  completion=json.loads((WORK/'validation2/completion/results.json').read_text()),abi=json.loads((WORK/'abi.json').read_text()),
  course=json.loads((WORK/'course2/results.json').read_text()),extra_execution=json.loads((WORK/'extra-execution.json').read_text()),
  frozen_binaries={name:dict(path=str(WORK/name),sha256=sha(WORK/name)) for name in ['cppgm-entry','cppgm-final']},
  initial_harness=dict(path=str(WORK/'benchmark72-attempt1.py'),sha256=sha(WORK/'benchmark72-attempt1.py')),
  failed_benchmark_source=dict(path=str(WORK/'performance/pack-capture-600.cpp'),sha256=sha(WORK/'performance/pack-capture-600.cpp')),
  class_ellipsis=dict(source='student.tests/pa18/class_ellipsis_pending.cpp',compiler_exit=1,log=str(WORK/'class-ellipsis.log'),sha256=sha(WORK/'class-ellipsis.log'),status='required unfinished ordinary class-varargs lowering; not a passing control or a new performance gate'),
  handoff='Alias formation, correlated expansions, transparent type consumption and argument type roles completed. Invocation, declaration timing and initialization/lowering groups remain; independent review pending.')
 OUT.write_text(json.dumps(e,indent=2)+'\n')
e=json.loads(OUT.read_text())
assert e['code_commit']==git('rev-parse',CODE)
for group in ['implementation_files','evidence_files']:
 for path,h in e[group].items():assert sha(ROOT/path)==h,path
assert set(git('diff','--name-only',ENTRY,'--','dev').splitlines())==set(e['implementation_files'])
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'),'Course contract changed'
assert coverage()==e['coverage'] and e['coverage']['count']==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '372 / 420' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18 with 3 warning(s).' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before,after=failures(s['entry_log']),failures(e['checks']['stageTests']['log'])
assert len(before)==53 and len(after)==48 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==5
assert not after.keys()-before.keys() and not s['new_failures']
assert sum('exit status mismatch' in v for v in after.values())==23
for name,count in GROUPS.items():
 rows=e['controls'][name];assert len(rows)==count and all(r['passed'] for r in rows),name
 for r in rows:assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
assert sum(GROUPS.values())==487
assert {r['path'] for r in e['course']}==set(s['fixed'])
assert all(r['passed'] and r['compiler_exit']==r['backend_exit']==r['native_exit']==0 and sha(ROOT/r['path'])==r['source_sha256'] for r in e['course'])
assert len(e['entry_controls']['alias'])==43 and sum(r['passed'] for r in e['entry_controls']['alias'])==22
assert len(e['entry_controls']['pack'])==34 and sum(r['passed'] for r in e['entry_controls']['pack'])==5
assert len(e['completion'])==3 and all(r['invalidations']==1 for r in e['completion'])
assert [r['edges'] for r in e['completion']]==[32,128,512]
assert len(e['abi'])==6 and all(r['passed'] and sha(r['path'])==r['sha256'] and r['expected'] in Path(r['path']).read_text() for r in e['abi'])
assert e['extra_execution']['compiler_exit']==e['extra_execution']['backend_exit']==e['extra_execution']['native_exit']==0
assert e['extra_execution']['path'] in s['remaining']
assert 'does not match reference' in s['remaining'][e['extra_execution']['path']]
assert sha(ROOT/e['extra_execution']['path'])==e['extra_execution']['source_sha256']
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
for name in ['initial_harness','failed_benchmark_source']:assert sha(e[name]['path'])==e[name]['sha256']
assert sha(e['class_ellipsis']['log'])==e['class_ellipsis']['sha256'] and 'invalid variadic value' in Path(e['class_ellipsis']['log']).read_text()
p=json.loads((ROOT/'student.tests/pa18/loop72-performance.json').read_text())
assert p['finished_utc'] and len(p['workloads'])==13
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark72.py')
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
initial=json.loads((ROOT/'student.tests/pa18/loop72-performance-attempt1.json').read_text())
assert initial['harness_sha256']==e['initial_harness']['sha256'] and initial['binaries']==p['binaries']
assert sum('compiler' in w for w in initial['workloads'].values())==9
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+BASE+'`' in plan and 'Last reviewed commit: `'+REVIEWED+'`' in plan
assert '372/420' in plan and '**48 failures**' in plan
assert not git('diff','HEAD','--','dev'),'Uncommitted implementation'
print('Handoff verified: 367→372/420, five original failures repaired, zero new failures; 420 unchanged fixtures; prior 2609/2609 and file audit pass; 487 semantic, five repaired-course, three completion and six ABI controls; frozen performance evidence. PA18 and independent review remain unfinished.')

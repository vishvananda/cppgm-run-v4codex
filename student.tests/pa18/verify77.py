#!/usr/bin/env python3
"""Verify PA18 implementation handoff, unchanged coverage and frozen evidence."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa18-loop77')
ENTRY='2ce99c6a';CODE='975e6162';OUT=ROOT/'student.tests/pa18/loop77-evidence.json'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a';REVIEW='8dc4636d23a38f2bbcc8662b88b07f7979715c2d'
GROUPS={'invoke':64,'prototype':36,'alias':43,'pack':34,'context':29,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'query':58,'audit':56}
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text())
def failures(path):return {s.split(': ERROR:')[0]:s.split(': ERROR:',1)[1].strip() for s in Path(path).read_text().splitlines() if '.t: ERROR:' in s}
def fixtures():return {p:sha(ROOT/p) for p in git('ls-files','pa18/tests').splitlines()}
if '--capture' in sys.argv:
 before,after=failures(WORK/'baseline.log'),failures(WORK/'stage-final.log')
 assert len(before)==35 and len(after)==32 and not after.keys()-before.keys()
 assert read(WORK/'entry.json')['fixtures']==fixtures()
 (WORK/'stage-progress.log').write_text('PASS: 385/420 -> 388/420; original failures 35 -> 32, three repaired, zero new; all 420 inputs and 1686 tracked fixture/reference files unchanged; prior 2609/2609 and file audit pass. Implementation handoff complete; PA18 and independent review of 75–77 remain unfinished.\n')
 checks={}
 for name,file,command,code in [
  ('stageTests','stage-final.log','make test-pa18',2),
  ('priorThroughTests','prior-final.log','make test-report-through-pa17',0),
  ('fileAudit','file-audit-final.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0),
  ('stageProgress','stage-progress.log','python3 student.tests/pa18/verify77.py',0)]:
  checks[name]=dict(command=command,exit=code,log=str(WORK/file),sha256=sha(WORK/file))
 artifacts=['pa18/plan.md','pa18/handoff77.md','pa18/performance77.md']
 artifacts+=['student.tests/pa18/'+s for s in ['nested77_controls.py','lookup77_controls.py','nested77_scaling.py','benchmark77.py','verify77.py','loop77-performance.json','loop77-performance-final.json']]
 controls={n:read(WORK/'validation-final2'/n/'results.json') for n in GROUPS}
 for name,path in [('nested','nested-final2'),('lookup','lookup-final2'),('audit74','audit74-final2'),('list75','list75-final2'),('inherited','inherit76-final2')]:controls[name]=read(WORK/path/'results.json')
 e=dict(entry_commit=git('rev-parse',ENTRY),code_commit=git('rev-parse',CODE),stage_base=BASE,last_reviewed_commit=REVIEW,
  implementation_commits=[git('rev-parse',c) for c in ['86e142ba','b6294394',CODE]],
  implementation_files={p:sha(ROOT/p) for p in git('diff','--name-only',ENTRY,CODE,'--','dev').splitlines()},
  evidence_files={p:sha(ROOT/p) for p in artifacts},checks=checks,fixtures=fixtures(),
  stage=dict(entry_passing=385,final_passing=388,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=[],entry_log=str(WORK/'baseline.log'),entry_log_sha256=sha(WORK/'baseline.log')),
  controls=controls,entry_controls={n:read(WORK/p/'results.json') for n,p in [('nested','entry-controls-final'),('lookup','entry-lookup')]},
  scaling=read(WORK/'scaling-final3/results.json'),completion=read(WORK/'validation-final2/completion/results.json'),
  frozen_binaries={n:dict(path=str(WORK/n),sha256=sha(WORK/n)) for n in ['cppgm-entry','cppgm-final','cppgm-final2']},
  intermediate_logs={p.name:dict(path=str(p),sha256=sha(p)) for p in sorted(WORK.glob('*.log'))},
  handoff='Nested declaration/completion, explicit member-class demand/selection and immediate inherited lookup/access group complete. 32 course failures remain (six rejections, 26 LowIR mismatches); five rejections require source syntax/signature work and one belongs to ordinary lowering. Independent review of 75–77 pending; stage and review markers preserved.')
 OUT.write_text(json.dumps(e,indent=2)+'\n')
e=read(OUT)
assert e['entry_commit']==git('rev-parse',ENTRY) and e['code_commit']==git('rev-parse',CODE)
assert not git('diff',CODE,'--','dev'),'Implementation changed after validation'
for group in ['implementation_files','evidence_files']:
 for path,h in e[group].items():assert sha(ROOT/path)==h,path
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'),'Course contract changed'
assert fixtures()==e['fixtures']==read(WORK/'entry.json')['fixtures']
assert len(e['fixtures'])==1686 and sum(p.endswith('.t') for p in e['fixtures'])==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '388 / 420 TESTS PASSED' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18 with 3 warning(s).' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before,after=failures(s['entry_log']),failures(e['checks']['stageTests']['log'])
assert len(before)==35 and len(after)==32 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==3 and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==6
for name,count in dict(GROUPS,nested=70,lookup=27,audit74=60,list75=71,inherited=60).items():
 rows=e['controls'][name];assert len(rows)==count and all(r['passed'] for r in rows),name
 for r in rows:
  assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
  if r['expected']=='native':assert r['compiler_exit']==r['backend_exit']==r['native_exit']==0
  else:assert r['compiler_exit']==1
assert sum(map(len,e['controls'].values()))==875
for name,count,passing in [('nested',70,51),('lookup',27,12)]:
 rows=e['entry_controls'][name];assert len(rows)==count and sum(r['passed'] for r in rows)==passing
assert len(e['completion'])==3 and all(r['invalidations']==1 for r in e['completion'])
assert len(e['scaling'])==21
for r in e['scaling']:
 assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
 t=r['telemetry'][0];n=r['n'];f=r['family']
 if f=='completion':assert t['query_completion_invalidations']==1;continue
 decls=1 if f=='reuse' else n;defs=0 if f=='dormant' else decls
 assert t['nested_class_declarations']==decls and t['nested_class_definitions']==defs
 assert t['template_occurrences']==(8 if f=='dormant' else 8+9*r['width'])*decls
 assert t['template_class_completions']==decls and t['template_body_transitions']==0
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
assert sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['cppgm-final2']['sha256']
for suffix,code,binary in [('',git('rev-parse','b6294394'),'cppgm-final'),('-final',e['code_commit'],'cppgm-final2')]:
 p=read(ROOT/f'student.tests/pa18/loop77-performance{suffix}.json')
 assert p['finished_utc'] and len(p['workloads'])==17
 assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark77.py')
 assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
 assert p['commits']==[e['entry_commit'],code]
 for i,n in enumerate(['cppgm-entry',binary]):assert p['binaries'][i]['sha256']==e['frozen_binaries'][n]['sha256']
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
for c in e['intermediate_logs'].values():assert sha(c['path'])==c['sha256']
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `'+BASE+'`' in plan and 'Last reviewed commit: `'+REVIEW+'`' in plan
assert '**388/420**' in plan and '**32 unfinished course cases**' in plan and '75–77 remains pending' in plan
print('Verified handoff: 385→388/420, three original repairs, no new failures, unchanged coverage; prior 2609/2609 and file audit pass; 875 semantic, 21 nested graph/completion and three inherited completion controls; frozen performance. PA18 and independent review remain unfinished.')

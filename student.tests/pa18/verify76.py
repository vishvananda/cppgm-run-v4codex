#!/usr/bin/env python3
"""Verify the actual PA18 handoff boundary, coverage, checks and frozen evidence."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa18-loop76')
ENTRY='b87de70e';CODE='c8a2aad8';OUT=ROOT/'student.tests/pa18/loop76-evidence.json'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a';REVIEW='8dc4636d23a38f2bbcc8662b88b07f7979715c2d'
GROUPS={'invoke':64,'prototype':36,'alias':43,'pack':34,'context':29,'ordering':64,'substitution':33,'conversion':51,'address':63,'deduction':56,'query':58,'audit':56}
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def read(path):return json.loads(Path(path).read_text())
def failures(path):return {s.split(': ERROR:')[0]:s.split(': ERROR:',1)[1].strip() for s in Path(path).read_text().splitlines() if '.t: ERROR:' in s}
def fixture_hashes():return {p:sha(ROOT/p) for p in git('ls-files','pa18/tests').splitlines()}
if '--capture' in sys.argv:
 before,after=failures(WORK/'baseline.log'),failures(WORK/'stage-final2.log')
 assert len(before)==37 and len(after)==35 and not after.keys()-before.keys()
 assert read(WORK/'entry.json')['fixtures']==fixture_hashes()
 (WORK/'stage-progress.log').write_text('PASS: 383/420 -> 385/420; failures 37 -> 35, two original failures repaired, zero new failures; all 420 inputs and 1686 tracked fixture/reference files unchanged; prior 2609/2609 and file audit pass. PA18 remains unfinished with 35 failures; independent review pending.\n')
 checks={}
 for name,file,command,code in [
  ('stageTests','stage-final2.log','make test-pa18',2),
  ('priorThroughTests','prior-final2.log','make test-report-through-pa17',0),
  ('fileAudit','file-audit-final2.log','perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src',0),
  ('stageProgress','stage-progress.log','python3 student.tests/pa18/verify76.py',0)]:
  checks[name]=dict(command=command,exit=code,log=str(WORK/file),sha256=sha(WORK/file))
 artifacts=['pa18/plan.md','pa18/handoff76.md','pa18/performance76.md']
 artifacts+=['student.tests/pa18/'+p for p in ['inherit76_controls.py','inherit76_scaling.py','inherit76_course.py','benchmark76.py','verify76.py','report76.py','loop76-performance.json','loop76-performance-final.json']]
 controls={name:read(WORK/'validation-final'/name/'results.json') for name in GROUPS}
 for name,path in [('inherited','controls-final'),('audit74','audit74-final'),('list75','list75-final')]:controls[name]=read(WORK/path/'results.json')
 e=dict(entry_commit=git('rev-parse',ENTRY),code_commit=git('rev-parse',CODE),stage_base=BASE,last_reviewed_commit=REVIEW,
  implementation_commits=[git('rev-parse',x) for x in ['e052e939','bff709db',CODE]],
  implementation_files={p:sha(ROOT/p) for p in git('diff','--name-only',ENTRY,CODE,'--','dev').splitlines()},
  evidence_files={p:sha(ROOT/p) for p in artifacts},checks=checks,fixtures=fixture_hashes(),
  stage=dict(entry_passing=383,final_passing=385,total=420,fixed=sorted(before.keys()-after.keys()),remaining=after,new_failures=[],entry_log=str(WORK/'baseline.log'),entry_log_sha256=sha(WORK/'baseline.log')),
  controls=controls,entry_controls=read(WORK/'entry-controls2/results.json'),
  course=read(WORK/'course-final/results.json'),scaling=read(WORK/'scaling-final/results.json'),completion=read(WORK/'validation-final/completion/results.json'),
  frozen_binaries={n:dict(path=str(WORK/n),sha256=sha(WORK/n)) for n in ['cppgm-entry','cppgm-final2']},
  intermediate_logs={p.name:dict(path=str(p),sha256=sha(p)) for p in sorted(WORK.glob('*.log'))},
  handoff='Inherited constructor candidates/forwarding/defaults and consumers complete. 35 course failures remain (9 rejections, 26 LowIR mismatches). Third inherited course source executes but still fails ordinary object-root/empty-tag policy comparison. Independent review of 75–76 pending.')
 OUT.write_text(json.dumps(e,indent=2)+'\n')
e=read(OUT)
assert e['entry_commit']==git('rev-parse',ENTRY) and e['code_commit']==git('rev-parse',CODE)
for group in ['implementation_files','evidence_files']:
 for path,h in e[group].items():assert sha(ROOT/path)==h,path
assert not git('diff',CODE,'--','dev'),'Implementation changed since validated commit'
assert not git('diff','--name-only',ENTRY,'--',':(glob)pa*/tests/**',':(glob)pa*/scripts/**',':(glob)pa*/Makefile','scripts','Makefile'),'Course contract changed'
assert fixture_hashes()==e['fixtures']==read(WORK/'entry.json')['fixtures']
assert len(e['fixtures'])==1686 and sum(p.endswith('.t') for p in e['fixtures'])==420
for name,c in e['checks'].items():assert sha(c['log'])==c['sha256'],name
assert '385 / 420 TESTS PASSED' in Path(e['checks']['stageTests']['log']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2609 / 2609)' in Path(e['checks']['priorThroughTests']['log']).read_text()
assert 'File audit passed for pa18 with 3 warning(s).' in Path(e['checks']['fileAudit']['log']).read_text()
s=e['stage'];assert sha(s['entry_log'])==s['entry_log_sha256']
before,after=failures(s['entry_log']),failures(e['checks']['stageTests']['log'])
assert len(before)==37 and len(after)==35 and after==s['remaining']
assert sorted(before.keys()-after.keys())==s['fixed'] and len(s['fixed'])==2 and not after.keys()-before.keys()
assert sum('exit status mismatch' in v for v in after.values())==9
for name,count in dict(GROUPS,inherited=60,audit74=60,list75=71).items():
 rows=e['controls'][name];assert len(rows)==count and all(r['passed'] for r in rows),name
 for row in rows:assert hashlib.sha256(row['source'].encode()).hexdigest()==row['source_sha256']
assert sum(len(rows) for rows in e['controls'].values())==778
assert len(e['entry_controls'])==60 and sum(r['passed'] for r in e['entry_controls'])==17
assert len(e['course'])==3 and all(r['passed'] and r['compiler_exit']==r['backend_exit']==r['native_exit']==0 for r in e['course'])
for row in e['course']:assert sha(ROOT/'pa18/tests/general'/(row['name']+'.t'))==row['source_sha256']
assert len(e['completion'])==3 and all(r['invalidations']==1 for r in e['completion'])
assert len(e['scaling'])==12
for r in e['scaling']:
 assert hashlib.sha256(r['source'].encode()).hexdigest()==r['source_sha256']
 t=r['telemetry'][0];n=r['n'];f=r['family']
 assert t['semantic_inherited_arguments']==(0 if f=='declaration' else 1 if f=='reuse' else n)
 assert t['template_body_transitions']==(n if f=='body' else 0)
for b in e['frozen_binaries'].values():assert sha(b['path'])==b['sha256']
assert sha(ROOT/'dev/cppgm++')==e['frozen_binaries']['cppgm-final2']['sha256']
p=read(ROOT/'student.tests/pa18/loop76-performance-final.json')
assert p['finished_utc'] and len(p['workloads'])==15
assert p['harness_sha256']==sha(ROOT/'student.tests/pa18/benchmark76.py')
assert p['shared_harness_sha256']==sha(ROOT/'student.tests/pa10/benchmark.py')
assert p['commits']==[e['entry_commit'],e['code_commit']]
for i,n in enumerate(['cppgm-entry','cppgm-final2']):assert p['binaries'][i]['sha256']==e['frozen_binaries'][n]['sha256']
assert p['backend']['sha256']==sha(p['backend']['path'])
first=read(ROOT/'student.tests/pa18/loop76-performance.json')
assert first['finished_utc'] and len(first['workloads'])==15
assert first['harness_sha256']==p['harness_sha256']
for b in first['binaries']:assert sha(b['path'])==b['sha256']
assert first['commits'][1]==git('rev-parse','bff709db')
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
assert '385/420' in plan and '**35 unfinished course cases**' in plan
print('Verified implementation handoff: 383→385/420, two original repairs, no new failures, unchanged coverage; prior 2609/2609 and file audit pass; 778 semantic, three course, twelve forwarding and three completion controls; frozen performance. PA18 and independent review remain unfinished.')

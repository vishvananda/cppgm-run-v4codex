#!/usr/bin/env python3
"""Verify frozen observations, output equivalence, work scaling and course progress."""
from pathlib import Path
import hashlib,json,os,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa15-value'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
observations=0
for filename in ['performance-preliminary.json','performance.json']:
 p=json.loads((ROOT/'student.tests/pa15'/filename).read_text())
 harness=ART/'benchmark-preliminary.py' if filename=='performance-preliminary.json' else ROOT/'student.tests/pa15/benchmark.py'
 assert sha(harness)==p['harness_sha256']
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 for name,w in p['workloads'].items():
  assert sha(w['source_path'])==w['source_sha256']
  for output in w['outputs']:
   assert sha(output['path'])==output['sha256']
   if 'native' in output:
    assert sha(output['native']['path'])==output['native']['sha256']
    assert output['native']['checked_exit']==0
  if w['common_correct']:
   a,b=w['outputs'];assert a['sha256']==b['sha256']
   if 'native' in a:assert a['native']==dict(b['native'],path=a['native']['path'])
  else:assert w['baseline_rejection']['exit_code']!=0
  for phase in ['compiler','runtime']:
   if phase not in w:continue
   m=w[phase];expected=[0,0,0,0,0,1,1,0,0,1,1,0] if w['common_correct'] else [1]*6
   assert [r['binary'] for r in m['observations']]==expected
   for r in m['observations']+m['warmups']:assert r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0
   observations+=len(m['observations'])+len(m['warmups'])
  if name.startswith('values-'):
   n=int(name.split('-')[1]);d=w['outputs'][0]['telemetry'][0]
   assert d['semantic_specializations']==d['template_body_transitions']==n
   assert d['semantic_type_query_work']==3*n+5 and d['semantic_value_query_work']==3*n+1
   assert d['semantic_template_binding_work']==11
  if name.startswith('defaults-'):
   n=int(name.split('-')[1]);d=w['outputs'][0]['telemetry'][0]
   assert d['semantic_specializations']==d['template_class_completions']==n
   assert d['template_body_transitions']==0
   assert d['semantic_type_query_work']==2*n+4 and d['semantic_value_query_work']==2*n+2
 assert p.get('finished_utc')
assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
assert sha(ROOT/'student.tests/pa15/benchmark.py')==p['harness_sha256']
entry=(ART/'entry-tests.log').read_text();final=(ART/'final-stage.log').read_text()
errors=lambda text:set(re.findall(r'^(pa15/[^:]+\.t): ERROR:',text,re.M))
a,b=errors(entry),errors(final)
assert len(a)==121 and len(b)==63 and b<a
assert '114 / 177 TESTS PASSED' in final
assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in (ART/'final-prior.log').read_text()
assert 'File audit passed for pa15' in (ART/'final-file-audit.log').read_text()
assert '10 constant groups passed' in (ART/'final-personal-constants.log').read_text()
assert '26 value argument groups passed' in (ART/'final-personal-values.log').read_text()
base='8000f3c8ef4647d57f2c0775192585f14cab33d8'
trees={}
for n in range(1,16):
 path=f'pa{n}/tests';expected=git('rev-parse',f'{base}:{path}')
 assert git('rev-parse',f'HEAD:{path}')==expected
 assert not git('diff',base,'--',path)
 trees[path]=expected
assert not git('diff',base,'--','scripts','Makefile','TESTING_AND_REFERENCES.md')
proof=dict(stage_base_commit=base,implementation_commit=p['implementation_commit'],
 stage=dict(entry_passed=56,current_passed=114,total=177,original_failures=121,current_failures=63,
 resolved_original_failures=sorted(a-b),remaining_implementation_failures=sorted(b)),
 prior=dict(passed=1935,total=1935,exit_code=0),file_audit=dict(exit_code=0,inherited_warnings=3),
 personal_controls=dict(constants=10,value_arguments=26,exit_code=0),
 stage_check_exit_code=2,progress_check='58 original failures resolved; no formerly passing case fails; coverage and comparison unchanged',
 fixture_trees=trees,performance_observations_including_warmups=observations,
 binary_sha256=p['binaries'][1]['sha256'],checks={name:dict(path=str(ART/name),sha256=sha(ART/name)) for name in ['entry-tests.log','final-stage.log','final-prior.log','final-file-audit.log','final-personal-constants.log','final-personal-values.log']},
 independent_review='pending; scalar-value handoff does not close whole-stage implementation or independent audit')
(ROOT/'student.tests/pa15/handoff.json').write_text(json.dumps(proof,indent=2)+'\n')
print(f'PASS: 58 original failures resolved, 1935 prior tests, fixture preservation, {observations} performance observations')

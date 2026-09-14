#!/usr/bin/env python3
"""Verify the PA15 full-stage implementation handoff, preserving audit markers."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
WORK=Path('/tmp/pa15-loop35')
ENTRY='d705aafc4845cca2ad645b8c7ea45194936c9180'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
p=json.loads((ROOT/'student.tests/pa15/execution-performance.json').read_text())
assert not p['source_diff'] and p['finished_utc']
assert sha(ROOT/'student.tests/pa15/execution_benchmark.py')==p['harness_sha256']
assert sha(ROOT/'student.tests/pa10/benchmark.py')==p['shared_harness_sha256']
assert sha(p['backend']['path'])==p['backend']['sha256']
for b in p['binaries']:assert sha(b['path'])==b['sha256']
assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
assert git('rev-parse','HEAD:dev')==git('rev-parse',p['implementation_commit']+':dev')
assert not git('diff','HEAD','--','dev')
observations=0
for name,w in p['workloads'].items():
 assert sha(w['source_path'])==w['source_sha256']
 common=w['comparison']=='exact'
 if not common:
  probe=w['entry_probe']
  if w['comparison']=='entry-rejected':assert probe['exit_code']!=0
  else:
   assert probe['exit_code']==0 and sha(probe['path'])==probe['sha256']
   n=int(name.split('-')[1]);assert sum(s.startswith('global ') for s in Path(probe['path']).read_text().splitlines())==n
 for o in w['outputs']:
  assert sha(o['path'])==o['sha256']
  if 'native' in o:assert sha(o['native']['path'])==o['native']['sha256'] and o['native']['checked_exit']==0
 if common:
  a,b=w['outputs'];assert a['sha256']==b['sha256']
  if 'native' in a:assert a['native']['sha256']==b['native']['sha256']
 for phase in ['compiler','runtime']:
  if phase not in w:continue
  m=w[phase];expected=[0,0,0,0,0,1,1,0,0,1,1,0] if common else [1]*6
  assert [r['binary'] for r in m['observations']]==expected
  for r in m['observations']+m['warmups']:assert r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0
  observations+=len(m['observations'])+len(m['warmups'])
 b=w['outputs'][-1];t=b['telemetry'][0]
 if name.startswith('calls-'):
  n=int(name.split('-')[1]);assert t['semantic_constant_bodies']==1
  assert t['semantic_constant_activations']==n and t['semantic_constant_execution_steps']==5*n and t['semantic_constant_execution_hits']==n
 if name.startswith('receivers-'):
  n=int(name.split('-')[1]);assert t['semantic_constant_bodies']==t['semantic_constant_activations']==t['semantic_constant_execution_steps']==n
 if name.startswith('dormant-'):assert t['semantic_body_checks']==int(name.split('-')[1])
 if name.startswith('storage-'):
  n=int(name.split('-')[1]);assert t['semantic_body_checks']==0
  assert sum(s.startswith('global ') for s in Path(b['path']).read_text().splitlines())==2*n
assert observations==224
checks=json.loads((WORK/'required-checks.json').read_text())
for c in checks.values():assert c['exit_code']==0
assert 'ALL TESTS PASSED SUCCESSFULLY! (177 / 177)' in Path(checks['stageTests']['path']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in Path(checks['priorThroughTests']['path']).read_text()
assert 'File audit passed for pa15 with 3 warning(s).' in Path(checks['fileAudit']['path']).read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)' in (WORK/'through-final.log').read_text()
for c in checks.values():c['sha256']=sha(c['path'])
entry_path=WORK/'entry-stage.log'
errors=lambda text:set(re.findall(r'^(pa15/[^:]+\.t): ERROR:',text,re.M))
entry_errors=errors(entry_path.read_text());assert len(entry_errors)==4
assert not errors(Path(checks['stageTests']['path']).read_text())
personal={
 'execution':'24 native and 18 rejection controls passed',
 'value_arguments':'26 value argument groups passed',
 'constants':'10 constant groups passed',
 'specializations':'24 native controls; 13 rejection controls passed',
 'packs':'27 native controls; 8 rejections passed',
 'class_patterns':'15 native and 7 rejection class-pattern controls passed',
 'initialization':'23 native and 13 rejection controls passed',
 'checkpoint_audit':'11 native and 7 rejection audit controls passed',
}
personal_evidence={}
for name,expected in personal.items():
 path=WORK/f'personal-{name}.log';assert expected in path.read_text()
 if name=='initialization':assert '3 global-address LowIR controls passed' in path.read_text()
 personal_evidence[name]=dict(result=expected,path=str(path),sha256=sha(path),script_sha256=sha(ROOT/f'student.tests/pa15/{name}.py'))
trees={}
for n in range(1,16):
 path=f'pa{n}/tests';trees[path]=git('rev-parse',ENTRY+':'+path)
 assert git('rev-parse','HEAD:'+path)==trees[path] and not git('diff',ENTRY,'--',path)
preserved=['scripts','Makefile','TESTING_AND_REFERENCES.md','spec.md','pa15/README.md','pa15/audit.md','pa15/reference-corrections.md','reference-binaries']
assert not git('diff',ENTRY,'--',*preserved)
plan=(ROOT/'pa15/plan.md').read_text()
markers={label:re.search(label+r': `([^`]+)`',plan).group(1) for label in ['Stage base commit','Last reviewed commit']}
old=git('show',ENTRY+':pa15/plan.md')
for label,value in markers.items():assert f'{label}: `{value}`' in old
assert 'independent' in plan.lower() and '177/177' in plan and 'no known unfinished behavior group' in plan
proof=dict(entry_commit=ENTRY,implementation_commit=p['implementation_commit'],review_markers=markers,
 entry_stage_log=dict(path=str(entry_path),sha256=sha(entry_path)),
 stage=dict(entry_passed=173,current_passed=177,total=177,entry_failures=4,current_failures=0,resolved_original_failures=sorted(entry_errors)),
 stageProgress=dict(exit_code=0,reason='All four original failures resolved; no coverage or comparison changes; earlier stages pass'),
 prior=dict(passed=1935,total=1935,exit_code=0),through=dict(passed=2112,total=2112,exit_code=0,path=str(WORK/'through-final.log'),sha256=sha(WORK/'through-final.log')),
 file_audit=dict(exit_code=0,inherited_warnings=3),required_checks=checks,personal_controls=personal_evidence,
 fixture_trees=trees,performance_observations_including_warmups=observations,performance_sha256=sha(ROOT/'student.tests/pa15/execution-performance.json'),
 source_tree=git('rev-parse','HEAD:dev'),binary_sha256=p['binaries'][1]['sha256'],
 remaining_required_implementation=[],independent_review='Full-stage audit pending for all changes after 538cfcb00441f57c0629f6d27fddbad723539479; markers preserved; implementation handoff does not authorize advancement')
(ROOT/'student.tests/pa15/execution-handoff.json').write_text(json.dumps(proof,indent=2)+'\n')
print('PASS: 4 original failures resolved; PA15 177/177; prior 1935/1935; through 2112/2112; file audit; fixture preservation; 224 frozen observations')

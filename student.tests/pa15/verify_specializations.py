#!/usr/bin/env python3
"""Verify exact artifacts, original-failure reduction, fixture identity and work."""
from pathlib import Path
import hashlib,json,os,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa15-specialization'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
observations=0
for filename in ['specialization-performance-preliminary.json','specialization-performance-noise.json','specialization-performance.json']:
 p=json.loads((ROOT/'student.tests/pa15'/filename).read_text())
 assert 'finished_utc' in p,filename
 assert sha(ROOT/'student.tests/pa15/specialization_benchmark.py')==p['harness_sha256']
 assert sha(ROOT/'student.tests/pa10/benchmark.py')==p['shared_harness_sha256']
 for b in p['binaries']:assert sha(b['path'])==b['sha256']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 for name,w in p['workloads'].items():
  assert sha(w['source_path'])==w['source_sha256']
  for out in w['outputs']:
   assert sha(out['path'])==out['sha256']
   if 'native' in out:assert sha(out['native']['path'])==out['native']['sha256']
  if w['common_correct']:
   assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256']
   if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256']
  else:
   control=w['baseline_control'];assert control['compile_exit'] or control['native_exit']
  for kind in ['compiler','runtime']:
   if kind not in w:continue
   data=w[kind];rows=data['observations'];assert all(r['checked_exit']==0 for r in rows+data['warmups'])
   assert [r['binary'] for r in rows]==([0]*4+[0,1,1,0]*2 if w['common_correct'] else [1]*6)
   observations+=len(rows)+len(data['warmups'])
  if name.startswith('specialized-classes-') or name.startswith('variable-queries-'):
   n=int(name.rsplit('-',1)[1]);stats=w['outputs'][0]['telemetry'][0]
   assert stats['semantic_specializations']==n
   if name.startswith('specialized-classes-'):
    assert stats['explicit_specialization_selections']==n and stats['template_class_completions']==0
   else:assert stats['variable_template_initializers']==n and stats['variable_template_reuses']==2*n
assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
entry=(ART/'entry-stage.log').read_text();final=(ART/'final-stage.log').read_text()
a=set(re.findall(r'^(pa15/tests/.*?\.t): ERROR',entry,re.M));b=set(re.findall(r'^(pa15/tests/.*?\.t): ERROR',final,re.M))
assert len(a)==63 and len(b)==49 and b<a
assert '128 / 177 TESTS PASSED' in final
assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in (ART/'final-prior.log').read_text()
assert 'File audit passed for pa15' in (ART/'final-file-audit.log').read_text()
assert '24 native controls; 13 rejection controls passed' in (ART/'final-personal-specializations.log').read_text()
assert '26 value argument groups passed' in (ART/'final-personal-values.log').read_text()
assert '10 constant groups passed' in (ART/'final-personal-constants.log').read_text()
base='8000f3c8ef4647d57f2c0775192585f14cab33d8';entry_commit='d3475a79395f05b05873e940531726bcbbe8fc7a'
plan=(ROOT/'pa15/plan.md').read_text()
assert f'Stage base commit: `{base}`' in plan and f'Last reviewed commit: `{base}`' in plan
trees={}
for n in range(1,16):
 path=f'pa{n}/tests';expected=git('rev-parse',f'{base}:{path}')
 assert git('rev-parse',f'HEAD:{path}')==expected and not git('diff',base,'--',path)
 trees[path]=expected
assert not git('diff',entry_commit,'--','scripts','Makefile','reference-binaries','TESTING_AND_REFERENCES.md','spec.md','pa15/README.md')
checks=['entry-stage.log','final-stage.log','final-prior.log','final-file-audit.log','final-personal-specializations.log','final-personal-values.log','final-personal-constants.log']
proof=dict(stage_base_commit=base,last_reviewed_commit=base,entry_commit=entry_commit,implementation_commit=p['implementation_commit'],
 stage=dict(entry_passed=114,current_passed=128,total=177,entry_failures=63,current_failures=49,resolved_original_failures=sorted(a-b),remaining_implementation_failures=sorted(b)),
 prior=dict(passed=1935,total=1935,exit_code=0),file_audit=dict(exit_code=0,inherited_warnings=3),stage_check_exit_code=2,
 progress_check='14 entry failures resolved; no regressions; unchanged coverage and comparison',
 personal_controls=dict(native=24,rejections=13,values=26,constants=10),fixture_trees=trees,
 performance_observations_including_warmups=observations,binary_sha256=p['binaries'][1]['sha256'],
 checks={name:dict(path=str(ART/name),sha256=sha(ART/name)) for name in checks},
 handoff_boundary='Canonical explicit selection and constant variable query/storage facts complete; packs, broader substitution, execution and body-obligation/initialization owners remain implementation work.',
 independent_review='pending: whole-stage correctness, architecture and performance; original markers unchanged')
(ROOT/'student.tests/pa15/specialization-handoff.json').write_text(json.dumps(proof,indent=2)+'\n')
print(f'PASS: 14 original failures resolved; 1935 prior tests; fixture preservation; {observations} frozen observations')

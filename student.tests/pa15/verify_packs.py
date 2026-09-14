#!/usr/bin/env python3
"""Verify retained performance evidence, original failures and fixture identity."""
from pathlib import Path
import hashlib,json,os,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa15-packs'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
observations=0;campaigns=[]
for filename in ['performance.json','performance-final.json']:
 path=ART/filename;p=json.loads(path.read_text())
 assert 'finished_utc' in p,filename
 assert sha(ROOT/'student.tests/pa15/pack_benchmark.py')==p['harness_sha256']
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
  else:assert w['entry_probe']['exit']!=0
  for kind in ['compiler','runtime']:
   if kind not in w:continue
   d=w[kind];rows=d['observations']
   assert all(r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0 for r in rows+d['warmups'])
   assert [r['binary'] for r in rows]==([0]*4+[0,1,1,0]*2 if w['common_correct'] else [1]*6)
   observations+=len(rows)+len(d['warmups'])
  if name.startswith(('pack-signatures-','nested-packs-')):
   n=int(name.rsplit('-',1)[1]);s=w['outputs'][0]['telemetry'][0]
   nested=name.startswith('nested')
   assert s['semantic_pack_expansion_work']==(18 if nested else 1)
   assert s['semantic_pack_expansion_lanes']==(7 if nested else 3)*n
   assert s['template_body_transitions']==(3 if nested else 1)*n
   assert s['template_source_regions']==(9 if nested else 3)
   assert s['semantic_substitution_frames']==(13*n+11 if nested else 9*n)
 campaigns.append(dict(path=str(path),sha256=sha(path),implementation_commit=p['implementation_commit']))
assert sha(ROOT/'dev/cppgm++')==p['binaries'][1]['sha256']
entry=(ART/'entry-stage.log').read_text();final=(ART/'final-stage.log').read_text()
a=set(re.findall(r'^(pa15/tests/.*?\.t): ERROR',entry,re.M));b=set(re.findall(r'^(pa15/tests/.*?\.t): ERROR',final,re.M))
assert len(a)==49 and len(b)==11 and b<a
assert '166 / 177 TESTS PASSED' in final
assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in (ART/'final-prior.log').read_text()
assert 'File audit passed for pa15 with 3 warning(s)' in (ART/'final-audit.log').read_text()
assert '27 native controls; 8 rejections passed' in (ART/'final-personal.log').read_text()
assert '24 native controls; 13 rejection controls passed' in (ART/'final-specializations.log').read_text()
assert '26 value argument groups passed' in (ART/'final-values.log').read_text()
assert '10 constant groups passed' in (ART/'final-constants.log').read_text()
base='8000f3c8ef4647d57f2c0775192585f14cab33d8';entry_commit='19225b3deef4a85e5690c2d72fb7857f5a06721c'
plan=(ROOT/'pa15/plan.md').read_text()
assert f'Stage base commit: `{base}`' in plan and f'Last reviewed commit: `{base}`' in plan
trees={}
for n in range(1,16):
 path=f'pa{n}/tests';expected=git('rev-parse',f'{base}:{path}')
 assert git('rev-parse',f'HEAD:{path}')==expected and not git('diff',base,'--',path)
 trees[path]=expected
assert not git('diff',entry_commit,'--','scripts','Makefile','reference-binaries','TESTING_AND_REFERENCES.md','spec.md','pa15/README.md')
changed=git('diff','--name-only',entry_commit).splitlines()
assert all(f.startswith(('dev/','student.tests/pa15/')) or f=='pa15/plan.md' for f in changed),changed
checks=['entry-stage.log','final-stage.log','final-prior.log','final-audit.log','final-personal.log','final-specializations.log','final-values.log','final-constants.log']
proof=dict(stage_base_commit=base,last_reviewed_commit=base,entry_commit=entry_commit,implementation_commit=p['implementation_commit'],
 stage=dict(entry_passed=128,current_passed=166,total=177,entry_failures=49,current_failures=11,resolved_original_failures=sorted(a-b),remaining_implementation_failures=sorted(b)),
 prior=dict(passed=1935,total=1935,exit_code=0),file_audit=dict(exit_code=0,inherited_warnings=3),stage_check_exit_code=2,
 progress_check='38 original failures resolved; no regressions; unchanged coverage and comparison rules',
 personal_controls=dict(packs_native=27,packs_rejections=8,specializations_native=24,specializations_rejections=13,values=26,constants=10),fixture_trees=trees,
 performance_campaigns=campaigns,performance_observations_including_warmups=observations,binary_sha256=p['binaries'][1]['sha256'],
 checks={name:dict(path=str(ART/name),sha256=sha(ART/name)) for name in checks},
 handoff_boundary='Canonical packs, source-list expansion and literal-call construction complete across deduction, calls, braces, bases, constructor/destructor consumers and default queries. Remaining class-partial matching, constant-object execution/storage and ordinary body validation require different semantic owners; they remain implementation requirements.',
 independent_review='pending: whole-stage correctness, architecture, complete keys, sharing, demand boundaries and performance; original markers unchanged')
(ROOT/'student.tests/pa15/pack-handoff.json').write_text(json.dumps(proof,indent=2)+'\n')
print(f'PASS: 38 original failures resolved; 1935 prior tests; unchanged fixtures; {observations} frozen observations')

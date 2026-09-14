#!/usr/bin/env python3
"""Verify cumulative evidence without rewriting the three historical handoffs."""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ART=Path(os.environ['RALPH_ARTIFACT_DIR'])
AUDIT=ART/'pa15-audit'
BASE='8000f3c8ef4647d57f2c0775192585f14cab33d8'
ENTRY='db0686a3b17c87aa9d8dbd82e30346c034fc89b2'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
paths=[ROOT/'student.tests/pa15'/n for n in ['performance-preliminary.json','performance.json','specialization-performance-preliminary.json','specialization-performance-noise.json','specialization-performance.json']]
paths += [ART/'pa15-packs'/n for n in ['performance.json','performance-final.json']]
paths += [AUDIT/n for n in ['stage-performance.json','checkpoint-performance.json']]
# Repeats are additive observations, never replacements for noisy campaigns.
paths += sorted(AUDIT.glob('*-repeat-performance.json'))
observations=0;campaigns=[]
for path in paths:
 p=json.loads(path.read_text());assert p.get('finished_utc'),path
 if path.name=='performance-preliminary.json':harness=ART/'pa15-value/benchmark-preliminary.py'
 elif path.parent.name=='pa15-packs':harness=ROOT/'student.tests/pa15/pack_benchmark.py'
 elif path.parent==AUDIT:harness=ROOT/'student.tests/pa15/audit_benchmark.py'
 elif path.name.startswith('specialization-'):harness=ROOT/'student.tests/pa15/specialization_benchmark.py'
 else:harness=ROOT/'student.tests/pa15/benchmark.py'
 assert sha(harness)==p['harness_sha256'],harness
 if 'shared_harness_sha256' in p:assert sha(ROOT/'student.tests/pa10/benchmark.py')==p['shared_harness_sha256']
 for b in p['binaries']:assert sha(b['path'])==b['sha256'],b['path']
 assert sha(p['backend']['path'])==p['backend']['sha256']
 count=0
 for name,w in p['workloads'].items():
  assert sha(w['source_path'])==w['source_sha256'],w['source_path']
  for out in w['outputs']:
   assert sha(out['path'])==out['sha256'],out['path']
   if 'native' in out:
    assert sha(out['native']['path'])==out['native']['sha256']
    assert out['native']['checked_exit']==0
  if w['common_correct']:
   assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256']
   if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256']
  else:
   if 'baseline_rejection' in w:assert w['baseline_rejection']['exit_code']!=0
   elif 'entry_probe' in w:assert w['entry_probe']['exit']!=0
   else:
    control=w['baseline_control'];assert control['compile_exit'] or control['native_exit']
  for phase in ['compiler','runtime']:
   if phase not in w:continue
   d=w[phase];rows=d['observations']
   assert [r['binary'] for r in rows]==([0]*4+[0,1,1,0]*2 if w['common_correct'] else [1]*6)
   for r in rows+d['warmups']:assert r['checked_exit']==0 and r['wall_s']>0 and r['rss_kib']>0
   count+=len(rows)+len(d['warmups'])
 observations+=count;campaigns.append(dict(path=str(path),sha256=sha(path),invocations=count))
if '--measurements-only' in sys.argv:
 print(f'PASS: {len(campaigns)} campaigns, {observations} verified frozen invocations');sys.exit(0)
final=json.loads((AUDIT/'checkpoint-repeat-performance.json').read_text())
TIP=git('log','-1','--format=%H','--','dev','student.tests/pa15/*.py','student.tests/pa15/*.cpp')
assert not final['source_diff']
assert git('rev-parse',final['implementation_commit']+':dev')==git('rev-parse',TIP+':dev')
assert git('rev-parse',TIP+':dev')==git('rev-parse','HEAD:dev') and not git('diff',TIP,'--','dev')
assert sha(ROOT/'dev/cppgm++')==final['binaries'][1]['sha256']
for filename in ['plan.md','audit.md']:
 assert f'Last reviewed commit: `{TIP}`' in (ROOT/'pa15'/filename).read_text()
assert f'Stage base commit: `{BASE}`' in (ROOT/'pa15/plan.md').read_text()
# Scope is every accumulated commit and source delta, not the last handoff.
commits=git('rev-list','--reverse',f'{BASE}..{TIP}').splitlines()
assert len(git('rev-list',f'{BASE}..{ENTRY}').splitlines())==13
fixture_trees={}
for n in range(1,16):
 path=f'pa{n}/tests';expected=git('rev-parse',f'{BASE}:{path}')
 assert git('rev-parse',f'HEAD:{path}')==expected and not git('diff',BASE,'--',path)
 fixture_trees[path]=expected
assert not git('diff',BASE,'--','scripts','Makefile','TESTING_AND_REFERENCES.md','reference-binaries','NOTICE','spec.md','pa15/README.md')
errors=lambda p:set(re.findall(r'^(pa15/[^:]+\.t): ERROR:',p.read_text(),re.M))
a,b=errors(AUDIT/'entry-stage.log'),errors(AUDIT/'final-stage.log')
assert len(a)==11 and b==a
assert '166 / 177 TESTS PASSED' in (AUDIT/'final-stage.log').read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in (AUDIT/'final-prior.log').read_text()
assert 'File audit passed for pa15 with 3 warning(s)' in (AUDIT/'final-file-audit.log').read_text()
for filename,message in {
 'final-personal.log':'11 native and 7 rejection audit controls passed',
 'final-packs.log':'27 native controls; 8 rejections passed',
 'final-specializations.log':'24 native controls; 13 rejection controls passed',
 'final-values.log':'26 value argument groups passed',
 'final-constants.log':'10 constant groups passed',
}.items():assert message in (AUDIT/filename).read_text()
assert (AUDIT/'final-index.log').read_text()=='PASS: binding removals preserve all keys and empty state\n'
# Current once-per-owner measurements at both sizes; a fourfold increase in
# demands must scale the semantic transitions exactly, independent of timings.
for name,w in final['workloads'].items():
 if not re.search(r'-(1000|4000)$',name):continue
 n=int(name.rsplit('-',1)[1]);stats=w['outputs'][-1]['telemetry'][0]
 if name.startswith('nested-packs-'):
  assert stats['semantic_pack_expansion_work']==18 and stats['semantic_pack_expansion_lanes']==7*n
  assert stats['template_body_transitions']==3*n and stats['template_source_regions']==9
  assert stats['semantic_substitution_frames']==13*n+11
 elif name.startswith('defaults-'):
  assert stats['template_class_completions']==n and stats['template_body_transitions']==0
 elif name.startswith('values-'):
  assert stats['semantic_specializations']==stats['template_body_transitions']==n
 elif name.startswith('selected-classes-'):
  assert stats['explicit_specialization_selections']==n and stats['template_class_completions']==0
 elif name.startswith('pack-targets-'):
  assert stats['template_body_transitions']==n
  assert stats['semantic_pack_expansion_lanes']==2*n
 elif name.startswith('pack-selections-'):
  assert stats['explicit_specialization_selections']==n and stats['template_body_transitions']==0
checks={p.name:dict(path=str(p),sha256=sha(p)) for p in sorted(AUDIT.glob('final-*.log'))}
proof=dict(stage_base_commit=BASE,audit_entry_commit=ENTRY,last_reviewed_commit=TIP,
 reviewed_commits=[dict(commit=c,subject=git('show','-s','--format=%s',c)) for c in commits],
 reviewed_source_files=git('diff','--name-only',BASE,TIP,'--','dev').splitlines(),
 measured_implementation_commit=final['implementation_commit'],code_tree=git('rev-parse',TIP+':dev'),binary_sha256=sha(ROOT/'dev/cppgm++'),
 stage=dict(passed=166,total=177,entry_failures=11,current_failures=11,failures=sorted(b),exit_code=2),
 prior=dict(passed=1935,total=1935,exit_code=0),file_audit=dict(exit_code=0,inherited_warnings=3),
 fixture_trees=fixture_trees,checks=checks,performance_campaigns=campaigns,
 performance_invocations_including_warmups=observations,reference_changes=False,
 status='checkpoint audit complete; full-stage implementation still has the same 11 failures')
(ROOT/'student.tests/pa15/checkpoint-evidence.json').write_text(json.dumps(proof,indent=2)+'\n')
print(f'PASS: {len(commits)} reviewed commits; identical 11 failures/177 cases; 1935 earlier tests; {observations} frozen invocations; unchanged references')

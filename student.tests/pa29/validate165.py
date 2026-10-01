#!/usr/bin/env python3
"""Verify PA29 implementation165 handoff commands, progress and preserved coverage."""
import collections,hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();ev=root/'student.tests/pa29/evidence165';ev.mkdir(exist_ok=True)
entry='61e023f39a4c1bafdccc484257ab51df98f87088'
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def save(name,value):(ev/name).write_text(json.dumps(value,indent=2)+'\n')
def failures(path):return sorted(set(re.findall(r'(pa29/tests/[^\s:]+\.t): ERROR:',path.read_text())))
before=failures(out/'baseline.log');after=failures(out/'stage-required.log')
assert len(before)==54 and len(after)==53 and set(after)<set(before)
assert failures(out/'through-final.log')==after
assert '4538 / 4538' in (out/'prior-final.log').read_text()
assert '350 / 403' in (out/'stage-required.log').read_text()
assert '4888 / 4941' in (out/'through-final.log').read_text()
assert 'File audit passed for pa29' in (out/'file-audit-final.log').read_text()
fixtures={}
for name in git('ls-files','pa29/tests').decode().splitlines():
 data=(root/name).read_bytes();assert data==git('show',entry+':'+name);fixtures[name]=sha(data)
assert sum(p.endswith('.t') for p in fixtures)==403
assert not git('diff',entry,'--','pa29/tests','pa29/scripts','scripts','Makefile','pa29/Makefile')
save('coverage.json',dict(source_count=403,all_fixtures_and_sidecars_unchanged=True,fixture_sha256=fixtures,comparison_and_discovery_changes=[],reference_corrections=[]))
code_tip=json.loads((ev/'performance-manifest.json').read_text())['code_tip']
final_sha=json.loads((ev/'performance-manifest.json').read_text())['binaries']['B']['sha256']
assert not git('diff',code_tip,'--','dev')
save('stage-delta.json',dict(entry=entry,code_tip=code_tip,total=403,before_pass=349,after_pass=350,entry_failures=before,final_failures=after,new_failures=[],resolved=sorted(set(before)-set(after))))
checks={}
for command,log,status in [('make test-pa29','stage-required.log',2),('make test-report-through-pa28','prior-final.log',0),('make test-report-through-pa29','through-final.log',2),('perl scripts/cppgm_file_audit.pl --stage pa29 --paths dev/src','file-audit-final.log',0)]:
 checks[command]=dict(exit_status=status,log=str(out/log),log_sha256=sha((out/log).read_bytes()))
for script,folder,name,source in [('controls165.py','controls-final','controls.json','results.json'),('inspection165.py','inspection-final','inspection.json','inspection.json'),('controls164.py','inherited164','inherited164.json','results.json')]:
 data=json.loads((out/folder/source).read_text());assert all(r['passed'] for r in data['checks']);assert data['compiler_sha256']==final_sha;save(name,data)
 checks['python3 student.tests/pa29/'+script+' '+str(out/folder)]=dict(exit_status=0,checks=len(data['checks']))
prior=json.loads((root/'student.tests/pa29/evidence164/remaining.json').read_text())
rows=[r for r in prior['failures'] if r['test'] in after]
assert len(rows)==53
save('remaining.json',dict(entry=entry,stage='pa29',passed=350,total=403,counts=dict(collections.Counter(r['owner'] for r in rows)),failures=rows,additional_unfinished=[dict(owner='source-invocation context',test='pa29/tests/run/800-source-location-builtin-macros-run.t',reducer='student.tests/pa29/pending165/source-invocation.cpp',disposition='Unfinished implementation: lexical versus nested default/member-initializer invocation coordinates need separate typed facts'),dict(owner='constexpr aggregate mutation overlay',reducer='student.tests/pa29/pending165/aggregate-mutation.cpp',disposition='Unfinished inherited implementation: positive C++14 behavior rejected by both entry and final; never an expected-rejection contract')],resolved_reducers=['student.tests/pa29/pending164/evaluation-context.cpp']))
manifest=json.loads((ev/'performance-manifest.json').read_text())
assert sha((root/'dev/cppgm++').read_bytes())==manifest['binaries']['B']['sha256']
for name,folder in [('common-performance.json','common-final'),('affected-performance.json','affected-final')]:
 data=json.loads((out/folder/'performance.json').read_text());save(name,data)
 assert all(r['status']==0 for r in data['runs'])
 assert (data['binaries']['B']['sha256'] if 'binaries' in data else data['binary']['sha256'])==final_sha
 if 'binaries' in data:
  assert len(data['runs'])==224
  for image in data['images'].values():
   assert image['A']['object_sha256']==image['B']['object_sha256']
   assert image['A']['executable_sha256']==image['B']['executable_sha256']
 else:
  assert len(data['runs'])==96
  for row in data['runs']:
   if row['mode']!='compile':continue
   storage=row['workload'].startswith('storage');n=int(re.search(r'\d+$',row['workload']).group())
   counters=row['phase_counters'][0]
   for key,value in dict(semantic_evaluation_mode_observations=4*n if storage else n,semantic_evaluation_mode_values=2*n if storage else n,semantic_evaluation_mode_activations=3*n+2 if storage else n+1,semantic_context_initializers=3*n if storage else n,semantic_context_references=n if storage else 0).items():
    assert counters[key]==value
save('validation.json',dict(entry=entry,code_tip=code_tip,compiler_sha256=sha((root/'dev/cppgm++').read_bytes()),checks=checks,stage_progress='PASS: one existing failure removed; no new failures or coverage change',prior_passed=4538,stage_passed=350,stage_total=403,independent_review='pending; review markers preserved',whole_stage='unfinished implementation and independent audit remain',discarded_interim_reports='Concurrent first reports shared root .test_counts; authoritative reports were rerun serially. No final coverage or count is derived from those interim logs.'))
print('PA29 implementation165 validation evidence: PASS',flush=True)

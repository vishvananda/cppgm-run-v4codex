#!/usr/bin/env python3
"""Verify this implementation handoff, including the incomplete-stage boundary."""
from pathlib import Path
import hashlib,json,posixpath,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def checked(item):
 p=Path(item['path']);p=p if p.is_absolute() else ROOT/p
 assert sha(p)==item['sha256'],str(p)
 return p
report=json.loads((ROOT/'student.tests/pa16/validity-handoff.json').read_text())
for entry in report['source_files']+report['evidence_files']:checked(entry)
assert sha(ROOT/'dev/cppgm++')==report['compiler_sha256']
contract=['spec.md','TESTING_AND_REFERENCES.md','Makefile','scripts','reference-binaries']
for n in range(1,17):contract += [f'pa{n}/tests',f'pa{n}/scripts',f'pa{n}/Makefile',f'pa{n}/README.md',f'pa{n}/pa{n}.gram']
subprocess.run(['git','diff','--exit-code',report['entry_commit'],'--',*contract],cwd=ROOT,check=True)
assert len(list((ROOT/'pa16/tests').rglob('*.t')))==154
logs={name:checked(item) for name,item in report['logs'].items()}
failures=lambda path:set(re.findall(r'(pa16/tests/[^:]+): ERROR:',path.read_text()))
before,after=failures(logs['baseline']),failures(logs['stage'])
assert len(before)==76 and len(after)==61 and not after-before
assert sorted(before-after)==report['fixed_tests']
assert sorted(after)==report['remaining_tests']
assert 'TEST SUMMARY: 93 / 154 TESTS PASSED' in logs['stage'].read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (2112 / 2112)' in logs['prior'].read_text()
assert 'TEST SUMMARY: 2205 / 2266 TESTS PASSED' in logs['through'].read_text()
assert failures(logs['through'])==after
assert not re.search(r'pa(?:[1-9]|1[0-5])/tests/[^:]+: ERROR:',logs['through'].read_text())
assert 'File audit passed for pa16 with 3 warning(s)' in logs['audit'].read_text()
for name,summary in {
 'validity':"24 native, 29 rejection controls; failures: []",
 'exceptions':"41 native, 8 rejection controls; failures: []",
 'scalar':"21 native and 10 rejection controls passed",
 'floating':"19 native and 13 rejection controls passed",
 'storage':"27 native storage controls passed",
}.items():assert summary in logs[name].read_text(),name

observations=0
names={f'{g}-{n}' for g in ['template','memory-float','literal-classes','exception-defaults','exception-dependent'] for n in [1000,4000]}
names|={f'runtime-{g}' for g in ['calls','memory','floating','literal-class']}
for entry in report['campaigns']:
 campaign=json.loads(checked(entry).read_text())
 assert campaign['finished_utc'] and set(campaign['workloads'])==names
 for binary in campaign['binaries']:checked(binary)
 assert campaign['binaries'][1]['sha256']==report['compiler_sha256']
 assert campaign['implementation_commit']==report['implementation_commit'] and not campaign['source_diff']
 assert sha(ROOT/'student.tests/pa16/validity_benchmark.py')==campaign['harness_sha256']
 assert sha(ROOT/'student.tests/pa10/benchmark.py')==campaign['shared_harness_sha256']
 checked(campaign['backend'])
 for name,workload in campaign['workloads'].items():
  assert sha(workload['source_path'])==workload['source_sha256']
  for output in workload['outputs']:
   checked(output)
   if 'native' in output:checked(output['native']);assert output['native']['checked_exit']==0
  common=workload['comparison']=='exact'
  if common:
   assert len({o['sha256'] for o in workload['outputs']})==1
   if 'runtime' in workload:assert len({o['native']['sha256'] for o in workload['outputs']})==1
  else:assert workload['entry_probe']['exit_code']!=0
  for phase in ['compiler','runtime']:
   if phase not in workload:continue
   samples=workload[phase]
   assert [r['binary'] for r in samples['warmups']]==([0,1] if common else [1])
   assert [r['binary'] for r in samples['observations']]==([0,0,0,0,0,1,1,0,0,1,1,0] if common else [1]*6)
   for row in samples['warmups']+samples['observations']:
    assert row['wall_s']>0 and row['rss_kib']>0 and row['checked_exit']==0
    observations+=1
 for n in [1000,4000]:
  def telemetry(group):return {k:v for phase in campaign['workloads'][f'{group}-{n}']['outputs'][-1]['telemetry'] for k,v in phase.items()}
  assert telemetry('literal-classes')['constexpr_validity_work']==6*n
  assert telemetry('exception-defaults')['exception_work']==5*n
  values=telemetry('exception-dependent')
  assert values['exception_work']==7*n and values['template_body_transitions']==0 and values['semantic_body_checks']==0
assert observations==476

# Historical manifests describe earlier implementations. Check their source
# against that commit, while keeping their raw artifacts and counters intact.
historical=json.loads(checked(report['historical_manifest']).read_text())
def historical_source(commit,path):
 # The manifests hash file contents, following tracked wrapper symlinks. Git
 # stores the link spelling as its blob; resolve it within the historical tree.
 while True:
  tree=subprocess.check_output(['git','ls-tree',commit,'--',path],cwd=ROOT)
  data=subprocess.check_output(['git','show',commit+':'+path],cwd=ROOT)
  if not tree.startswith(b'120000 '):return data
  path=posixpath.normpath(posixpath.join(posixpath.dirname(path),data.decode()))
for entry in historical['source_files']:
 data=historical_source(report['entry_commit'],entry['path'])
 assert hashlib.sha256(data).hexdigest()==entry['sha256'],entry['path']
for entry in list(historical['logs'].values())+historical['retained_evidence']+historical['campaigns']+[historical['noise'],historical['historical_manifest']]:checked(entry)
for record in historical['campaigns']:
 campaign=json.loads(checked(record).read_text())
 for entry in campaign['binaries']:checked(entry)
 checked(campaign['backend'])
 for work in campaign['workloads'].values():
  assert sha(work['source_path'])==work['source_sha256']
  for output in work['outputs']:
   checked(output)
   if 'native' in output:checked(output['native'])
plan=(ROOT/'pa16/plan.md').read_text()
assert f"Stage base commit: `{report['stage_base']}`" in plan
assert f"Last reviewed commit: `{report['last_reviewed_commit']}`" in plan
assert 'Remaining implementation' in plan and 'independent review' in plan.lower()
assert 'Initializer and declaration completion' in plan
print('PASS: 15 entry failures fixed, no lost passes; 154 unchanged fixtures; prior 2112/2112; '
 'through 2205/2266; file audit; 132 native / 60 rejection controls; 476 frozen performance observations; '
 'source/binary provenance and historical evidence; implementation/review boundary preserved')

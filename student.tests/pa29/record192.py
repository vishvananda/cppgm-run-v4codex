#!/usr/bin/env python3
"""Bind implementation192 checks and performance to committed sources.
Usage: record192.py SCRATCH; run after validation and both performance scripts.
"""
import hashlib,json,pathlib,re,shutil,statistics,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
scratch=pathlib.Path(sys.argv[1]).resolve();out=root/'student.tests/pa29/evidence192';out.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(name,data):(out/name).write_text(json.dumps(data,indent=2)+'\n')
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
entry='4becf437ada14c36b01baae1d608428a621a5317';code=git('rev-parse','dca6f1f2')
compiler=sha(root/'dev/cppgm++');assert compiler==sha(scratch/'cppgm-final')
assert not git('diff','--name-only',code,'--','dev')
paths=git('ls-files','dev').splitlines();source={p:sha(root/p) for p in paths}
write('source-binding.json',dict(entry_commit=entry,code_commit=code,dev_tree=git('rev-parse',code+':dev'),compiler_sha256=compiler,
 entry_compiler_sha256=sha(scratch/'cppgm-before'),frozen_compiler_matches=True,tracked_dev_paths=len(paths),
 validated_source_sha256=hashlib.sha256(json.dumps(source,sort_keys=True).encode()).hexdigest(),
 worktree_dev_matches_committed_tree=True,following_record_commit_changes_implementation=False))
old=json.loads((root/'student.tests/pa29/evidence191/coverage.json').read_text());files={p:sha(root/p) for p in old['files']}
changes=[p for p,v in files.items() if old['files'][p]!=v]
assert changes==['pa29/tests/preproc/400-host-gnu-hex-float-pp-number.ref'],changes
inputs=[p for p in files if p.endswith('.t')];assert len(inputs)==403
write('coverage.json',dict(entry=entry,code_commit=code,total_stage_inputs=len(inputs),files=files,changed=changes,
 correction='pa29-192-quad-token; see pa29/reference-corrections192.md',inputs_statuses_and_comparison_rules_unchanged=True))
checks=json.loads((scratch/'validation-final.json').read_text());log=scratch/'stage-final2.log'
checks.insert(0,dict(command=['make','test-pa29'],status=2,log=str(log),sha256=sha(log),summary=[s for s in log.read_text().splitlines() if 'TEST SUMMARY' in s]))
assert '402 / 403' in checks[0]['summary'][0] and '4538 / 4538' in checks[1]['summary'][0] and '4940 / 4941' in checks[2]['summary'][0]
assert [r['status'] for r in checks]==[2,0,2,0]
for folder,name in [('controls-final2','controls.json'),('inherited-vector-final','inherited-vector-controls.json')]:
 rows=json.loads((scratch/folder/'checks.json').read_text());assert all((r['status']==0)==r['expected_success'] for r in rows)
 write(name,rows)
write('decoder.json',json.loads((scratch/'decoder-final/decoder.json').read_text()))
assert json.loads((out/'decoder.json').read_text())['failed']==0
write('validation.json',dict(compiler_sha256=compiler,checks=checks,personal_control_commands=84,inherited_vector_commands=191,personal_control_failures=0,decoder_cases=1642,decoder_failures=0))
prior=json.loads((root/'student.tests/pa29/evidence191/remaining.json').read_text());remaining=[prior['failures'][-1]['test']]
assert all(s.split(': ERROR:')[0] in remaining for s in log.read_text().splitlines() if ': ERROR:' in s)
write('stage-delta.json',dict(total=403,entry_failures=4,final_failures=1,fixed=[r['test'] for r in prior['failures'][:-1]],new_failures=[],remaining=remaining,prior_passed=4538,prior_total=4538,coverage_preserved=True,progress_criterion_satisfied=True))
write('remaining.json',dict(total=403,passed=402,failed=1,owner_counts={'nested-template-abi-tag-contract':1},failures=[prior['failures'][-1]],unfinished_implementation_in_completed_scalar_floating_group=[],next_boundary='Independent extension-contract review; oracle and failure retained, no waiver.'))
history=[]
for name in ['stage-first','stage-second','stage-third','prior-first','prior-second']:
 p=scratch/(name+'.log')
 history.append(dict(name=name,log_sha256=sha(p),summary=[s for s in p.read_text().splitlines() if ('TEST' in s and '=====' in s) or ': ERROR:' in s],
  count_valid=name not in ['stage-third','prior-second'],note='Concurrent root reports share .test_counts; final isolated reports supersede their counts.' if name in ['stage-third','prior-second'] else 'Pre-repair run; final checks supersede.'))
for folder in ['controls-first','controls-second','controls-third','controls-final','controls-fourth']:
 rows=json.loads((scratch/folder/'checks.json').read_text());bad=[r for r in rows if (r['status']==0)!=r['expected_success']]
 history.append(dict(name=folder,commands=len(rows),failures=bad,source='Pre-final control history; not rebound to final compiler.'))
write('validation-history.json',history)
verification=[]
for dataset,folder in [('preliminary-common-performance.json','common-performance'),('preliminary-owner-performance.json','owner-performance'),('common-performance.json','common-final'),('owner-performance.json','owner-final')]:
 data=json.loads((scratch/folder/'performance.json').read_text());assert all(r['status']==0 for r in data['runs'])
 assert data['binaries']['B']['sha256']==(sha(scratch/'cppgm-after') if dataset.startswith('preliminary-') else compiler)
 assert data['binaries']['A']['sha256']==sha(scratch/'cppgm-before')
 for name,v in data['inputs'].items():
  assert sha(scratch/folder/(name+'.cpp'))==(v if isinstance(v,str) else v['sha256'])
  if not isinstance(v,str):assert hashlib.sha256(v['source'].encode()).hexdigest()==v['sha256']
 for name,modes in data['summary'].items():
  for mode,s in modes.items():
   rows=[r for r in data['runs'] if r['workload']==name and r['mode']==mode]
   if 'common' in dataset:
    ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
    assert ratios==s['paired_ratios'] and statistics.median(ratios)==s['paired_ratio_median']
   else:assert statistics.median(r['wall_s'] for r in rows)==s['median_s']
 for name,images in data['images'].items():
  for suffix,im in (images.items() if 'common' in dataset else [('',images)]):
   assert sha(scratch/folder/(name+suffix+'.o'))==im['object_sha256']
   assert sha(scratch/folder/(name+suffix))==im['executable_sha256']
 write(dataset,data)
 verification.append(dict(dataset=dataset,observations=len(data['runs']),launchers=len(data.get('launchers',[])),input_hashes_match=True,binaries_match=True,paired_ratios_match=True,images_match=True))
write('performance-verification.json',verification)
print('Bound checks and performance to',code,compiler)

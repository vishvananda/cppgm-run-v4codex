#!/usr/bin/env python3
"""Recompute PA29 implementation193 evidence and bind it to committed code."""
import hashlib,json,pathlib,statistics,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];scratch=pathlib.Path(sys.argv[1]).resolve();out=root/'student.tests/pa29/evidence193'
entry='71b9f44aa662154c0c072fd9e4df6510324fce09';code='60db24f609676f3cf60af549978939b2ecd39f60'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
def write(name,value):(out/name).write_text(json.dumps(value,indent=2)+'\n')
compiler=sha(root/'dev/cppgm++');assert compiler==sha(scratch/'final')
assert not git('diff','--name-only',code,'--','dev')
source={p:sha(root/p) for p in git('ls-files','dev').splitlines()}
write('source-binding.json',dict(entry_commit=entry,code_commit=code,dev_tree=git('rev-parse',code+':dev'),compiler_sha256=compiler,entry_compiler_sha256=sha(scratch/'entry'),tracked_dev_paths=len(source),validated_source_sha256=hashlib.sha256(json.dumps(source,sort_keys=True).encode()).hexdigest(),worktree_dev_matches_committed_tree=True,following_record_commit_changes_implementation=False,personal_sources={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29/source193').iterdir())}))
old=json.loads((root/'student.tests/pa29/evidence192/coverage.json').read_text());files={p:sha(root/p) for p in old['files']}
assert files==old['files'];inputs=[p for p in files if p.endswith('.t')];assert len(inputs)==403
assert not git('diff','--name-only',entry,'HEAD','--','pa29/tests','pa29/scripts','scripts','Makefile','TESTING_AND_REFERENCES.md')
write('coverage.json',dict(entry_commit=entry,code_commit=code,total_stage_inputs=len(inputs),files=files,changed=[],inputs_statuses_references_harness_and_comparison_rules_unchanged=True))
checks=json.loads((scratch/'validation-complete.json').read_text());assert len(checks)==4 and all(c['status']==0 for c in checks)
for c in checks:assert c['log_sha256']==sha(pathlib.Path(c['log']))
for c,count in zip(checks,['403 / 403','4538 / 4538','4941 / 4941']):assert count in c['summary'][0]
controls=json.loads((scratch/'controls-complete/checks.json').read_text());assert controls['compiler_sha256']==compiler and not controls['failures']
assert all((c['status']==0)==c['expected_success'] for c in controls['commands'])
assert all(c['passed'] for c in controls['properties']);write('controls.json',controls)
write('validation.json',dict(compiler_sha256=compiler,checks=checks,personal_control_commands=len(controls['commands']),personal_control_properties=len(controls['properties']),personal_control_failures=0))
write('stage-delta.json',dict(total=403,entry_failures=1,final_failures=0,fixed=['pa29/tests/run/800-out-of-class-nested-template-abi-tag-suppression-run.t'],new_failures=[],remaining=[],prior_passed=4538,prior_total=4538,coverage_preserved=True,progress_criterion_satisfied=True))
write('remaining.json',dict(total=403,passed=403,failed=0,unfinished_implementation=[],independent_review=['Whole-stage architecture/correctness/performance audit of implementation191–193 and prior-stage inheritance; review markers unchanged.'],next_boundary='Return implementation handoff to Ralph; stage advancement requires independent full audit.'))
write('preliminary-controls.json',json.loads((scratch/'controls/checks.json').read_text()))
write('intermediate-controls.json',json.loads((scratch/'controls-final/checks.json').read_text()))
verification=[]
for dataset,folder in [('common-performance.json','common'),('owner-performance.json','owner')]:
 data=json.loads((scratch/folder/'performance.json').read_text());assert data['binaries']['B']['sha256']==compiler and data['binaries']['A']['sha256']==sha(scratch/'entry')
 assert all(r['status']==0 for r in data['runs'])
 for name,v in data['inputs'].items():assert sha(scratch/folder/(name+'.cpp'))==(v if isinstance(v,str) else v['sha256'])
 for name,images in data['images'].items():
  for label,v in images.items():
   assert sha(scratch/folder/(name+label+'.o'))==v['object_sha256'];assert sha(scratch/folder/(name+label))==v['executable_sha256']
  if 'A' in images:assert images['A']['object_sha256']==images['B']['object_sha256'] and images['A']['executable_sha256']==images['B']['executable_sha256']
 for name,modes in data['summary'].items():
  for mode,s in modes.items():
   rows=[r for r in data['runs'] if r['workload']==name and r['mode']==mode]
   if 'paired_ratios' in s:
    ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
    assert ratios==s['paired_ratios'] and statistics.median(ratios)==s['paired_ratio_median']
   for label in [k for k in ['A','B'] if k in s]:
    values=[r for r in rows if r['label']==label and ('paired_ratios' not in s or r['block'])]
    assert s[label]['median_s']==statistics.median(r['wall_s'] for r in values)
    assert s[label]['peak_rss_kib']==max(r['peak_rss_kib'] for r in values)
 write(dataset,data);verification.append(dict(dataset=dataset,observations=len(data['runs']),launchers=len(data.get('launchers',[])),input_hashes_match=True,binaries_match=True,paired_ratios_match=True,images_match=True))
write('performance-verification.json',verification)
print('Verified',code,compiler,verification)

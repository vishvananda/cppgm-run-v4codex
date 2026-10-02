#!/usr/bin/env python3
"""Verify implementation 213's incomplete, committed handoff and raw evidence."""
import hashlib,json,pathlib,re,statistics,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2];E=ROOT/'student.tests/pa32/evidence213'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def read(name):return json.loads((E/name).read_text())
b=read('binding.json');artifact=pathlib.Path(b['artifact_root'])
def frozen(path):return artifact/pathlib.Path(path).relative_to('/tmp/pa32-213') if path.startswith('/tmp/pa32-213/') else pathlib.Path(path)
for name,digest in b['sources'].items():assert sha(ROOT/name)==digest,name
for name,digest in b['artifacts'].items():assert sha(name)==digest,name
for name,digest in b['binaries'].items():assert sha(name)==digest,name
assert sha(ROOT/'dev/cppgm++')==sha(artifact/'B-handoff')
assert sha(ROOT/'dev/lowiropt')==sha(artifact/'lowiropt-B-handoff')
assert git('rev-parse','HEAD:dev')==b['dev_tree']
assert not git('diff',b['implementation_commit'],'--','dev')
assert git('rev-parse','HEAD:pa32/tests')==b['contract_tree']==git('rev-parse',b['entry_commit']+':pa32/tests')
for path in git('diff','--name-only',b['entry_commit']).splitlines():
 assert path.startswith(('dev/','student.tests/pa32/')) or path=='pa32/plan.md',path
plan=(ROOT/'pa32/plan.md').read_text()
assert 'Stage base commit: '+b['stage_base'] in plan
assert 'Last reviewed commit: '+b['last_reviewed'] in plan
checks=read('checks.json');c={v['name']:v for v in checks}
for row in checks:assert sha(frozen(row['log']))==row['sha256']
for name in ['prior','file','normal-objects','debug-objects','memory','memory_native','memory-bounds','loops','loop_trip_properties','loop-bounds','objects','local','dataflow','calls','audit','trace','loop-trace','memory-trace']:
 assert c[name]['exit_code']==0,name
for name in ['stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3']:assert c[name]['exit_code']==2,name
assert 'ALL TESTS PASSED SUCCESSFULLY! (5178 / 5178)' in frozen(c['prior']['log']).read_text()
assert '5380 / 5397 TESTS PASSED' in frozen(c['through']['log']).read_text()
assert 'File audit passed for pa32 with 4 warning(s)' in frozen(c['file']['log']).read_text()
for lane in ['normal-objects','debug-objects']:assert 'PASS (25/25)' in frozen(c[lane]['log']).read_text()
debug=frozen(c['debug']['log']).read_text()
for level,count in [(1,3),(2,1),(3,1)]:assert f'pa32 tests/debuginfo/o{level}: PASS ({count}/{count})' in debug
baseline=(artifact/'baseline.log').read_text();current=frozen(c['stage']['log']).read_text()
def failures(text):return set(re.findall(r'^(pa32/[^:]+): ERROR:',text,re.M))
old,new=failures(baseline),failures(current)
assert len(old)==23 and len(new)==17 and new<old
assert sorted(old-new)==b['resolved_failures'] and sorted(new)==b['remaining_failures']
assert '196 / 219 TESTS PASSED' in baseline and '202 / 219 TESTS PASSED' in current
# Recompute every published summary, including noise calibration and outliers.
def summary(rows,s,extra=True):
 aa=[v['wall_s'] for v in rows if not v['block']]
 assert len(aa)==4 and s['AA_range_s']==[min(aa),max(aa)]
 ratios=[]
 for block in range(1,7):
  vals={k:[v for v in rows if v['block']==block and v['label']==k] for k in 'AB'}
  assert all(len(v)==2 for v in vals.values())
  ratios.append(statistics.mean(v['wall_s'] for v in vals['B'])/statistics.mean(v['wall_s'] for v in vals['A']))
 assert ratios==s['paired_ratios'] and statistics.median(ratios)==s['paired_ratio_median']
 assert [min(ratios),max(ratios)]==s['paired_ratio_range']
 if not extra:return
 for label in 'AB':
  vals=[v for v in rows if v['block'] and v['label']==label]
  assert statistics.median(v['wall_s'] for v in vals)==s[label]['median_s']
  assert [min(v['wall_s'] for v in vals),max(v['wall_s'] for v in vals)]==s[label]['range_s']
  assert max(v['peak_rss_kib'] for v in vals)==s[label]['peak_rss_kib']
 assert all(v['status']==0 for v in rows)
count=0
for lane,directory in b['performance_lanes'].items():
 d=read(lane+'.json');count+=len(d['runs'])
 for binary in d['binaries'].values():assert sha(frozen(binary['path']))==binary['sha256']
 if lane.endswith('-long'):
  summary(d['runs'],d['summary'],False);continue
 if lane.endswith('selfhost'):
  summary(d['runs'],d['summary'])
  for path,digest in d['inputs'].items():assert sha(path)==digest
  continue
 for name,s in d['summary'].items():
  for mode in ['compile','runtime']:summary([v for v in d['runs'] if v['workload']==name and v['mode']==mode],s[mode])
  images=d['images'][name]
  for label in 'AB':
   assert sha(artifact/directory/(name+label+'.o'))==images[label]['object_sha256']
   assert sha(artifact/directory/(name+label))==images[label]['executable_sha256']
  if lane in ['affected','exploratory','intermediate','policy','pre-cost']:
   for ext,key in [('.lowir','lowir_sha256'),('.cpp','main_sha256')]:assert sha(artifact/directory/(name+ext))==d['inputs'][name][key]
   if lane=='affected':
    if name in ['loads','diamonds']:
     assert all(v<1 for v in s['runtime']['paired_ratios'])
     assert s['compile']['B']['median_s']-s['compile']['A']['median_s'] < s['runtime']['A']['median_s']-s['runtime']['B']['median_s']
    if name in ['conditional','private-loads']:assert images['A']['object_sha256']==images['B']['object_sha256']
    if name in ['copies','private']:assert s['compile']['paired_ratio_median']<1
    # Ratios are disclosed diagnostic targets, not invented completion gates.
  else:
   assert sha(artifact/directory/(name+'.cpp'))==d['inputs'][name]
assert count==b['sample_count']
bounds=read('bounds.json')
assert bounds[0]['stats']['memory_work']*10==bounds[2]['stats']['memory_work']
assert bounds[3]['stats']['memory_declined']==1
assert any(v['case']=='ELF-storage-alias' for v in bounds)
assert any(v.get('cases')==2136 for v in bounds)
assert not git('status','--porcelain'), 'handoff must be committed and clean'
print(f'PA32 implementation 213 handoff PASS: 23 -> 17 unchanged-coverage failures; prior/file pass; replay 25/25; {count} frozen samples; clean tree')

#!/usr/bin/env python3
"""Verify implementation 212's incomplete handoff and its frozen evidence."""
import hashlib,json,pathlib,re,statistics,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2];E=ROOT/'student.tests/pa32/evidence212'
def sha(p):return hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def read(name):return json.loads((E/name).read_text())
b=read('binding.json');artifact=pathlib.Path(b['artifact_root'])
def frozen(path):return artifact/pathlib.Path(path).relative_to('/tmp/pa32-212') if path.startswith('/tmp/pa32-212/') else pathlib.Path(path)
for name,digest in b['sources'].items():assert sha(ROOT/name)==digest,name
for name,digest in b['artifacts'].items():assert sha(name)==digest,name
for name,digest in b['binaries'].items():assert sha(name)==digest,name
assert sha(ROOT/'dev/cppgm++')==sha(artifact/'B-final')
assert sha(ROOT/'dev/lowiropt')==sha(artifact/'optB-final')
assert git('rev-parse','HEAD:dev')==b['dev_tree']
assert not git('diff',b['implementation_commit'],'--','dev')
assert git('rev-parse','HEAD:pa32/tests')==b['contract_tree']==git('rev-parse',b['entry_commit']+':pa32/tests')
for path in git('diff','--name-only',b['entry_commit']).splitlines():
 assert path.startswith(('dev/','student.tests/pa32/')) or path=='pa32/plan.md',path
plan=(ROOT/'pa32/plan.md').read_text()
assert 'Stage base commit: '+b['stage_base'] in plan
assert 'Last reviewed commit: '+b['last_reviewed'] in plan
checks=read('checks.json');c={v['name']:v for v in checks}
for row in checks:assert sha(row['log'])==row['sha256']
for name in ['prior','file','debug-objects','loops','loop_trip_properties','loop-bounds','objects','local','dataflow','calls','audit','trace','loop-trace']:
 assert c[name]['exit_code']==0,name
for name in ['stage','through','debug','debug-driver-o1','debug-driver-o2','debug-driver-o3']:
 assert c[name]['exit_code']==2,name
assert 'ALL TESTS PASSED SUCCESSFULLY! (5178 / 5178)' in pathlib.Path(c['prior']['log']).read_text()
assert '5374 / 5397 TESTS PASSED' in pathlib.Path(c['through']['log']).read_text()
assert 'File audit passed for pa32 with 4 warning(s)' in pathlib.Path(c['file']['log']).read_text()
assert 'PASS (25/25)' in pathlib.Path(c['debug-objects']['log']).read_text()
debug=pathlib.Path(c['debug']['log']).read_text()
for level,count in [(1,3),(2,1),(3,1)]:assert f'pa32 tests/debuginfo/o{level}: PASS ({count}/{count})' in debug
baseline=(artifact/'baseline.log').read_text();current=pathlib.Path(c['stage']['log']).read_text()
def failures(text):return set(re.findall(r'^(pa32/[^:]+): ERROR:',text,re.M))
old,new=failures(baseline),failures(current)
assert len(old)==29 and len(new)==23 and new<old
assert sorted(old-new)==b['resolved_failures'] and sorted(new)==b['remaining_failures']
assert '190 / 219 TESTS PASSED' in baseline and '196 / 219 TESTS PASSED' in current
# Recompute every summary from all observations, retaining A/A and outliers.
def summary(rows,s):
 aa=[v['wall_s'] for v in rows if not v['block']]
 assert len(aa)==4 and s['AA_range_s']==[min(aa),max(aa)]
 ratios=[]
 for block in range(1,7):
  vals={k:[v for v in rows if v['block']==block and v['label']==k] for k in 'AB'}
  assert all(len(v)==2 for v in vals.values())
  ratios.append(statistics.mean(v['wall_s'] for v in vals['B'])/statistics.mean(v['wall_s'] for v in vals['A']))
 assert ratios==s['paired_ratios'] and statistics.median(ratios)==s['paired_ratio_median']
 assert [min(ratios),max(ratios)]==s['paired_ratio_range']
 for label in 'AB':
  vals=[v for v in rows if v['block'] and v['label']==label]
  assert statistics.median(v['wall_s'] for v in vals)==s[label]['median_s']
  assert [min(v['wall_s'] for v in vals),max(v['wall_s'] for v in vals)]==s[label]['range_s']
  assert max(v['peak_rss_kib'] for v in vals)==s[label]['peak_rss_kib']
 assert all(v['status']==0 for v in rows)
count=0
for lane,directory in [('affected','performance-final'),('exploratory','performance'),('common-o0','common-o0'),('common-o1','common-o1'),('common-o3','common-o3'),('selfhost','selfhost')]:
 d=read(lane+'.json');count+=len(d['runs'])
 for binary in d['binaries'].values():assert sha(frozen(binary['path']))==binary['sha256']
 if lane=='selfhost':
  summary(d['runs'],d['summary']);assert d['images']['A']==d['images']['B']
  for path,digest in d['inputs'].items():assert sha(path)==digest
  continue
 for name,s in d['summary'].items():
  for mode in ['compile','runtime']:summary([v for v in d['runs'] if v['workload']==name and v['mode']==mode],s[mode])
  images=d['images'][name]
  for label in 'AB':
   assert sha(artifact/directory/(name+label+'.o'))==images[label]['object_sha256']
   assert sha(artifact/directory/(name+label))==images[label]['executable_sha256']
  if lane in ['affected','exploratory']:
   assert all(v<1 for v in s['runtime']['paired_ratios']), 'repeatable benefit required'
   assert s['compile']['B']['median_s']-s['compile']['A']['median_s'] < s['runtime']['A']['median_s']-s['runtime']['B']['median_s']
   for ext,key in [('.lowir','lowir_sha256'),('.cpp','main_sha256')]:assert sha(artifact/directory/(name+ext))==d['inputs'][name][key]
   # Diagnostic ratios are disclosed measurements, not invented exit gates.
   if lane=='affected' and name=='unroll':
    stats=next(x for x in images['B']['telemetry'] if 'loops_unrolled' in x)
    assert stats['loop_clone_reserved']<=4096 and stats['loops_unrolled']==163
  else:
   assert images['A']['object_sha256']==images['B']['object_sha256']
   assert sha(artifact/directory/(name+'.cpp'))==d['inputs'][name]
assert count==924
for row in read('bounds.json'):
 if row.get('case')=='one-function-budget':assert row['telemetry']['loop_clone_reserved']<=256
 if row.get('case')=='body-budget':assert row['telemetry']['loop_clone_reserved']==0
assert not git('status','--porcelain'), 'handoff must be committed and clean'
print('PA32 implementation 212 handoff PASS: 29 -> 23 unchanged-coverage failures; prior/file pass; direct debug 5/5; 924 frozen samples; clean tree')

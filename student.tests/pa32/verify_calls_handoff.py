#!/usr/bin/env python3
"""Check the PA32-209 implementation handoff, not whole-stage completion."""
import hashlib,json,pathlib,re,statistics,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2]
HERE=pathlib.Path(__file__).resolve().parent/'evidence209'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
binding=json.loads((HERE/'binding.json').read_text())
for name,value in binding['source_sha256'].items(): assert sha(ROOT/name)==value,name
for name,value in binding['evidence_sha256'].items(): assert sha(HERE/name)==value,name
for name,value in binding['artifact_sha256'].items(): assert sha(pathlib.Path(name))==value,name
assert subprocess.check_output(['git','diff',binding['entry_head'],'--','pa32/tests','pa32/scripts','scripts','TESTING_AND_REFERENCES.md','spec.md'],cwd=ROOT)==b''
raw=0
for path in sorted(HERE.glob('*.json')):
 if path.name=='binding.json': continue
 d=json.loads(path.read_text())
 for binary in d['binaries'].values():assert sha(pathlib.Path(binary['path']))==binary['sha256'],binary['path']
 groups={}
 for row in d['runs']:
  assert row['status']==0
  groups.setdefault((row.get('workload','component'),row.get('mode','compile')),[]).append(row)
  raw+=1
 for (workload,mode),rows in groups.items():
  assert len(rows)==28,(path,workload,mode,len(rows))
  for block in range(7):
   assert ''.join(r['label'] for r in rows if r['block']==block)==('AAAA' if block==0 else 'ABBA')
  ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==block and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==block and r['label']=='A') for block in range(1,7)]
  s=d['summary'] if workload=='component' else d['summary'][workload][mode]
  assert ratios==s['paired_ratios']
  assert statistics.median(ratios)==s['paired_ratio_median']
  assert [min(ratios),max(ratios)]==s['paired_ratio_range']
  for label in 'AB':
   chosen=[r for r in rows if r['block'] and r['label']==label]
   assert s[label]['peak_rss_kib']==max(r['peak_rss_kib'] for r in chosen)
   assert s[label]['median_s']==statistics.median(r['wall_s'] for r in chosen)
  if path.name in binding['accepted_measurements'] and mode=='compile':
   assert s['paired_ratio_median']<=2
   assert s['B']['peak_rss_kib']<=1.75*s['A']['peak_rss_kib']
 for workload in binding['benefit_workloads'].get(path.name,[]):
  assert max(d['summary'][workload]['runtime']['paired_ratios'])<1
  images=d['images'][workload]
  assert images['B']['object_text_bytes']<=1.5*images['A']['object_text_bytes']
 for workload in binding['identical_objects'].get(path.name,[]):
  assert d['images'][workload]['A']['object_sha256']==d['images'][workload]['B']['object_sha256']
# Check raw sources and objects as well as metadata describing them.
art=pathlib.Path(binding['artifact_dir'])
for sub in ['affected-final','constants-final','common-final-o1','common-final-o0']:
 d=json.loads((art/sub/'performance.json').read_text())
 for workload,images in d['images'].items():
  for label,expected in images.items():
   assert sha(art/sub/(workload+label+'.o'))==expected['object_sha256']
   assert sha(art/sub/(workload+label))==expected['executable_sha256']
 for name,value in d['inputs'].items():
  if isinstance(value,str):assert sha(art/sub/(name+'.cpp'))==value
  else:
   assert sha(art/sub/(name+'.lowir'))==value['lowir_sha256']
   assert sha(art/sub/(name+'.cpp'))==value['main_sha256']
def failures(name):return {l.split(':')[0] for l in (art/name).read_text().splitlines() if ': ERROR:' in l}
a,b=failures('baseline-tests.log'),failures('stage-final.log')
assert len(a)==92 and len(b)==41 and b<a,(len(a),len(b),b-a)
assert sorted(a-b)==binding['removed_failures']
assert '178 / 219 TESTS PASSED' in (art/'stage-final.log').read_text()
assert '5178 / 5178' in (art/'prior-final.log').read_text()
assert 'File audit passed for pa32' in (art/'audit-final.log').read_text()
assert 'PASS (25/25)' in (art/'debug-replay-final.log').read_text()
for log in ['local-final.log','dataflow-final.log','calls-final.log']:assert 'PASS' in (art/log).read_text()
plan=(ROOT/'pa32/plan.md').read_text()
for marker in ['Stage base commit','Last reviewed commit']:
 assert marker+': e82bf4152fe8d6d68b9cd966655db0d8142cf81b' in plan
print(f'PA32-209 handoff evidence: PASS ({len(binding["source_sha256"])} source bindings, {raw} raw samples, 51 original failures removed; independent audit still owed)')

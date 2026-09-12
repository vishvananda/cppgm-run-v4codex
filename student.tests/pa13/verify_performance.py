#!/usr/bin/env python3
"""Verify the checked-in PA13 evidence against its frozen artifacts."""
from pathlib import Path
import hashlib,json
root=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(record):assert sha(record['path'])==record['sha256'],record['path']
p=root/'student.tests/pa13/performance.json';data=json.loads(p.read_text())
for b in data['binaries']:verify(b)
assert sha(root/'student.tests/pa13/benchmark.py')==data['harness_sha256']
assert sha(root/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
assert sha(root/'reference-binaries/lowir2native')==data['backend_sha256']
count=0
for name,w in data['workloads'].items():
 assert sha(w['source_path'])==w['source_sha256'],name
 for out in w['outputs']:
  verify(out)
  if 'native' in out:verify(out['native']);assert out['native']['checked_exit']==0
 if len(w['outputs'])==2:
  assert w['outputs'][0]['sha256']==w['outputs'][1]['sha256'],name
  if 'native' in w['outputs'][0]:assert w['outputs'][0]['native']['sha256']==w['outputs'][1]['native']['sha256'],name
 for key in ('compiler','runtime'):
  if key not in w:continue
  c=w[key];expected=[0,0,0,0,0,1,1,0,0,1,1,0] if len(w['outputs'])==2 else [1]*6
  assert [r['binary'] for r in c['observations']]==expected
  for r in c['observations']+c['warmups']:assert r['wall_s']>0 and r['rss_kib']>0 and r['checked_exit']==0
  count+=len(c['observations'])+len(c['warmups'])
r=json.loads((root/'student.tests/pa13/performance-repeat.json').read_text())
assert r['source_report_sha256']==sha(p)
assert r['harness_sha256']==sha(root/'student.tests/pa13/repeat_benchmark.py')
for name,w in r['workloads'].items():
 assert sha(w['source_path'])==w['source_sha256']
 assert w['output_hashes']==[o['sha256'] for o in data['workloads'][name]['outputs']]
 assert [v['binary'] for v in w['observations']]==[0]*4+[0,1,1,0]*4
 count+=len(w['observations'])+len(w['warmups'])
assert count==352,count
print('352 observations and all frozen input/binary/output hashes verified')

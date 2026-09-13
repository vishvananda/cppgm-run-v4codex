#!/usr/bin/env python3
"""Check frozen lifecycle observations, legality proofs, states and layouts."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign,outputs

def verify():
 proof=document('lifecycle-proofs.json')
 assert shared.sha(ROOT/'student.tests/pa14/lifecycle_evidence.py')==proof['harness_sha256']
 for row in proof['sources']+proof['artifacts']+[proof['spec'],proof['standard'],proof['handout']]:binary(row)
 assert len(proof['coverage'])==1266 and sum(r['path'].endswith('.t') for r in proof['coverage'])==314
 assert len(proof['personal'])==35
 for row in proof['coverage']+proof['personal']:assert shared.sha(ROOT/row['path'])==row['sha256']
 for row in proof['checkpoints']:checked(row)
 assert [r['count'] for r in proof['checkpoints']]==[314,1621]
 assert len(proof['controls'])==6
 for group in proof['controls']:
  binary(group['manifest'])
  for row in group['files']:binary(row)
  rows=json.loads(Path(group['manifest']['path']).read_text())
  if group['group']=='definitions':
   assert len(rows['checks'])==22
   for row in rows['checks']:checked(row)
  else:
   assert len(rows)==(16 if group['group']=='properties' else 6)
   for row in rows:
    assert row['exit_code']==(1 if row.get('valid')==False else 0)
    text=Path(row['log']).read_text()
    if group['group']=='actions':
     if row['name']=='transfer-failure':assert 'failures 10000 deleted 0' in text
     elif row['name']=='transfer-deleted':assert 'failures 0 deleted 1' in text
     else:assert ('before -1 after '+('-1 failures 10000' if 'failure' in row['name'] else '1 failures 0')) in text
 validation=document('lifecycle-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/lifecycle_validation.py')==validation['harness_sha256']
 assert len(validation['checks'])==79 and len(validation['reducers'])==8
 for row in validation['checks']:checked(row)
 for row in validation['binaries']:binary(row)
 for row in validation['reducers']:
  binary(row)
  for pre in ['source','native']:assert shared.sha(row[pre+'_path'])==row[pre+'_sha256']
 for name in ['sanitizer-parity','baseline-parity']:
  row=next(r for r in validation['checks'] if r['name']==name)
  assert '349 source status/output/sanitizer checks PASS' in Path(row['log']).read_text()
 row=next(r for r in validation['checks'] if r['name']=='through')
 assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in Path(row['log']).read_text()
 layout=document('lifecycle-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/lifecycle_layout.py')==layout['harness_sha256']
 assert shared.sha(layout['source_path'])==layout['source_sha256']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   # The default-fact verifier now owns live headers; these stay frozen.
  for k in ['binary','dump']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 assert layout['layouts'][0]['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56,6216,120,28,24,64,1472]
 assert layout['layouts'][1]['sizes']==[112,36,36,20,8,504,48,48,120,32,20,56,6216,120,28,24,64,1472]
 data=document('lifecycle-performance.json')
 assert data['binaries'][1]['sha256']==validation['binaries'][1]['sha256']
 assert data['binaries'][0]['sha256']==validation['binaries'][0]['sha256']
 assert shared.sha(ROOT/'student.tests/pa14/lifecycle_benchmark.py')==data['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
 assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
 assert shared.sha(data['parent_path'])==data['parent_sha256']
 for row in data['binaries']:binary(row)
 assert len(data['workloads'])==15
 count=natives=0
 for name,row in data['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256'];outputs(row)
  for b,o in enumerate(row['outputs']):assert o['binary_sha256']==data['binaries'][b]['sha256'] and Path(o['path']).stat().st_size==o['bytes']
  count+=campaign(row['compiler'])
  if 'runtime' in row:
   natives+=1;count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
  if row.get('shape'):
   n,k,s,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
   assert stats['semantic_member_demands']==stats['semantic_demand_processed']==k+1
   assert stats['semantic_destruction_actions']==k*s
 assert natives==7 and count==308
 noise=document('lifecycle-noise.json')
 assert shared.sha(ROOT/'student.tests/pa14/lifecycle_noise.py')==noise['harness_sha256']
 assert shared.sha(noise['parent_path'])==noise['parent_sha256']
 assert noise['binaries']==data['binaries'] and len(noise['workloads'])==5
 for name,row in noise['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256']==data['workloads'][name]['source_sha256']
  outputs(row);count+=campaign(row['compiler'])
  for b,o in enumerate(row['outputs']):
   assert o['sha256']==data['workloads'][name]['outputs'][b]['sha256']
  if 'runtime' in row:
   count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
 assert count==392
 print('392 lifecycle observations, 349 parity inputs, 20 public query runs, cross-TU lifetime controls and 24 frozen headers verified')
 return count
if __name__=='__main__':verify()

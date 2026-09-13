#!/usr/bin/env python3
"""Current virtual-demand ownership and complete, unfiltered A/B evidence."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign,outputs

def verify():
 proof=document('virtual-demand-proofs.json')
 assert shared.sha(ROOT/'student.tests/pa14/virtual_demand_evidence.py')==proof['harness_sha256']
 for row in proof['sources']+proof['outputs']+proof['entry_observations']+proof['deduplication']+[proof['spec'],proof['standard'],proof['handout'],proof['host']]:binary(row)
 for row in proof['checks']:checked(row)
 limit=proof['backend_limitation']
 assert limit['exit_code']==1 and shared.sha(limit['log'])==limit['log_sha256']
 assert 'undefined native symbol: pure_virtual' in Path(limit['log']).read_text()
 assert len(proof['coverage'])==1266 and sum(r['path'].endswith('.t') for r in proof['coverage'])==314
 assert len(proof['personal'])==35
 for row in proof['coverage']+proof['personal']:assert shared.sha(ROOT/row['path'])==row['sha256']
 for group in proof['controls']:
  for row in group['logs']+[group['binary'],group['manifest']]:binary(row)
  rows=json.loads(Path(group['manifest']['path']).read_text());assert len(rows)==6
  for row in rows:
   assert row['exit_code']==0
   failed='failure' in row['name'];text=Path(row['log']).read_text()
   assert text.startswith('failures '+str(10000 if failed else 0)+' failed_vtables '+str(int(row['name']=='failure'))+'\n')
 layout=document('virtual-demand-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/virtual_demand_layout.py')==layout['harness_sha256']
 assert shared.sha(layout['source_path'])==layout['source_sha256']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
  for k in ('binary','dump'):assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 assert layout['layouts'][0]['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56,6160,120,28,24,64,1456]
 assert layout['layouts'][1]['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56,6216,120,28,24,64,1472]
 validation=document('virtual-demand-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/virtual_demand_validation.py')==validation['harness_sha256']
 assert len(validation['checks'])==63 and len(validation['reducers'])==8
 for row in validation['checks']:checked(row)
 for row in validation['binaries']:binary(row)
 for row in validation['reducers']:
  for prefix in ('source','native'):assert shared.sha(row[prefix+'_path'])==row[prefix+'_sha256']
  binary(row)
 for name,count in [('stage',314),('prior',1621),('through',1935)]:
  row=next(r for r in validation['checks'] if r['name']==name)
  text=Path(row['log']).read_text();assert 'ALL TESTS PASSED SUCCESSFULLY!' in text and str(count) in text
 for name in ['sanitizer-parity','baseline-parity']:
  row=next(r for r in validation['checks'] if r['name']==name)
  assert '349 source status/output/sanitizer checks PASS' in Path(row['log']).read_text()
 abi=document('virtual-demand-abi-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/virtual_demand_abi_validation.py')==abi['harness_sha256']
 assert len(abi['checks'])==12
 for row in abi['binaries']:binary(row)
 for row in abi['checks']:checked(row);assert shared.sha(row['harness_path'])==row['harness_sha256']
 data=document('virtual-demand-performance.json');noise=document('virtual-demand-noise.json')
 assert data['binaries'][1]['sha256']==validation['binaries'][1]['sha256']
 assert data['binaries']==noise['binaries']
 count=0
 for report,harness,total,native_count in [(data,'virtual_demand_benchmark.py',15,7),(noise,'virtual_demand_noise.py',5,1)]:
  assert shared.sha(ROOT/'student.tests/pa14'/harness)==report['harness_sha256']
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==report['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==report['backend_sha256']
  assert shared.sha(report['parent_path'])==report['parent_sha256']
  assert len(report['workloads'])==total
  natives=0
  for row in report['binaries']:binary(row)
  for name,row in report['workloads'].items():
   assert shared.sha(row['source_path'])==row['source_sha256'];outputs(row)
   for b,o in enumerate(row['outputs']):assert o['binary_sha256']==report['binaries'][b]['sha256'] and Path(o['path']).stat().st_size==o['bytes']
   for a,b in zip(row['outputs'][0]['telemetry'],row['outputs'][1]['telemetry']):
    for k in a:
     if not k.endswith('_ms') and k!='peak_rss_kib':assert a[k]==b[k],(name,k,a[k],b[k])
   count+=campaign(row['compiler'])
   if 'runtime' in row:
    natives+=1;count+=campaign(row['runtime'])
    assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
  assert natives==native_count
 assert count==392
 for name,row in data['workloads'].items():
  if not row.get('shape'):continue
  n,k,s,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
  assert stats['semantic_key_vtable_notifications']==stats['semantic_key_vtable_processed']==k
  assert stats['semantic_vtable_emissions']==stats['semantic_virtual_demands']==k
  assert stats['semantic_virtual_slot_work']==k*s
  assert stats['semantic_vtable_queue_bytes']<=16*k
 small=data['workloads']['virtual-16000-1-4-1']['outputs'][1]['telemetry'][0]
 for name in ['virtual-64000-1-4-1','virtual-16000-1-4-4000']:
  row=data['workloads'][name]['outputs'][1]['telemetry'][0]
  assert row['lower_virtual_cache_bytes']==small['lower_virtual_cache_bytes']==56
  assert row['semantic_vtable_queue_bytes']==small['semantic_vtable_queue_bytes']==8
 dedup=json.loads(Path(next(r['path'] for r in proof['deduplication'] if r['path'].endswith('deduplication-complete.json'))).read_text())
 assert dedup['completed'] and len(dedup['groups'])==63 and dedup['estimated_saved_bytes']==5157573103
 for row in dedup['groups']:
  first=Path(row['paths'][0]);assert shared.sha(first)==row['sha256'] and first.stat().st_size==row['bytes']
  for name in row['paths']:assert first.samefile(name) and not (Path(name).stat().st_mode & 0o222)
 print('392 virtual-demand observations, 349 parity inputs, 12 public state runs, 24 frozen layout headers and preserved deduplicated archives verified')
 return count
if __name__=='__main__':verify()

#!/usr/bin/env python3
"""Current terminal-state evidence and stage-scoped performance acceptance."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign,outputs

def verify():
 proof=document('demand-failure-proofs.json')
 for name,field in [('demand_failure_evidence.py','harness_sha256'),('check_demand_failures.py','control_sha256'),('demand-failures.cc','probe_sha256')]:
  assert shared.sha(ROOT/'student.tests/pa14'/name)==proof[field]
 for row in proof['binaries']+proof['entry_observations']+proof['scope_correction']+[proof['spec'],proof['standard'],proof['native_source'],proof['host']]:binary(row)
 for row in proof['checks']:checked(row)
 assert [r['name'] for r in proof['checks']]==['stage','prior','through','file-audit']
 assert proof['host']['build_exit']==proof['host']['run_exit']==0
 assert len(proof['controls'])==18
 for row in proof['controls']:
  checked(row);assert 'failures 10000 ' in Path(row['log']).read_text()
 assert len(proof['coverage'])==1266 and sum(r['path'].endswith('.t') for r in proof['coverage'])==314
 for r in proof['coverage']:assert shared.sha(ROOT/r['path'])==r['sha256']
 layout=document('demand-failure-layout.json')
 assert layout['build_exit']==0 and len(layout['headers'])==18
 for h in layout['headers']:assert shared.sha(h['path'])==shared.sha(h['source'])==h['sha256']
 for k in ('source','binary','dump'):assert shared.sha(layout[k+'_path'])==layout[k+'_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56,6160,120,28,24]
 validation=document('demand-failure-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/demand_failure_validation.py')==validation['harness_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','optional_diagnostics','abi_controls','reducer_controls')]==[314,1621,1935,34,348,166,1,6,7]
 assert len(validation['checks'])==121
 for r in validation['checks']:checked(r)
 for b in validation['binaries']:binary(b)
 for r in validation['personal']+validation['coverage']:assert shared.sha(ROOT/r['path'])==r['sha256']
 assert len(validation['personal'])==34 and validation['coverage']==proof['coverage']
 for row in validation['reducers']+validation['signature_reducers']+[validation['owner_control'],validation['initializer_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for o in row['outputs']:
   assert o['native_exit']==0 and shared.sha(o['path'])==o['sha256'] and shared.sha(o['native_path'])==o['native_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 for name in ('store_control','fact_store_control'):
  row=validation[name];assert shared.sha(row['source_path'])==row['source_sha256']
  for o in row['outputs']:binary(o);assert o['exit_code']==0
 data=document('demand-failure-performance.json')
 assert shared.sha(ROOT/'student.tests/pa14/demand_failure_benchmark.py')==data['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
 assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
 assert shared.sha(data['parent_path'])==data['parent_sha256']
 parent=json.loads(Path(data['parent_path']).read_text())
 assert data['binaries'][0]['sha256']==parent['binaries'][1]['sha256']
 assert data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']==shared.sha(ROOT/'dev/cppgm++')
 for b in data['binaries']:binary(b)
 assert len(data['workloads'])==14
 count=native=0
 for name,row in data['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256']==parent['workloads'][name]['source_sha256']
  outputs(row)
  assert row['outputs'][0]['sha256']==parent['workloads'][name]['outputs'][1]['sha256']
  for b,o in enumerate(row['outputs']):
   assert o['binary_sha256']==data['binaries'][b]['sha256']
   assert Path(o['path']).stat().st_size==o['bytes']
  # Every observed work/storage counter remains equal. Timing and process RSS
  # are measurements, not logical fact counts or pass/fail thresholds.
  for a,b in zip(row['outputs'][0]['telemetry'],row['outputs'][1]['telemetry']):
   for k in a:
    if not k.endswith('_ms') and k!='peak_rss_kib':assert a[k]==b[k],(name,k,a[k],b[k])
  count+=campaign(row['compiler'])
  if 'runtime' in row:
   native+=1;count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
 assert native==5 and count==266
 fast_validation=document('demand-failure-fastpath-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/demand_failure_fastpath_validation.py')==fast_validation['harness_sha256']
 assert fast_validation['control_sha256']==proof['control_sha256'] and fast_validation['probe_sha256']==proof['probe_sha256']
 assert shared.sha(fast_validation['parent_path'])==fast_validation['parent_sha256']
 for b in fast_validation['binaries']:binary(b)
 for r in fast_validation['checks']:checked(r)
 assert len(fast_validation['checks'])==61 and len(fast_validation['reducers'])==8
 for row in fast_validation['reducers']:
  assert shared.sha(row['source_path'])==row['source_sha256']
  assert shared.sha(row['path'])==row['sha256'] and shared.sha(row['native_path'])==row['native_sha256']
 for mode in ['release','sanitized']:
  folder=Path(fast_validation['checks'][0]['log']).parent/('demands-'+mode)
  rows=json.loads((folder/'checks.json').read_text());assert len(rows)==9
  for row in rows:assert row['exit_code']==0 and 'failures 10000 ' in Path(row['log']).read_text()
 fast=document('demand-failure-fastpath-performance.json')
 assert shared.sha(ROOT/'student.tests/pa14/demand_failure_fastpath_benchmark.py')==fast['harness_sha256']
 assert shared.sha(fast['parent_path'])==fast['parent_sha256']==shared.sha(ROOT/'student.tests/pa14/demand-failure-performance.json')
 assert [b['sha256'] for b in fast['binaries']]==[b['sha256'] for b in fast_validation['binaries'][:2]]
 assert fast['binaries'][0]['sha256']==data['binaries'][1]['sha256']
 # The shortcut trial is retained as evidence, not as the accepted compiler.
 for b in fast['binaries']:binary(b)
 assert list(fast['workloads'])==['declaration-instances-1000','demand-uses-1000-128-4','calls-4','demand-runtime']
 for name,row in fast['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256']==data['workloads'][name]['source_sha256']
  outputs(row)
  assert row['outputs'][0]['sha256']==data['workloads'][name]['outputs'][1]['sha256']
  for a,b in zip(row['outputs'][0]['telemetry'],row['outputs'][1]['telemetry']):
   for k in a:
    if not k.endswith('_ms') and k!='peak_rss_kib':assert a[k]==b[k],(name,k,a[k],b[k])
  count+=campaign(row['compiler'])
  if 'runtime' in row:
   count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
 assert count==336
 handoff=document('demand-failure-handoff.json')
 assert shared.sha(ROOT/'student.tests/pa14/demand_failure_handoff.py')==handoff['harness_sha256']
 assert handoff['accepted_binary']==data['binaries'][1] and handoff['total_observations']==14112
 assert handoff['trial']['status']=='rejected';binary(handoff['trial']['patch'])
 for row in handoff['checks']:checked(row)
 for row in handoff['evidence']+handoff['verifications']:binary(row)
 for row in handoff['final_controls']:
  for item in row['logs']+[row['source'],row['binary'],row['manifest']]:binary(item)
 print('336 demand-state observations including the rejected shortcut, 348 sanitizer inputs, 18 final repeated-demand runs and live layouts verified')
 return count
if __name__=='__main__':verify()

#!/usr/bin/env python3
"""Verify default semantics, unchanged coverage, live layouts and all samples."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign,outputs

def verify(check_live=True):
 proof=document('default-proofs.json')
 assert shared.sha(ROOT/'student.tests/pa14/default_evidence.py')==proof['harness_sha256']
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
  if group['group']=='defaults':
   assert len(rows['checks'])==67
   for row in rows['checks']:
    assert row['exit_code']==row['expected']
    assert shared.sha(row['log'])==row['log_sha256']
  else:
   assert len(rows)==(7 if group['group']=='facts' else 3)
   for row in rows:
    assert row['exit_code']==0
    lines=Path(row['log']).read_text().splitlines();stats=json.loads(lines[1])
    expected=2 if row['name']=='isolation' else 1
    assert stats['template_default_argument_work']==stats['template_default_facts']==expected
    if group['group']=='states':
     assert stats['template_default_demands']==expected
     assert ('failures '+('10000' if row['name']=='dependency-failure' else '0')) in lines[0]
 validation=document('default-final-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/default_validation.py')==validation['harness_sha256']
 assert len(validation['checks'])==85 and len(validation['reducers'])==8
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
 layout=document('default-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/default_layout.py')==layout['harness_sha256']
 assert shared.sha(layout['source_path'])==layout['source_sha256']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   if check_live and row['label']=='current':assert shared.sha(h['source'])==h['sha256']
  for k in ['binary','dump']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 assert layout['layouts'][0]['sizes']==[112,36,36,20,8,504,48,48,120,32,20,56,6216,120,28,24,64,1472]
 assert layout['layouts'][1]['sizes']==[112,36,36,20,8,504,48,48,120,32,20,56,6312,120,28,24,64,1472]
 row=layout['defaults']
 for k in ['source','binary']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
 assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']==[20,12,68]
 row=layout['entry_list']
 for k in ['source','binary']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
 assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']==[68]
 data=document('default-final-performance.json')
 assert data['binaries'][1]['sha256']==validation['binaries'][1]['sha256']
 if check_live:assert shared.sha(ROOT/'dev/cppgm++')==data['binaries'][1]['sha256']
 assert data['binaries'][0]['sha256']==validation['binaries'][0]['sha256']
 assert shared.sha(ROOT/'student.tests/pa14/default_benchmark.py')==data['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
 assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
 assert shared.sha(data['parent_path'])==data['parent_sha256']
 for row in data['binaries']:binary(row)
 assert len(data['workloads'])==17
 count=natives=0
 for name,row in data['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256'];outputs(row)
  for b,o in enumerate(row['outputs']):assert o['binary_sha256']==data['binaries'][b]['sha256'] and Path(o['path']).stat().st_size==o['bytes']
  count+=campaign(row['compiler'])
  if 'runtime' in row:
   natives+=1;count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
  if row.get('shape'):
   n,k,m,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
   assert stats['template_default_argument_work']==stats['template_default_facts']==stats['template_default_demands']==k
   assert stats['template_default_dependencies']==stats['template_default_dependency_work']==3*k
 for m in [1,32]:
  low=data['workloads'][f'default-16000-1-{m}-1']['outputs'][1]['telemetry'][0]
  high=data['workloads'][f'default-16000-1-{m}-4000']['outputs'][1]['telemetry'][0]
  assert high['semantic_candidate_work']-low['semantic_candidate_work']==3999
 assert natives==8 and count==350
 noise=document('default-final-noise.json')
 assert shared.sha(ROOT/'student.tests/pa14/default_final_noise.py')==noise['harness_sha256']
 assert shared.sha(noise['parent_path'])==noise['parent_sha256']
 assert noise['binaries']==data['binaries'] and len(noise['workloads'])==8
 assert data['finished_utc']<=noise['started_utc']
 for name,row in noise['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256']==data['workloads'][name]['source_sha256']
  outputs(row);count+=campaign(row['compiler'])
  for b,o in enumerate(row['outputs']):assert o['sha256']==data['workloads'][name]['outputs'][b]['sha256']
  if 'runtime' in row:
   count+=campaign(row['runtime'])
   assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
 assert count==476
 # The first frozen binary had not yet repaired the two access-context
 # reducers. Its correct comparison workloads and every sample remain evidence.
 for filename,harness,workloads in [('default-performance.json','default_benchmark.py',17),('default-noise.json','default_noise.py',8)]:
  prior=document(filename)
  assert shared.sha(ROOT/'student.tests/pa14'/harness)==prior['harness_sha256']
  assert shared.sha(prior['parent_path'])==prior['parent_sha256']
  for row in prior['binaries']:binary(row)
  assert len(prior['workloads'])==workloads
  for row in prior['workloads'].values():
   assert shared.sha(row['source_path'])==row['source_sha256'];outputs(row)
   count+=campaign(row['compiler'])
   if 'runtime' in row:count+=campaign(row['runtime'])
 assert count==952
 prior=document('default-validation.json')
 assert len(prior['checks'])==85
 for row in prior['checks']:checked(row)
 for row in prior['binaries']:binary(row)
 print('952 default observations, 349 unchanged parity inputs, 20 public query runs, 25 source controls and 24 '+('live' if check_live else 'frozen')+' headers verified')
 return count
if __name__=='__main__':verify()

#!/usr/bin/env python3
"""Verify mode legality, bounded cache ownership and all frozen AA/ABBA observations."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign

def verify():
 v=document('destination-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/destination_audit_validation.py')==v['harness_sha256']
 assert shared.sha(ROOT/'dev/cppgm++')==v['binaries'][1]['sha256']
 assert len(v['checks'])==73 and len(v['coverage'])==1266
 recovery=v['recovery']
 for field in ['harness','prior_manifest']:binary(recovery[field])
 failed=recovery['failed_check'];assert failed['exit_code']==1
 assert shared.sha(failed['log'])==failed['log_sha256'] and 'No space left on device' in Path(failed['log']).read_text()
 prior=json.loads(Path(recovery['prior_manifest']['path']).read_text())
 reused={r['name']:r for r in prior['checks'] if r['exit_code']==0}
 assert set(recovery['reused'])==set(reused)
 assert recovery['rerun'][0]=='sanitized-body-publication'
 assert len(recovery['reused'])+len(recovery['rerun'])==73
 for row in v['checks']:
  if row['name'] in reused:assert row==reused[row['name']]
 for row in v['binaries']+v['sources']+v['coverage']:binary(row)
 for row in v['checks']:checked(row)
 logs={r['name']:Path(r['log']).read_text() for r in v['checks']}
 assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in logs['through']
 assert '349 source status/output/sanitizer checks PASS' in logs['sanitizer-parity']
 assert '349 course-contract parity checks PASS' in logs['entry-parity']
 for label in ['release','sanitized']:
  for name,total,reject,harness in [('initializers',82,71,'check_initialization_facts.py'),('modes',75,49,'check_initialization_modes.py')]:
   row=next(r for r in v['checks'] if r['name']==label+'-'+name)
   controls=json.loads((Path(row['command'][-1])/'checks.json').read_text())
   assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14'/harness)
   binary(controls['binary']);assert len(controls['checks'])==total
   assert len({r['source_path'] for r in controls['checks']})==total
   assert sum(r['reject'] for r in controls['checks'])==reject
   for control in controls['checks']:
    assert shared.sha(control['source_path'])==control['source_sha256']
    assert shared.sha(control['log'])==control['log_sha256']
    diagnostic=Path(control['log']).read_text()
    assert 'Sanitizer' not in diagnostic and 'runtime error:' not in diagnostic
    assert control['exit_code']==int(control['reject'])
    if not control['reject']:binary(control['native']);assert control['native']['exit_code']==0
  assert logs[label+'-mode-cache'].count('10000 terminal copy failures; 10000 direct successes; stable nodes/entities/plans/conversions')==2
  row=next(r for r in v['checks'] if r['name']==label+'-mode-cache')
  controls=json.loads((Path(row['command'][2])/'checks.json').read_text())
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_initialization_mode_cache.py')
  assert controls['probe_source_sha256']==shared.sha(ROOT/'student.tests/pa14/initialization-modes.cc')
  assert len(controls['checks'])==2
  for r in controls['checks']:checked(r)
 layout=document('destination-layout.json');old=document('modes-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/destination_layout.py')==layout['harness_sha256']
 assert layout['layouts'][0]['sizes']==old['layouts'][1]['sizes']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   if row['label']=='current':assert shared.sha(h['source'])==h['sha256']
  for k in ['binary','dump']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 a,b=[r['sizes'] for r in layout['layouts']]
 assert a==b and a[12]==6552
 for row in layout['layouts']:
  records=row['conversion_records']
  assert shared.sha(records['source_path'])==records['source_sha256']
  assert shared.sha(records['binary_path'])==records['binary_sha256']
  assert list(map(int,shared.run([records['binary_path']]).stdout.split()))==records['sizes']==[52,28,8,80]
 count=0
 for filename,harness,expected in [('destination-performance.json','destination_benchmark_audit.py',37),('destination-noise.json','destination_audit_noise.py',11)]:
  data=document(filename)
  assert shared.sha(ROOT/'student.tests/pa14'/harness)==data['harness_sha256']
  assert shared.sha(data['parent_path'])==data['parent_sha256']
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
  assert [r['sha256'] for r in data['binaries']]==[r['sha256'] for r in v['binaries'][:2]]
  assert len(data['workloads'])==expected
  for row in data['binaries']:binary(row)
  for name,row in data['workloads'].items():
   assert shared.sha(row['source_path'])==row['source_sha256']
   for out in row['outputs']:binary(out);assert Path(out['path']).stat().st_size==out['bytes']
   if row.get('comparison'):
    proof=row['comparison'];assert proof['exit_code']==0
    assert shared.sha(proof['log'])==proof['log_sha256']
    assert proof['canonical'][0]['sha256']==proof['canonical'][1]['sha256']
    for output in proof['canonical']:binary(output)
   else:assert row['outputs'][0]['sha256']==row['outputs'][1]['sha256']
   count+=campaign(row['compiler'])
   if 'runtime' in row:
    count+=campaign(row['runtime'])
    for output in row['outputs']:binary(output['native']);assert output['native']['checked_exit']==0
    assert row['outputs'][0]['native']['text_bytes']==row['outputs'][1]['native']['text_bytes']
    assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
   if row.get('shape'):
    n,k,m,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
    assert stats['semantic_initializer_recipe_work']==2*m
    assert stats['semantic_initializer_recipe_uses']==2*k*m
    assert stats['semantic_body_checks']==stats['semantic_body_lifetime_checks']
    for counter in ['semantic_entities','semantic_scopes']:
     assert stats[counter]==row['outputs'][0]['telemetry'][0][counter]-k*m
 assert count==882
 for label in ['release','sanitized']:
  row=next(r for r in v['checks'] if r['name']==label+'-destinations')
  controls=json.loads((Path(row['command'][3])/'checks.json').read_text())
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_conversion_destination.py')
  assert controls['probe_source_sha256']==shared.sha(ROOT/'student.tests/pa14/conversion-destination.cc')
  binary(controls['binary']);assert len(controls['checks'])==6
  for case in controls['checks']:
   assert shared.sha(case['source_path'])==case['source_sha256']
   binary(case['native']);assert case['native']['exit_code']==0
  assert shared.sha(controls['probe_log'])==controls['probe_log_sha256']
  assert '4 destinations, 2 temporaries, 3 recipes; 10000 stable finish queries' in Path(controls['probe_log']).read_text()
  assert '2 destinations, 0 temporaries, 1 recipes; 10000 stable finish queries' in Path(controls['probe_log']).read_text()
 handoff=document('destination-handoff.json')
 assert shared.sha(handoff['binary'])==handoff['binary_sha256']==v['binaries'][1]['sha256']
 assert len(handoff['cases'])==5
 for row in handoff['cases']:
  assert Path(row['source_path']).read_text()==row['source']
  assert shared.sha(row['source_path'])==row['source_sha256']
  assert shared.sha(row['log'])==row['log_sha256']
  assert row['exit_code']==0 and row['required_exit']==1
 print('882 destination observations, 73 checks, 349 sanitizer/parity inputs, 150 mode controls and destination/cache ownership verified')
 return count
if __name__=='__main__':verify()

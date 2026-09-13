#!/usr/bin/env python3
"""Verify live initialization ownership and every frozen AA/ABBA observation."""
from pathlib import Path
import json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign

def verify():
 v=document('initializer-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/initializer_audit_validation.py')==v['harness_sha256']
 assert shared.sha(ROOT/'dev/cppgm++')==v['binaries'][1]['sha256']
 assert len(v['checks'])==67 and len(v['coverage'])==1266
 refresh=v['refresh']
 for name in ['harness','prior_manifest','prior_control']:binary(refresh[name])
 assert refresh['rechecked']==['release-initializers','sanitized-initializers']
 for row in v['binaries']+v['sources']+v['coverage']:binary(row)
 for row in v['checks']:checked(row)
 logs={r['name']:Path(r['log']).read_text() for r in v['checks']}
 assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in logs['through']
 assert '349 source status/output/sanitizer checks PASS' in logs['sanitizer-parity']
 assert '349 course-contract parity checks PASS; 0 presentation differences' in logs['entry-parity']
 for label in ['release','sanitized']:
  assert '82 initialization checks PASS' in logs[label+'-initializers']
  assert '64 statement checks PASS' in logs[label+'-statements']
  row=next(r for r in v['checks'] if r['name']==label+'-initializers')
  controls=json.loads((Path(row['command'][-1])/'checks.json').read_text())
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_initialization_facts.py')
  binary(controls['binary']);assert len(controls['checks'])==82
  assert len({r['source_path'] for r in controls['checks']})==82
  assert sum(r['reject'] for r in controls['checks'])==71
  for control in controls['checks']:
   assert shared.sha(control['source_path'])==control['source_sha256']
   assert shared.sha(control['log'])==control['log_sha256']
   diagnostic=Path(control['log']).read_text()
   assert 'Sanitizer' not in diagnostic and 'runtime error:' not in diagnostic
   assert control['exit_code']==int(control['reject'])
   if not control['reject']:binary(control['native']);assert control['native']['exit_code']==0
 layout=document('initializer-layout.json');old=document('body-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/initializer_layout.py')==layout['harness_sha256']
 assert layout['layouts'][0]['sizes']==old['layouts'][1]['sizes']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   if row['label']=='current':assert shared.sha(h['source'])==h['sha256']
  for k in ['binary','dump']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 a,b=[r['sizes'] for r in layout['layouts']]
 assert a[:12]==b[:12] and a[13:]==b[13:] and [a[12],b[12]]==[6376,6520]
 count=0
 for filename,harness,expected in [('initializer-performance.json','initializer_benchmark_audit.py',27),('initializer-noise.json','initializer_audit_noise.py',6)]:
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
   for out in row['outputs']:
    binary(out);assert Path(out['path']).stat().st_size==out['bytes']
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
   if row.get('shape'):
    n,k,m,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
    assert stats['semantic_initializer_recipe_work']==2*m
    assert stats['semantic_initializer_recipe_uses']==2*k*m
    assert stats['semantic_body_checks']==stats['semantic_body_lifetime_checks']
 assert count==616
 proof=document('initializer-proofs.json')
 for name in ['harness','validation','control_manifest','standard','remaining_manifest']:binary(proof[name])
 for row in proof['binaries']:binary(row)
 assert len(proof['cases'])==82 and len(proof['demand_controls'])==4 and len(proof['remaining'])==4
 assert sum(r['required_exit']==1 and r['entry_exit']==0 for r in proof['cases'])==71
 for row in proof['cases']:
  binary(row['source']);binary(row['entry_log']);assert row['current_exit']==row['required_exit']
  for output in row.get('outputs',[]):binary(output)
  for name in ['entry_native','current_native']:
   if name in row:binary(row[name])
  if row.get('comparison'):
   comparison=row['comparison'];assert comparison['exit_code']==0
   assert shared.sha(comparison['log'])==comparison['log_sha256']
   for output in comparison['canonical']:binary(output)
   assert comparison['canonical'][0]['sha256']==comparison['canonical'][1]['sha256']
 for row in proof['demand_controls']:
  binary(row['source']);assert len(row['outputs'])==3
  for output in row['outputs']:
   binary(output['binary']);binary(output['log']);assert output['exit_code']==row['required_exit']
   diagnostic=Path(output['log']['path']).read_text()
   assert 'Sanitizer' not in diagnostic and 'runtime error:' not in diagnostic
 for row in proof['remaining']:
  assert Path(row['source_path']).read_text()==row['source'] and row['exit_code']==0
 modes=document('initializer-mode-handoff.json')
 assert shared.sha(modes['binary'])==modes['binary_sha256']==v['binaries'][1]['sha256']
 assert len(modes['cases'])==4
 for row in modes['cases']:
  assert Path(row['source_path']).read_text()==row['source'] and row['exit_code']!=row['required_exit']
 print('616 initializer observations, 67 checks, 349 sanitizer/parity inputs, 164 source controls and 24 live headers verified')
 return count
if __name__=='__main__':verify()

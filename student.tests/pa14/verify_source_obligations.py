#!/usr/bin/env python3
"""Verify source ownership, complete validation and every frozen observation."""
from pathlib import Path
import gzip,hashlib,json
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign

def archive(row):
 p=Path(row['archive_path']);assert shared.sha(p)==row['archive_sha256']
 assert p.stat().st_size==row['archive_bytes']
 h=hashlib.sha256();size=0
 with gzip.open(p,'rb') as f:
  for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk);size+=len(chunk)
 assert h.hexdigest()==row['sha256'] and size==row['bytes']

def verify():
 v=document('source-obligations-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/source_obligations_validation.py')==v['harness_sha256']
 assert shared.sha(ROOT/'dev/cppgm++')==v['binaries'][1]['sha256']
 assert len(v['checks'])==79 and len(v['coverage'])==1266
 for row in v['binaries']+v['sources']+v['coverage']:binary(row)
 for row in v['checks']:checked(row)
 for row in v['archives']:archive(row)
 archived={r['path']:r for r in v['archives']}
 logs={r['name']:Path(r['log']).read_text() for r in v['checks']}
 assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in logs['through']
 assert '349 source status/output/sanitizer checks PASS' in logs['sanitizer-parity']
 assert '349 course-contract parity checks PASS' in logs['entry-parity']
 for label in ['release','sanitized']:
  row=next(r for r in v['checks'] if r['name']==label+'-source-obligations')
  controls=json.loads((Path(row['command'][-1])/'checks.json').read_text())
  binary(controls['binary']);assert len(controls['checks'])==242
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_source_obligations.py')
  assert len({r['source_path'] for r in controls['checks']})==242
  for control in controls['checks']:
   assert shared.sha(control['source_path'])==control['source_sha256']
   assert shared.sha(control['log'])==control['log_sha256']
   assert control['exit_code']==int(control['reject'])
   if 'native' in control:binary(control['native']);assert control['native']['exit_code']==0
   if 'backend_limitation' in control:
    limit=control['backend_limitation'];assert limit['exit_code']==1
    assert shared.sha(limit['log'])==limit['log_sha256']
    assert 'undefined native symbol: pure_virtual' in Path(limit['log']).read_text()
  row=next(r for r in v['checks'] if r['name']==label+'-source-property-cache')
  controls=json.loads((Path(row['command'][2])/'checks.json').read_text())
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_source_property_cache.py')
  assert controls['probe_source_sha256']==shared.sha(ROOT/'student.tests/pa14/source-obligations.cc')
  if controls['binary']['path'] in archived:assert controls['binary']['sha256']==archived[controls['binary']['path']]['sha256']
  else:binary(controls['binary'])
  binary(controls['source']);assert shared.sha(controls['log'])==controls['log_sha256']
  assert '10000 source property queries without body/actions demand; 10000 stable concrete finish queries' in Path(controls['log']).read_text()
  row=next(r for r in v['checks'] if r['name']==label+'-source-conditionals')
  controls=json.loads((Path(row['command'][-1])/'checks.json').read_text())
  binary(controls['binary']);assert len(controls['checks'])==4
  assert controls['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/check_source_conditionals.py')
  for control in controls['checks']:
   assert shared.sha(control['source_path'])==control['source_sha256']
   binary(control['native'])
   for row in control['runs']:checked(row)
 proof=document('source-obligations-proofs.json')
 assert proof['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/source_obligations_proofs.py')
 for key in ['entry','controls','standard','handout','spec']:binary(proof[key])
 assert len(proof['checks'])==242 and sum(r['incorrect'] for r in proof['checks'])==171
 assert sum(r['entry_exit']!=r['required_exit'] for r in proof['checks'])==169
 for row in proof['checks']:
  binary(row['source']);binary(row['log'])
  if 'backend' in row:binary(row['backend']['log'])
  if 'native' in row:binary(row['native']['binary'])
 handoff=document('source-obligations-handoff.json')
 assert shared.sha(handoff['prior_path'])==handoff['prior_sha256']
 prior=json.loads(Path(handoff['prior_path']).read_text())
 assert len(handoff['checks'])==10
 for label in ['release','sanitized']:
  rows=[r for r in handoff['checks'] if r['label']==label]
  assert len(rows)==5 and len({r['source_sha256'] for r in rows})==5
  for row in rows:
   binary(row['binary'])
   assert row['binary']['sha256']==v['binaries'][1 if label=='release' else 2]['sha256']
   assert row['exit_code']==row['required_exit']==1
   assert shared.sha(row['source_path'])==row['source_sha256']
   assert shared.sha(row['log'])==row['log_sha256']
   assert any(r['source_sha256']==row['source_sha256'] for r in prior['cases'])
 journal=document('source-obligations-journal.json')
 for row in journal['files']:binary(row)
 for row in journal['archive_manifests']:
  binary(row)
  for item in json.loads(Path(row['path']).read_text()):archive(item)
 initial=json.loads(Path(journal['initial_validation']['path']).read_text())
 binary(journal['initial_validation'])
 for row in initial['archives']:archive(row)
 layout=document('source-obligations-layout.json')
 assert layout['harness_sha256']==shared.sha(ROOT/'student.tests/pa14/source_obligations_layout.py')
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   if row['label']=='current':assert shared.sha(h['source'])==h['sha256']
  for key in ['binary','dump']:assert shared.sha(row[key+'_path'])==row[key+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
  extra=row['conversion_records']
  for key in ['source','binary']:assert shared.sha(extra[key+'_path'])==extra[key+'_sha256']
  assert list(map(int,shared.run([extra['binary_path']]).stdout.split()))==extra['sizes']==[52,28,8,80]
 a,b=[r['sizes'] for r in layout['layouts']]
 assert [a[8],b[8],a[12],b[12]]==[120,124,6552,6760]
 assert all(x==y for i,(x,y) in enumerate(zip(a,b)) if i not in [8,12])
 count=0
 for filename,harness,total,natives in [('source-obligations-performance.json','source_obligations_benchmark.py',47,14),('source-obligations-noise.json','source_obligations_noise.py',15,4)]:
  data=document(filename)
  assert data['harness_sha256']==shared.sha(ROOT/'student.tests/pa14'/harness)
  assert shared.sha(data['parent_path'])==data['parent_sha256']
  parent=json.loads(Path(data['parent_path']).read_text())
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
  assert len(data['workloads'])==total and sum('runtime' in r for r in data['workloads'].values())==natives
  assert [r['sha256'] for r in data['binaries']]==[r['sha256'] for r in v['binaries'][:2]]
  for row in data['binaries']:binary(row)
  for name,row in data['workloads'].items():
   assert shared.sha(row['source_path'])==row['source_sha256']
   assert [o['binary_sha256'] for o in row['outputs']]==[b['sha256'] for b in data['binaries']]
   if name in parent['workloads']:assert row['source_sha256']==parent['workloads'][name]['source_sha256']
   for output in row['outputs']:binary(output);assert Path(output['path']).stat().st_size==output['bytes']
   if row.get('comparison'):
    comparison=row['comparison'];assert comparison['exit_code']==0
    assert shared.sha(comparison['log'])==comparison['log_sha256']
    assert shared.sha(comparison['source_path'])==comparison['source_sha256']==row['source_sha256']
    for output in comparison['copies']:binary(output)
    assert comparison['canonical'][0]['sha256']==comparison['canonical'][1]['sha256']
    for output in comparison['canonical']:binary(output)
   else:assert row['outputs'][0]['sha256']==row['outputs'][1]['sha256']
   count+=campaign(row['compiler'])
   if 'runtime' in row:
    count+=campaign(row['runtime'])
    for output in row['outputs']:binary(output['native']);assert output['native']['checked_exit']==0
    assert row['outputs'][0]['native']['text_bytes']==row['outputs'][1]['native']['text_bytes']
   if row.get('shape'):
    n,k,m,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
    if row['kind']=='source-default':
     assert stats['semantic_default_initialization_work']==m
     assert stats['semantic_default_initialization_uses']==k*m
    else:
     assert stats['semantic_initializer_recipe_work']==m
     assert stats['semantic_initializer_recipe_uses']==k*m
     assert stats['semantic_template_fixed_calls']==2*m
     assert stats['semantic_template_fixed_call_uses']==2*k*m
    assert stats['semantic_body_checks']==stats['semantic_body_lifetime_checks']
 assert count==1120
 from source_obligations_report import render
 assert (ROOT/'student.tests/pa14/source-obligations-performance.md').read_text()==render()
 print('1120 source-obligation observations, 79 checks, 484 source controls, eight branch programs and terminal property/reuse ownership verified')
 return count
if __name__=='__main__':verify()

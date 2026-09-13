#!/usr/bin/env python3
"""Verify the current body audit while retaining every prior frozen campaign."""
from pathlib import Path
import json,statistics
from verify_special_signatures import ROOT,document,checked,shared
from verify_declaration_facts import binary,campaign,outputs

def verify():
 validation=document('body-audit-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/body_audit_validation.py')==validation['harness_sha256']
 assert shared.sha(ROOT/'dev/cppgm++')==validation['binaries'][1]['sha256']
 for row in validation['binaries']+validation['sources']+validation['coverage']:binary(row)
 assert len(validation['coverage'])==1266
 assert len(validation['checks'])==51
 for row in validation['checks']:checked(row)
 checks={r['name']:Path(r['log']).read_text() for r in validation['checks']}
 assert 'ALL TESTS PASSED SUCCESSFULLY! (1935 / 1935)' in checks['through']
 assert '349 source status/output/sanitizer checks PASS' in checks['sanitizer-parity']
 assert '349 course-contract parity checks PASS; 1 presentation differences' in checks['entry-parity']
 for label in ('release','sanitized'):
  assert '64 statement checks PASS' in checks[label+'-statements']
  assert checks[label+'-body-publication'].count('finish failures 10000 body failures 10000')==8
  assert 'valid finish failures 0 body failures 0' in checks[label+'-body-publication']
 layout=document('body-layout.json')
 assert shared.sha(ROOT/'student.tests/pa14/body_layout.py')==layout['harness_sha256']
 old=document('default-layout.json')
 assert layout['layouts'][0]['sizes']==old['layouts'][1]['sizes']
 for row in layout['layouts']:
  assert row['build_exit']==0 and len(row['headers'])==24
  for h in row['headers']:
   assert shared.sha(h['path'])==h['sha256']
   if row['label']=='current':assert shared.sha(h['source'])==h['sha256']
  for k in ['binary','dump']:assert shared.sha(row[k+'_path'])==row[k+'_sha256']
  assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']
 before,after=[x['sizes'] for x in layout['layouts']]
 assert after[:12]==before[:12] and after[13:]==before[13:]
 assert before[12]==6312 and after[12]==6376 and after[0]==112
 data=document('body-audit-performance.json')
 assert data['binaries'][1]['sha256']==validation['binaries'][1]['sha256']
 assert data['binaries'][0]['sha256']==validation['binaries'][0]['sha256']
 count=0
 for filename,harness,expected in [('body-audit-performance.json','body_benchmark_audit.py',22),('body-audit-noise.json','body_audit_noise.py',6)]:
  data=document(filename)
  assert shared.sha(ROOT/'student.tests/pa14'/harness)==data['harness_sha256']
  assert shared.sha(data['parent_path'])==data['parent_sha256']
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
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
    for output in row['outputs']:binary(output['native'])
    assert row['outputs'][0]['native']['text_bytes']==row['outputs'][1]['native']['text_bytes']
   if row.get('shape'):
    n,k,m,q=row['shape'];stats=row['outputs'][1]['telemetry'][0]
    assert stats['semantic_statement_conversion_work']==m+1
    assert stats['semantic_statement_conversion_uses']==k*(m+1)
    assert stats['semantic_body_checks']==stats['semantic_body_lifetime_checks']
 assert count==532
 print('532 body observations, 349 sanitizer/parity inputs, 128 statement controls, 18 repeated body queries and 24 live headers verified')
 return count
if __name__=='__main__':verify()

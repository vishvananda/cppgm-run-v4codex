#!/usr/bin/env python3
"""Verify retained signatures, complete-class uses, enum identity and acceptance."""
from pathlib import Path
import json,re
from verify_special_signatures import ROOT,document,checked,compared,comparison_sources,shared
from verify_declaration_facts import binary,campaign

def native_output(o):
 checked(o);checked(o['native']);checked(o['executed'])
 assert shared.sha(o['path'])==o['sha256'] and shared.sha(o['native_path'])==o['native_sha256']

def verify():
 proof=document('signature-publication-proofs.json')
 assert shared.sha(ROOT/'student.tests/pa14/signature_publication_evidence.py')==proof['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/check_signature_publications.py')==proof['control_sha256']
 assert shared.sha(proof['standard_path'])==proof['standard_sha256'];comparison_sources(proof)
 for b in proof['binaries']+[proof['declaration_baseline']]:binary(b)
 assert len(proof['positive'])==6 and len(proof['rejections'])==10
 assert [r['name'] for r in proof['rejections'] if r['optional']]==['private_default']
 for row in proof['positive']:
  p=Path(row['source_path']);assert shared.sha(p)==shared.sha(ROOT/'student.tests/pa14'/p.name)==row['source_sha256'] and row['clauses']
  native_output(row['output'])
  for field in ['declaration_output']+(['entry_output'] if 'entry_output' in row else []):
   native_output(row[field])
   if field+'_comparison' in row:compared(row[field+'_comparison'],row['source_sha256'],[row[field],row['output']])
   else:assert row[field]['sha256']==row['output']['sha256']
   assert row[field]['native_sha256']==row['output']['native_sha256']
  if 'entry_rejection' in row:
   r=row['entry_rejection'];assert r['exit_code']==1 and shared.sha(r['log'])==r['log_sha256']
  host=row['host'];checked(host['build']);checked(host['executed']);binary(host)
 from check_signature_publications import CASES,CLAUSES,OPTIONAL
 for row in proof['rejections']:
  p=Path(row['source_path']);assert p.read_text()==CASES[row['name']] and shared.sha(p)==row['source_sha256']
  assert row['clauses']==CLAUSES[row['name']] and row['optional']==(row['name'] in OPTIONAL)
  assert len(row['outputs'])==2
  for o in row['outputs']+[row['host']]:
   assert o['exit_code'] in ((0,1) if row['optional'] else (1,)) and shared.sha(o['log'])==o['log_sha256']
 layout=proof['layout'];checked(layout['build']);assert len(layout['headers'])==18
 for h in layout['headers']:assert shared.sha(h['path'])==shared.sha(h['source'])==h['sha256']
 for k in ('source','binary','dump'):assert shared.sha(layout[k+'_path'])==layout[k+'_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/signature_publication_layout_probe.cc')==layout['source_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56]
 for name,size in (('ExpressionStore::Properties',24),('ExpressionStore::Use',20),('Analyzer',6152),('Analyzer::TemplatePrototype',24),('Analyzer::TemplateClassUse',20),('MemberFacts',124),('TemplateDefinition',32),('Fact',20),('FactStore',56)):
  m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',Path(layout['dump_path']).read_text());assert m,name
  assert int(m[1])==layout['records'][name]['size']==size and int(m[2])==layout['records'][name]['align']
 data=document('signature-publication-performance.json');comparison_sources(data)
 for path,field in ((ROOT/'student.tests/pa14/signature_publication_benchmark.py','harness_sha256'),(ROOT/'student.tests/pa10/benchmark.py','shared_harness_sha256'),(ROOT/'reference-binaries/lowir2native','backend_sha256')):assert shared.sha(path)==data[field]
 assert shared.sha(data['parent_path'])==data['parent_sha256'];parent=json.loads(Path(data['parent_path']).read_text())
 assert len(data['workloads'])==54 and len(parent['workloads'])==49 and all(n in data['workloads'] for n in parent['workloads'])
 assert [b['sha256'] for b in data['binaries']]==[b['sha256'] for b in proof['binaries']]
 assert data['binaries'][0]['sha256']==parent['binaries'][1]['sha256']
 for b in data['binaries']:binary(b)
 native=observations=0
 for name,row in data['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256']
  assert [o['binary_sha256'] for o in row['outputs']]==[b['sha256'] for b in data['binaries']]
  for o in row['outputs']:
   assert shared.sha(o['path'])==o['sha256'] and Path(o['path']).stat().st_size==o['bytes']
   if 'native' in o:binary(o['native']);assert o['native']['checked_exit']==0
  if 'comparison' in row:compared(row['comparison'],row['source_sha256'],row['outputs'])
  else:assert row['outputs'][0]['sha256']==row['outputs'][1]['sha256']
  if name in parent['workloads']:
   assert row['source_sha256']==parent['workloads'][name]['source_sha256']
   assert row['outputs'][0]['sha256']==parent['workloads'][name]['outputs'][1]['sha256']
  b=row['outputs'][1]['telemetry'][0]
  assert 0<b['semantic_fact_records']<=b['semantic_fact_slots']
  assert b['semantic_fact_storage_bytes']>=4*b['semantic_fact_slots']+20*b['semantic_fact_records']
  if name.startswith('signature-uses-'):
   n,k,q=map(int,name.split('-')[-3:]);a=row['outputs'][0]['telemetry'][0]
   assert b['semantic_template_signature_work']==k+1
   assert b['semantic_template_signature_uses']==n*(k+1)
   assert b['semantic_parameter_publications']==6*n*k
   for kind in ('default','initializer'):
    assert b['semantic_template_'+kind+'_binding_work']==b['semantic_template_'+kind+'_binding_queued']==k
   assert a['semantic_template_declaration_work']==b['semantic_template_declaration_work']==10*k+1
   assert a['semantic_declaration_publications']==b['semantic_declaration_publications']==n*(10*k+1)
   assert a['semantic_template_type_work']==8*k+1 and b['semantic_template_type_work']==2*k+1
   assert a['semantic_template_type_uses']==n*(12*k+1) and b['semantic_template_type_uses']==2*n*(2*k+1)
   assert a['semantic_type_substitution_work']==5*n and b['semantic_type_substitution_work']==n*(2*k+6)
   assert a['semantic_type_query_work']==b['semantic_type_query_work']==2
   assert a['semantic_entities']-b['semantic_entities']==6*k*(n-1)
   assert a['semantic_scopes']-b['semantic_scopes']==k*(n+1)
   # Canonical function-type construction is separate from source checking.
   assert a['semantic_signature_work']==n*(2*k+1)+9
   assert b['semantic_signature_work']==n*(3*k+1)+2*k+11
  observations+=campaign(row['compiler'])
  if 'runtime' in row:
   native+=1;assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256'];observations+=campaign(row['runtime'])
 assert native==12 and observations==924
 repeat=document('signature-publication-repeat.json')
 assert repeat['parent_sha256']==shared.sha(repeat['parent_path'])==shared.sha(ROOT/'student.tests/pa14/signature-publication-performance.json')
 assert repeat['binaries']==data['binaries'] and repeat['flags']==data['flags']
 assert shared.sha(ROOT/'student.tests/pa14/signature_publication_repeat.py')==repeat['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==repeat['shared_harness_sha256']
 assert list(repeat['workloads'])==['local-facts-1000-32-4']
 for name,row in repeat['workloads'].items():
  old=data['workloads'][name]
  assert shared.sha(row['source_path'])==row['source_sha256']==old['source_sha256']
  for b,o in enumerate(row['outputs']):
   assert o['binary_sha256']==data['binaries'][b]['sha256']
   assert shared.sha(o['path'])==o['sha256']==old['outputs'][b]['sha256']
   assert Path(o['path']).stat().st_size==o['bytes']
  observations+=campaign(row['compiler'])
 assert observations==938
 validation=document('signature-publication-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/signature_publication_validation.py')==validation['harness_sha256']
 assert validation['control_sha256']==proof['control_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','optional_diagnostics','abi_controls','reducer_controls')]==[314,1621,1935,33,347,166,1,6,7]
 assert len(validation['personal'])==33 and len(validation['coverage'])==1266 and len(validation['checks'])==118
 assert sum(r['path'].endswith('.t') for r in validation['coverage'])==314
 for r in validation['personal']+validation['coverage']:assert shared.sha(ROOT/r['path'])==r['sha256']
 for b in validation['binaries']:binary(b)
 for r in validation['checks']:checked(r)
 assert len(validation['signature_reducers'])==4
 for row in validation['reducers']+validation['signature_reducers']+[validation['owner_control'],validation['initializer_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for o in row['outputs']:
   assert o['native_exit']==0 and shared.sha(o['path'])==o['sha256'] and shared.sha(o['native_path'])==o['native_sha256']
   if 'binary_path' in o:assert shared.sha(o['binary_path'])==o['binary_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 for name in ('store_control','fact_store_control'):
  row=validation[name];assert shared.sha(row['source_path'])==row['source_sha256']
  if 'implementation_path' in row:assert shared.sha(row['implementation_path'])==row['implementation_sha256']
  for o in row['outputs']:binary(o);assert o['exit_code']==0
 handoff=document('signature-publication-handoff.json')
 assert handoff['entry_commit'].startswith('97006205') and handoff['implementation_commit'].startswith('dad19c39')
 assert handoff['release_sha256']==data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']
 assert [c['name'] for c in handoff['checks']]==['stage','prior','through','file_audit','native']
 for c in handoff['checks']:checked(c)
 for r in handoff['initial_observations']+handoff['intermediate_binaries']:binary(r)
 print('938 signature/publication observations, 347 sanitizer inputs, six native proofs, complete-class diagnostic controls and live layouts verified')
 return observations
if __name__=='__main__':verify()

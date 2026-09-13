#!/usr/bin/env python3
"""Verify declaration/fact ownership, complete trials and full-corpus acceptance."""
from pathlib import Path
import json,re,statistics,sys
from verify_special_signatures import ROOT,document,checked,compared,comparison_sources,shared

def campaign(c):
 rows=c['observations']
 assert [r['binary'] for r in rows]==shared.ORDER
 assert [r['binary'] for r in c['warmups']]==[0,1]
 for r in c['warmups']+rows:assert r['wall_s']>0 and r['rss_kib']>0 and r['checked_exit']==0
 assert c['aa_range_s']==[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
 assert c['paired_b_over_a']==[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
 return len(c['warmups'])+len(rows)
def binary(row):
 assert shared.sha(row['path'])==row['sha256']
 if 'text_bytes' in row:assert shared.text_size(row['path'])==row['text_bytes']
def outputs(row):
 for o in row['outputs']:
  assert shared.sha(o['path'])==o['sha256']
  if 'log' in o:assert shared.sha(o['log'])==o['log_sha256']
  if 'native' in o:binary(o['native']);assert o['native']['checked_exit']==0
 assert len(set(o['sha256'] for o in row['outputs']))==1

def verify():
 proof=document('declaration-fact-proofs.json')
 assert shared.sha(ROOT/'student.tests/pa14/declaration_fact_evidence.py')==proof['harness_sha256']
 assert shared.sha(proof['standard_path'])==proof['standard_sha256']
 comparison_sources(proof)
 for b in proof['binaries']+[proof['declaration_baseline']]:binary(b)
 assert len(proof['positive'])==2 and not proof['rejections']
 for row in proof['positive']:
  p=Path(row['source_path']);assert shared.sha(p)==shared.sha(ROOT/'student.tests/pa14'/p.name)==row['source_sha256'] and row['clauses']
  assert row['entry_rejection']['exit_code']==1
  assert shared.sha(row['entry_rejection']['log'])==row['entry_rejection']['log_sha256']
  assert len(row['outputs'])==1
  for o in row['outputs']+[row['declaration_output']]:
   checked(o);checked(o['native']);checked(o['executed'])
   assert shared.sha(o['path'])==o['sha256'] and shared.sha(o['native_path'])==o['native_sha256']
  if 'declaration_comparison' in row:compared(row['declaration_comparison'],row['source_sha256'],[row['declaration_output'],row['outputs'][0]])
  else:assert row['declaration_output']['sha256']==row['outputs'][0]['sha256']
  assert row['declaration_output']['native_sha256']==row['outputs'][0]['native_sha256']
  host=row['host'];checked(host['build']);checked(host['executed']);binary(host)
 layout=proof['layout'];checked(layout['build']);assert len(layout['headers'])==18
 for h in layout['headers']:assert shared.sha(h['path'])==shared.sha(h['source'])==h['sha256']
 for k in ('source','binary','dump'):assert shared.sha(layout[k+'_path'])==layout[k+'_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/declaration_fact_layout_probe.cc')==layout['source_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48,124,32,20,56]
 for name,size in (('ExpressionStore::Properties',24),('ExpressionStore::Use',20),('Analyzer',6000),('Analyzer::TemplatePrototype',24),('MemberFacts',124),('TemplateDefinition',32),('Fact',20),('FactStore',56)):
  m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',Path(layout['dump_path']).read_text());assert m,name
  assert int(m[1])==layout['records'][name]['size']==size and int(m[2])==layout['records'][name]['align']
 observations=0
 for suffix in ('sparse','views'):
  trial=document('declaration-fact-trial-'+suffix+'.json')
  assert shared.sha(ROOT/'student.tests/pa14/declaration_fact_trial.py')==trial['harness_sha256']
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==trial['shared_harness_sha256']
  assert shared.sha(trial['parent_path'])==trial['parent_sha256'];parent=json.loads(Path(trial['parent_path']).read_text())
  assert len(trial['workloads'])==3
  for b in trial['binaries']:binary(b)
  assert trial['binaries'][0]['sha256']==proof['binaries'][0]['sha256']
  if suffix=='views':assert trial['binaries'][1]['sha256']==proof['binaries'][1]['sha256']
  for name,row in trial['workloads'].items():
   assert shared.sha(row['source_path'])==row['source_sha256']==parent['workloads'][name]['source_sha256']
   outputs(row);assert row['outputs'][0]['sha256']==parent['workloads'][name]['outputs'][-1]['sha256']
   observations+=campaign(row['compiler'])
 assert observations==84
 data=document('declaration-fact-performance.json');comparison_sources(data)
 for path,field in ((ROOT/'student.tests/pa14/declaration_fact_benchmark.py','harness_sha256'),(ROOT/'student.tests/pa10/benchmark.py','shared_harness_sha256'),(ROOT/'reference-binaries/lowir2native','backend_sha256')):assert shared.sha(path)==data[field]
 assert shared.sha(data['parent_path'])==data['parent_sha256'];parent=json.loads(Path(data['parent_path']).read_text())
 assert len(data['workloads'])==49 and len(parent['workloads'])==44 and all(n in data['workloads'] for n in parent['workloads'])
 assert [b['sha256'] for b in data['binaries']]==[b['sha256'] for b in proof['binaries']]
 for b in data['binaries']:binary(b)
 native=0
 for name,row in data['workloads'].items():
  assert shared.sha(row['source_path'])==row['source_sha256'];outputs(row)
  assert [o['binary_sha256'] for o in row['outputs']]==[b['sha256'] for b in data['binaries']]
  for o in row['outputs']:
   assert Path(o['path']).stat().st_size==o['bytes']
   if name in parent['workloads']:assert o['sha256']==parent['workloads'][name]['outputs'][-1]['sha256']
  if name in parent['workloads']:assert row['source_sha256']==parent['workloads'][name]['source_sha256']
  b=row['outputs'][1]['telemetry'][0]
  assert 0<b['semantic_fact_records']<=b['semantic_fact_slots']
  assert b['semantic_fact_storage_bytes']>=4*b['semantic_fact_slots']+20*b['semantic_fact_records']
  if name.startswith('local-facts-'):
   n,k,q=map(int,name.split('-')[-3:]);a=row['outputs'][0]['telemetry'][0]
   assert b['semantic_template_declaration_work']==8*k+1 and b['semantic_declaration_publications']==n*(8*k+1)
   assert b['semantic_type_substitution_work']==b['semantic_type_substitution_records']==n*(4*k+1)
   assert b['semantic_type_query_work']==k*(3*n+4)
   assert a['semantic_template_type_work']==b['semantic_template_type_work']==4*k
   assert b['semantic_template_type_uses']==8*n*k and a['semantic_template_type_uses']==3*n*k
   assert b['semantic_fact_records']==n*(22+k*(27+14*q))+9+k*(27+12*q)
   assert b['semantic_fact_slots']==n*(65+k*(93+28*q))+28+k*(97+28*q)
  observations+=campaign(row['compiler'])
  if 'runtime' in row:
   native+=1;assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256'];observations+=campaign(row['runtime'])
 assert native==11 and observations==924
 validation=document('declaration-fact-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/declaration_fact_validation.py')==validation['harness_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','abi_controls','reducer_controls')]==[314,1621,1935,31,345,157,6,7]
 assert len(validation['personal'])==31 and len(validation['coverage'])==1266 and len(validation['checks'])==92
 assert sum(r['path'].endswith('.t') for r in validation['coverage'])==314
 for r in validation['personal']+validation['coverage']:assert shared.sha(ROOT/r['path'])==r['sha256']
 for b in validation['binaries']:binary(b)
 for r in validation['checks']:checked(r)
 for row in validation['reducers']+[validation['owner_control'],validation['initializer_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for o in row['outputs']:
   assert o['native_exit']==0 and shared.sha(o['path'])==o['sha256'] and shared.sha(o['native_path'])==o['native_sha256']
   if 'binary_path' in o:assert shared.sha(o['binary_path'])==o['binary_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 for name in ('store_control','fact_store_control'):
  row=validation[name];assert shared.sha(row['source_path'])==row['source_sha256']
  if 'implementation_path' in row:assert shared.sha(row['implementation_path'])==row['implementation_sha256']
  for o in row['outputs']:binary(o);assert o['exit_code']==0
 handoff=document('declaration-fact-handoff.json')
 assert handoff['entry_commit'].startswith('fda0a178') and handoff['implementation_commit'].startswith('af01c062')
 assert handoff['release_sha256']==data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']
 assert [c['name'] for c in handoff['checks']]==['stage','prior','through','file_audit','native']
 for c in handoff['checks']:checked(c)
 for r in handoff['initial_observations']+handoff['intermediate_binaries']:binary(r)
 print('924 declaration/fact observations, 345 sanitizer inputs, initializer/native proofs, sparse ownership equations and live layouts verified')
 return observations
if __name__=='__main__':verify()

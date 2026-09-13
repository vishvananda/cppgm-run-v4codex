#!/usr/bin/env python3
"""Verify ordinary definition ownership and all unfiltered performance evidence."""
from pathlib import Path
import json,re,statistics,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
def document(name):return json.loads((ROOT/'student.tests/pa14'/name).read_text())
def checked(row):
 assert row['exit_code']==0 and shared.sha(row['log'])==row['log_sha256']
def verify():
 proof=document('definition-demand-proofs.json')
 for name,field in (('definition_demand_evidence.py','harness_sha256'),('check_definition_demands.py','control_sha256')):
  assert shared.sha(ROOT/'student.tests/pa14'/name)==proof[field]
 assert shared.sha(proof['standard_path'])==proof['standard_sha256']
 for binary in proof['binaries']:assert shared.sha(binary['path'])==binary['sha256']
 assert len(proof['rejections'])==12 and len(proof['positive'])==4
 for row in proof['rejections']:
  assert shared.sha(row['source_path'])==row['source_sha256'] and row['clauses']
  assert [o['exit_code'] for o in row['outputs']]==[0,1] and row['host']['exit_code']==1
  for out in row['outputs']+[row['host']]:
   assert shared.sha(out['log'])==out['log_sha256']
   if 'path' in out:assert shared.sha(out['path'])==out['sha256']
 for row in proof['positive']:
  assert shared.sha(row['source_path'])==shared.sha(ROOT/f"student.tests/pa14/{row['name']}.cpp")==row['source_sha256']
  for out in row['outputs']:
   checked(out);checked(out['native']);checked(out['executed'])
   assert shared.sha(out['path'])==out['sha256'] and shared.sha(out['native_path'])==out['native_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
  if row['name']=='definition-overloads':
   a,b=[out['telemetry'][0] for out in row['outputs']]
   assert a['template_definition_applications']==6 and b['template_definition_applications']==b['template_definition_direct_applications']==b['template_definition_edges']==2
   assert b['template_definition_signature_work']==3
 layout=proof['layout'];checked(layout['build'])
 for header in layout['headers']:assert shared.sha(header['path'])==shared.sha(header['source'])==header['sha256']
 assert len(layout['headers'])==17
 for kind in ('source','binary','dump'):assert shared.sha(layout[kind+'_path'])==layout[kind+'_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/definition_demand_layout_probe.cc')==layout['source_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48,124,32]
 for name,size in (('ExpressionStore::Properties',24),('ExpressionStore::Use',20),('Analyzer',5920),('Analyzer::TemplatePrototype',24),('MemberFacts',124),('TemplateDefinition',32)):
  m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',Path(layout['dump_path']).read_text());assert m,name
  assert int(m[1])==layout['records'][name]['size']==size and int(m[2])==layout['records'][name]['align']
 data=document('definition-demand-performance.json')
 for path,field in ((ROOT/'student.tests/pa14/definition_demand_benchmark.py','harness_sha256'),(ROOT/'student.tests/pa10/benchmark.py','shared_harness_sha256'),(ROOT/'reference-binaries/lowir2native','backend_sha256')):
  assert shared.sha(path)==data[field]
 assert shared.sha(data['parent_path'])==data['parent_sha256'];parent=json.loads(Path(data['parent_path']).read_text())
 assert len(data['workloads'])==37 and len(parent['workloads'])==32
 assert all(name in data['workloads'] for name in parent['workloads'])
 for binary in data['binaries']:assert shared.sha(binary['path'])==binary['sha256'] and shared.text_size(binary['path'])==binary['text_bytes']
 assert [b['sha256'] for b in data['binaries']]==[b['sha256'] for b in proof['binaries']]
 count=0;native_count=0
 for name,work in data['workloads'].items():
  assert shared.sha(work['source_path'])==work['source_sha256'] and len(work['outputs'])==2
  if name in parent['workloads']:assert work['source_sha256']==parent['workloads'][name]['source_sha256']
  for out in work['outputs']:
   assert shared.sha(out['path'])==out['sha256'] and Path(out['path']).stat().st_size==out['bytes']
   if name in parent['workloads']:assert out['sha256']==parent['workloads'][name]['outputs'][-1]['sha256']
   if 'native' in out:
    native=out['native'];assert native['checked_exit']==0 and shared.sha(native['path'])==native['sha256'] and shared.text_size(native['path'])==native['text_bytes']
  assert len(set(o['sha256'] for o in work['outputs']))==1
  if 'runtime' in work:
   native_count+=1;assert len(set(o['native']['sha256'] for o in work['outputs']))==1
  for campaign in [work['compiler']]+([work['runtime']] if 'runtime' in work else []):
   rows=campaign['observations'];assert [r['binary'] for r in rows]==shared.ORDER and [r['binary'] for r in campaign['warmups']]==[0,1]
   for row in campaign['warmups']+rows:assert row['wall_s']>0 and row['rss_kib']>0 and row['checked_exit']==0;count+=1
   assert campaign['aa_range_s']==[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
   assert campaign['paired_b_over_a']==[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
  if name.startswith('demand-uses-'):
   n,width,requests=map(int,name.split('-')[-3:]);a,b=[o['telemetry'][0] for o in work['outputs']]
   assert a['template_definition_applications']==n*width
   assert b['template_definition_applications']==b['template_definition_direct_applications']==b['template_definition_edges']==n
   assert b['template_definition_signature_requests']==b['template_definition_signature_work']==width
   assert a['semantic_candidate_work']==b['semantic_candidate_work']==n*width*requests
   assert b['template_definition_requests']==n*(requests+1) and b['template_definition_hits']==n*requests
   assert a['template_occurrences']==n*(53*width+17) and b['template_occurrences']==n*(22*width+48)
 assert count==644 and native_count==9
 validation=document('definition-demand-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/definition_demand_validation.py')==validation['harness_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','abi_controls','reducer_controls')]==[314,1621,1935,28,342,136,6,7]
 for binary in validation['binaries']:assert shared.sha(binary['path'])==binary['sha256']
 assert len(validation['personal'])==28 and len(validation['coverage'])==1266
 for row in validation['coverage']+validation['personal']:assert shared.sha(ROOT/row['path'])==row['sha256']
 assert sum(row['path'].endswith('.t') for row in validation['coverage'])==314
 assert len(validation['checks'])==80 and len(validation['reducers'])==7
 for row in validation['checks']:checked(row)
 for row in validation['reducers']+[validation['owner_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for out in row['outputs']:
   assert out['native_exit']==0 and shared.sha(out['path'])==out['sha256'] and shared.sha(out['native_path'])==out['native_sha256']
   if 'binary_path' in out:assert shared.sha(out['binary_path'])==out['binary_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 control=validation['store_control'];assert shared.sha(control['source_path'])==control['source_sha256']
 for out in control['outputs']:assert out['exit_code']==0 and shared.sha(out['path'])==out['sha256']
 handoff=document('definition-demand-handoff.json')
 assert handoff['entry_commit'].startswith('167f5f43') and handoff['implementation_commit'].startswith('0fc50a95')
 assert handoff['release_sha256']==data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']
 assert [row['name'] for row in handoff['checks']]==['stage','prior','through','file_audit','native']
 for row in handoff['checks']:checked(row)
 for row in handoff['initial_observations']+handoff['intermediate_binaries']:assert shared.sha(row['path'])==row['sha256']
 print('644 definition-demand observations, twelve rejection proofs, 342 sanitizer inputs and current layout/parameter/lifetime controls verified')
 return count
if __name__=='__main__':verify()

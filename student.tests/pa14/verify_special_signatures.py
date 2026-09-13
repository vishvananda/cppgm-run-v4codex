#!/usr/bin/env python3
"""Verify special signatures, head frames and unfiltered performance evidence."""
from pathlib import Path
import json,re,statistics,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
def document(name):return json.loads((ROOT/'student.tests/pa14'/name).read_text())
def checked(row):
 assert row['exit_code']==0 and shared.sha(row['log'])==row['log_sha256']
def compared(row,source_sha,outputs):
 checked(row)
 assert shared.sha(row['source_path'])==row['source_sha256']==source_sha
 assert len(row['copies'])==len(row['canonical'])==2
 for copy,out in zip(row['copies'],outputs):assert shared.sha(copy['path'])==copy['sha256']==out['sha256']
 for item in row['canonical']:assert shared.sha(item['path'])==item['sha256']
 assert row['canonical'][0]['sha256']==row['canonical'][1]['sha256']
def comparison_sources(data):
 for name,field in (('special_signature_compare.py','comparison_harness_sha256'),('special_signature_compare.pl','comparison_adapter_sha256')):
  assert shared.sha(ROOT/'student.tests/pa14'/name)==data[field]
 assert shared.sha(ROOT/'scripts/compare_results_common.pl')==data['comparison_sha256']
 assert shared.sha(ROOT/'pa8/lowir.md')==data['contract_sha256']
def verify():
 proof=document('special-signature-proofs.json')
 for name,field in (('special_signature_evidence.py','harness_sha256'),('check_special_signatures.py','control_sha256')):
  assert shared.sha(ROOT/'student.tests/pa14'/name)==proof[field]
 assert shared.sha(proof['standard_path'])==proof['standard_sha256']
 for binary in proof['binaries']:assert shared.sha(binary['path'])==binary['sha256']
 comparison_sources(proof)
 assert len(proof['rejections'])==21 and len(proof['positive'])==2
 for row in proof['rejections']:
  assert shared.sha(row['source_path'])==row['source_sha256'] and row['clauses']
  assert [o['exit_code'] for o in row['outputs']]==[1 if row['name']=='conversion_noexcept' else 0,1] and row['host']['exit_code']==1
  for out in row['outputs']+[row['host']]:
   assert shared.sha(out['log'])==out['log_sha256']
   if 'path' in out:assert shared.sha(out['path'])==out['sha256']
 for row in proof['positive']:
  assert shared.sha(row['source_path'])==shared.sha(ROOT/f"student.tests/pa14/{row['name']}.cpp")==row['source_sha256']
  assert row['entry_rejection']['exit_code']==1 and shared.sha(row['entry_rejection']['log'])==row['entry_rejection']['log_sha256']
  assert len(row['outputs'])==1
  for out in row['outputs']+[row['head_output']]:
   checked(out);checked(out['native']);checked(out['executed'])
   assert shared.sha(out['path'])==out['sha256'] and shared.sha(out['native_path'])==out['native_sha256']
  if 'head_comparison' in row:compared(row['head_comparison'],row['source_sha256'],[row['head_output'],row['outputs'][0]])
  else:assert row['head_output']['sha256']==row['outputs'][0]['sha256']
  assert row['head_output']['native_sha256']==row['outputs'][0]['native_sha256']
  host=row['host'];checked(host['build']);checked(host['executed']);assert shared.sha(host['path'])==host['sha256']
 layout=proof['layout'];checked(layout['build'])
 # Declaration-fact probes validate current headers; preserve this completed
 # campaign's exact frozen source/layout snapshot.
 for header in layout['headers']:assert shared.sha(header['path'])==header['sha256']
 assert len(layout['headers'])==17
 for kind in ('source','binary','dump'):assert shared.sha(layout[kind+'_path'])==layout[kind+'_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/special_signature_layout_probe.cc')==layout['source_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48,124,32]
 for name,size in (('ExpressionStore::Properties',24),('ExpressionStore::Use',20),('Analyzer',5920),('Analyzer::TemplatePrototype',24),('MemberFacts',124),('TemplateDefinition',32)):
  m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',Path(layout['dump_path']).read_text());assert m,name
  assert int(m[1])==layout['records'][name]['size']==size and int(m[2])==layout['records'][name]['align']
 data=document('special-signature-performance.json')
 comparison_sources(data)
 for path,field in ((ROOT/'student.tests/pa14/special_signature_benchmark.py','harness_sha256'),(ROOT/'student.tests/pa10/benchmark.py','shared_harness_sha256'),(ROOT/'reference-binaries/lowir2native','backend_sha256')):
  assert shared.sha(path)==data[field]
 assert shared.sha(data['parent_path'])==data['parent_sha256'];parent=json.loads(Path(data['parent_path']).read_text())
 assert len(data['workloads'])==44 and len(parent['workloads'])==37
 assert all(name in data['workloads'] for name in parent['workloads'])
 for binary in data['binaries']:assert shared.sha(binary['path'])==binary['sha256'] and shared.text_size(binary['path'])==binary['text_bytes']
 assert [b['sha256'] for b in data['binaries']]==[b['sha256'] for b in proof['binaries']]
 head=data['head_baseline'];assert head==proof['head_baseline'] and shared.sha(head['path'])==head['sha256'] and shared.text_size(head['path'])==head['text_bytes']
 count=0;native_count=0
 for name,work in data['workloads'].items():
  assert shared.sha(work['source_path'])==work['source_sha256'] and len(work['outputs'])==2
  if name in parent['workloads']:assert work['source_sha256']==parent['workloads'][name]['source_sha256']
  expected_binaries=[head['sha256'],data['binaries'][1]['sha256']] if name.startswith('special-heads-') else [b['sha256'] for b in data['binaries']]
  assert [o['binary_sha256'] for o in work['outputs']]==expected_binaries
  for out in work['outputs']:
   assert shared.sha(out['path'])==out['sha256'] and Path(out['path']).stat().st_size==out['bytes']
   if name in parent['workloads']:assert out['sha256']==parent['workloads'][name]['outputs'][-1]['sha256']
   if 'native' in out:
    native=out['native'];assert native['checked_exit']==0 and shared.sha(native['path'])==native['sha256'] and shared.text_size(native['path'])==native['text_bytes']
  if 'comparison' in work:compared(work['comparison'],work['source_sha256'],work['outputs'])
  else:assert len(set(o['sha256'] for o in work['outputs']))==1
  if 'runtime' in work:
   native_count+=1;assert len(set(o['native']['sha256'] for o in work['outputs']))==1
  for campaign in [work['compiler']]+([work['runtime']] if 'runtime' in work else []):
   rows=campaign['observations'];assert [r['binary'] for r in rows]==shared.ORDER and [r['binary'] for r in campaign['warmups']]==[0,1]
   for row in campaign['warmups']+rows:assert row['wall_s']>0 and row['rss_kib']>0 and row['checked_exit']==0;count+=1
   assert campaign['aa_range_s']==[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
   assert campaign['paired_b_over_a']==[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
  if name.startswith('special-uses-'):
   n,width,requests=map(int,name.split('-')[-3:]);a,b=[o['telemetry'][0] for o in work['outputs']]
   assert a['template_definition_applications']==n*(width+1)
   assert b['template_definition_applications']==b['template_definition_direct_applications']==2*n
   assert b['template_definition_signature_requests']==b['template_definition_signature_work']==width+1
   assert a['semantic_candidate_work']==b['semantic_candidate_work']==n*width*requests
   assert a['semantic_substitution_frames']==n*(width+2) and b['semantic_substitution_frames']==3*n
   assert b['template_definition_edges']==2*n and b['template_definition_requests']==2*n*(requests+1) and b['template_definition_hits']==2*n*requests
   assert a['template_occurrences']==n*(47*width+52) and b['template_occurrences']==n*(17*width+82)
  if name.startswith('special-heads-'):
   n=int(name.rsplit('-',1)[1]);a,b=[o['telemetry'][0] for o in work['outputs']]
   assert a['template_definition_applications']==b['template_definition_applications']==5*n
   assert a['template_definition_direct_applications']==n and b['template_definition_direct_applications']==4*n
   assert a['semantic_substitution_frames']==b['semantic_substitution_frames']==6*n
   assert a['template_occurrences']==b['template_occurrences']==224*n
   assert a['template_definition_signature_work']==b['template_definition_signature_work']==4
 assert count==756 and native_count==10
 validation=document('special-signature-validation.json')
 assert shared.sha(ROOT/'student.tests/pa14/special_signature_validation.py')==validation['harness_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','abi_controls','reducer_controls')]==[314,1621,1935,30,344,157,6,7]
 for binary in validation['binaries']:assert shared.sha(binary['path'])==binary['sha256']
 assert len(validation['personal'])==30 and len(validation['coverage'])==1266
 for row in validation['coverage']+validation['personal']:assert shared.sha(ROOT/row['path'])==row['sha256']
 assert sum(row['path'].endswith('.t') for row in validation['coverage'])==314
 assert len(validation['checks'])==82 and len(validation['reducers'])==7
 for row in validation['checks']:checked(row)
 for row in validation['reducers']+[validation['owner_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for out in row['outputs']:
   assert out['native_exit']==0 and shared.sha(out['path'])==out['sha256'] and shared.sha(out['native_path'])==out['native_sha256']
   if 'binary_path' in out:assert shared.sha(out['binary_path'])==out['binary_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 control=validation['store_control'];assert shared.sha(control['source_path'])==control['source_sha256']
 for out in control['outputs']:assert out['exit_code']==0 and shared.sha(out['path'])==out['sha256']
 handoff=document('special-signature-handoff.json')
 assert handoff['entry_commit'].startswith('60cf761c') and handoff['implementation_commit'].startswith('5b947a18')
 assert handoff['release_sha256']==data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']
 assert [row['name'] for row in handoff['checks']]==['stage','prior','through','file_audit','native']
 for row in handoff['checks']:checked(row)
 for row in handoff['initial_observations']+handoff['intermediate_binaries']:assert shared.sha(row['path'])==row['sha256']
 print('756 special-signature observations, 20 new rejection proofs, 344 sanitizer inputs, typed head/native controls and frozen layouts verified')
 return count
if __name__=='__main__':verify()

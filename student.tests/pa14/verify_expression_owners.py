#!/usr/bin/env python3
"""Verify the immutable property, concrete use and contextual input evidence."""
from pathlib import Path
import json,statistics,sys,re
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
def verify(performance_name='expression-owner-performance.json', validation_name='expression-owner-validation.json',
           handoff_name='expression-owner-handoff.json', implementation='c4e4e6f4'):
 layout=json.loads((ROOT/'student.tests/pa14/expression-owner-layout.json').read_text())
 # This completed campaign owns its frozen header snapshot. The definition
 # demand probe now checks live headers, including its new prototype records.
 for header in layout['headers']:assert shared.sha(header['path'])==header['sha256']
 for kind in ('source','binary','dump'):assert shared.sha(layout[kind+'_path'])==layout[kind+'_sha256']
 assert shared.sha(ROOT/'student.tests/pa14/expression_owner_layout_probe.cc')==layout['source_sha256']
 assert list(map(int,shared.run([layout['binary_path']]).stdout.split()))==layout['sizes']==[112,36,36,20,8,504,48,48]
 for name,expected in (('ExpressionStore::Properties',24),('ExpressionStore::Use',20),('Analyzer',5680)):
  m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',Path(layout['dump_path']).read_text());assert m
  assert int(m[1])==layout['records'][name]['size']==expected and int(m[2])==layout['records'][name]['align']
 data=json.loads((ROOT/'student.tests/pa14'/performance_name).read_text())
 assert shared.sha(ROOT/'student.tests/pa14/expression_owner_benchmark.py')==data['harness_sha256']
 assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
 assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
 assert shared.sha(data['parent_path'])==data['parent_sha256']
 parent=json.loads(Path(data['parent_path']).read_text());assert len(data['workloads'])==32
 for binary in data['binaries']:
  assert shared.sha(binary['path'])==binary['sha256'] and shared.text_size(binary['path'])==binary['text_bytes']
 count=0
 for name,work in data['workloads'].items():
  assert shared.sha(work['source_path'])==work['source_sha256'] and len(work['outputs'])==2
  if name in parent['workloads']:assert work['source_sha256']==parent['workloads'][name]['source_sha256']
  for out in work['outputs']:
   assert shared.sha(out['path'])==out['sha256'] and Path(out['path']).stat().st_size==out['bytes']
   if name in parent['workloads']:assert out['sha256']==parent['workloads'][name]['outputs'][-1]['sha256']
   if 'native' in out:
    native=out['native'];assert native['checked_exit']==0 and shared.sha(native['path'])==native['sha256'] and shared.text_size(native['path'])==native['text_bytes']
  assert len(set(out['sha256'] for out in work['outputs']))==1
  if 'runtime' in work:assert len(set(out['native']['sha256'] for out in work['outputs']))==1
  for campaign in [work['compiler']]+([work['runtime']] if 'runtime' in work else []):
   rows=campaign['observations'];assert [r['binary'] for r in rows]==shared.ORDER and [r['binary'] for r in campaign['warmups']]==[0,1]
   for row in campaign['warmups']+rows:
    assert row['wall_s']>0 and row['rss_kib']>0 and row['checked_exit']==0;count+=1
   assert campaign['aa_range_s']==[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
   assert campaign['paired_b_over_a']==[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
  a,b=[out['telemetry'][0] for out in work['outputs']]
  for key in ('semantic_expression_work','semantic_conversion_work','template_occurrences','semantic_type_query_work','semantic_value_query_work'):
   assert a[key]==b[key],(name,key,a[key],b[key])
  assert b['semantic_expression_slots']==b['parsed_nodes']+b['template_occurrences']
  assert b['semantic_expression_uses']<=b['semantic_expression_slots']
  if name.startswith(('body-large-','value-offset-','input-uses-')):
   n,width=map(int,name.split('-')[-2:])
   if name.startswith('body-large-'):
    assert b['semantic_expression_facts']==4*n+4*width+4
    assert b['semantic_expression_uses']==n*(3*width+5)+3*width+4
   elif name.startswith('value-offset-'):
    assert b['semantic_expression_facts']==3*n+6*width+1
    assert b['semantic_expression_uses']==n*(5*width+4)+5*width+1
   else:
    assert b['semantic_expression_facts']==3*n+5*width+8
    assert b['semantic_expression_uses']==n*(5*width+6)+5*width+7
    assert b['semantic_call_argument_edges']==n+2*width
 assert count==560
 validation=json.loads((ROOT/'student.tests/pa14'/validation_name).read_text())
 assert shared.sha(ROOT/'student.tests/pa14/expression_owner_validation.py')==validation['harness_sha256']
 assert [validation[k] for k in ('stage_sources','prior_tests','through_tests','personal_native','parity_sources','rejection_controls','abi_controls','reducer_controls')]==[314,1621,1935,24,338,124,6,7]
 for binary in validation['binaries']:assert shared.sha(binary['path'])==binary['sha256']
 assert len(validation['personal'])==24 and len(validation['coverage'])==1266
 for row in validation['coverage']+validation['personal']:assert shared.sha(ROOT/row['path'])==row['sha256']
 assert sum(row['path'].endswith('.t') for row in validation['coverage'])==314
 assert len(validation['checks'])==78 and len(validation['reducers'])==7
 for row in validation['checks']:assert row['exit_code']==0 and shared.sha(row['log'])==row['log_sha256']
 for row in validation['reducers']+[validation['owner_control']]:
  assert shared.sha(row['source_path'])==row['source_sha256']
  for out in row['outputs']:
   assert out['native_exit']==0 and shared.sha(out['path'])==out['sha256'] and shared.sha(out['native_path'])==out['native_sha256']
   if 'binary_path' in out:assert shared.sha(out['binary_path'])==out['binary_sha256']
  assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 control=validation['store_control'];assert shared.sha(control['source_path'])==control['source_sha256']
 for out in control['outputs']:assert out['exit_code']==0 and shared.sha(out['path'])==out['sha256']
 handoff=json.loads((ROOT/'student.tests/pa14'/handoff_name).read_text())
 assert handoff['implementation_commit'].startswith(implementation) and handoff['entry_commit'].startswith('5a795af4')
 assert handoff['release_sha256']==data['binaries'][1]['sha256']==validation['binaries'][0]['sha256']
 assert [row['name'] for row in handoff['checks']]==['stage','prior','through','file_audit','native']
 for row in handoff['checks']:assert row['exit_code']==0 and shared.sha(row['log'])==row['log_sha256']
 for row in handoff['initial_observations']+handoff['intermediate_binaries']:assert shared.sha(row['path'])==row['sha256']
 assert handoff['failed_build']['exit_code']==2
 print('560 expression-owner observations, source/instance equations, 338 sanitizer inputs, 124 rejections and snapshot/lifetime controls verified')
 return count
if __name__=='__main__':verify()

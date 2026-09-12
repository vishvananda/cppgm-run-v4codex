#!/usr/bin/env python3
"""Verify frozen inputs/outputs/binaries, unabridged orders and paired results."""
from pathlib import Path
import json,statistics,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
observations=0
for filename in ('preliminary-performance.json','graph-fastpath-performance.json',
                 'call-context-preliminary-performance.json','performance.json','graph-read-performance.json',
                 'definition-performance.json','symbolic-preliminary-performance.json','symbolic-context-performance.json','symbolic-performance.json','packing-performance.json','transfer-preliminary-performance.json','transfer-deleted-performance.json','transfer-performance.json','body-preliminary-performance.json','body-repeated-performance.json','body-performance.json','call-performance.json'):
 data=json.loads((ROOT/'student.tests/pa14'/filename).read_text())
 graph=filename=='graph-read-performance.json'
 definitions=filename=='definition-performance.json'
 symbolic=filename.startswith('symbolic-')
 packing=filename=='packing-performance.json'
 transfer=filename.startswith('transfer-')
 body=filename.startswith('body-')
 calls=filename=='call-performance.json'
 harness=ROOT/'student.tests/pa14'/('call_benchmark.py' if calls else 'body_benchmark.py' if body else 'transfer_benchmark.py' if transfer else 'packing_benchmark.py' if packing else 'symbolic_benchmark.py' if symbolic else 'graph_read_benchmark.py' if graph else 'definition_benchmark.py' if definitions else 'benchmark.py')
 assert shared.sha(harness)==data['harness_sha256']
 if not graph:
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
  assert len(data['workloads'])==(27 if calls else 19 if transfer or body else 4 if packing else 24 if symbolic else 19 if definitions else 16)
 else: assert len(data['workloads'])==4
 for binary in data['binaries']:
  assert shared.sha(binary['path'])==binary['sha256']
  assert shared.text_size(binary['path'])==binary['text_bytes']
 for name,work in data['workloads'].items():
  assert shared.sha(work['source_path'])==work['source_sha256']
  for out in work['outputs']:
   assert shared.sha(out['path'])==out['sha256']
   assert Path(out['path']).stat().st_size==out['bytes']
   if 'native' in out:
    native=out['native'];assert shared.sha(native['path'])==native['sha256']
    assert shared.text_size(native['path'])==native['text_bytes'] and native['checked_exit']==0
  if len(work['outputs'])==2 and (not transfer or work['exact_required']): assert work['outputs'][0]['sha256']==work['outputs'][1]['sha256']
  campaigns=[work] if graph else [work['compiler']]+([work['runtime']] if 'runtime' in work else [])
  for campaign in campaigns:
   rows=campaign['observations'];common=len(work['outputs'])==2
   assert [r['binary'] for r in rows]==(shared.ORDER if common else [1]*6)
   assert len(campaign['warmups'])==(2 if common else 1)
   for row in campaign['warmups']+rows:
    samples=row['samples'] if graph else [row]
    if graph: assert len(samples)==work['repeat']
    observations+=len(samples)
    assert all(r['wall_s']>0 and r['rss_kib']>0 and r['checked_exit']==0 for r in samples)
   if common:
    pairs=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
    assert pairs==campaign['paired_c_over_b' if graph else 'paired_b_over_a']
  if calls and name.rsplit('-',1)[1].isdigit() and name.startswith(('call-instances-','call-materializations-','call-unused-')):
   a,b=[out['telemetry'][0] for out in work['outputs']];n=int(name.rsplit('-',1)[1])
   assert a['semantic_entities']==b['semantic_entities'] and a['semantic_scopes']==b['semantic_scopes']
   if name.startswith('call-unused-'):
    assert b['semantic_template_fixed_calls']==5*n and b['semantic_template_fixed_call_uses']==0
    assert b['template_occurrences']==0 and b['semantic_candidate_work']==11*n
    assert b['semantic_conversion_work']==15*n+12 and b['semantic_conversions']==9*n+12
   else:
    objects=name.startswith('call-materializations-')
    assert b['semantic_template_fixed_calls']==5 and b['semantic_template_fixed_call_uses']==5*n
    assert a['template_occurrences']==b['template_occurrences']==(53 if objects else 50)*n
    assert a['semantic_candidate_work']==(11*n+6 if objects else 12*n)
    assert b['semantic_candidate_work']==n+(16 if objects else 11)
    assert a['semantic_conversion_work']==18*n+12 and b['semantic_conversion_work']==3*n+27
    assert a['semantic_conversions']==(14*n+7 if objects else 12*n+12)
    assert b['semantic_conversions']==(8*n+18 if objects else 3*n+21)
    if objects:
     assert a['semantic_conversion_objects']==2*n+1 and b['semantic_conversion_objects']==2*n+3
     assert a['semantic_user_conversions']==n and b['semantic_user_conversions']==n+1
  if transfer and name.startswith('transfer-instances-'):
   t=work['outputs'][-1]['telemetry'][0];n=int(name.rsplit('-',1)[1])
   assert t['template_class_completions']==n and t['semantic_template_binding_work']==23
   assert t['semantic_transfer_actions']==(4 if filename=='transfer-preliminary-performance.json' else 2)*n
  if body and name.startswith('fixed-instances-'):
   a,b=[out['telemetry'][0] for out in work['outputs']];n=int(name.rsplit('-',1)[1])
   assert b['semantic_template_fixed_expressions']==20 and b['semantic_template_fixed_uses']==20*n
   assert a['semantic_expression_work']==23*n and b['semantic_expression_work']==3*n+20
   assert a['semantic_conversion_work']==18*n and b['semantic_conversion_work']==6*n+12
   assert a['semantic_conversions']==20*n and b['semantic_conversions']==6*n+14
   assert a['template_occurrences']==b['template_occurrences']==76*n
  if body or calls:
   assert all(out['telemetry'][0]['semantic_expression_bytes']==36 and
              out['telemetry'][0]['semantic_entity_bytes']==112 for out in work['outputs'])
  if body and name.startswith('fixed-unused-'):
   a,b=[out['telemetry'][0] for out in work['outputs']];n=int(name.rsplit('-',1)[1])
   assert a['semantic_expression_work']==0 and b['semantic_template_fixed_expressions']==20*n
   assert b['semantic_template_fixed_uses']==0 and b['semantic_conversion_work']==12*n
   assert b['semantic_conversions']==14*n and b['template_occurrences']==0
  if not graph and name.startswith('template-repeat-'):
   assert work['outputs'][0]['telemetry'][0]['template_body_transitions']==1
   assert work['outputs'][0]['telemetry'][0]['template_occurrences']==24
  if not graph and name.startswith('class-instances-'):
   assert work['outputs'][0]['telemetry'][0]['template_class_completions']==int(name.rsplit('-',1)[1])
  if (definitions or symbolic) and name.startswith('definition-instances-'):
   assert work['outputs'][0]['telemetry'][0]['template_definition_applications']==3*int(name.rsplit('-',1)[1])
  if (symbolic or packing) and name.startswith('query-instances-'):
   t=work['outputs'][0]['telemetry'][0];n=int(name.rsplit('-',1)[1])
   assert t['semantic_type_query_work']==3*n+6
   assert t['semantic_template_binding_work']==12 and t['semantic_template_bindings']==2
  if symbolic and name.startswith('binding-instances-'):
   t=work['outputs'][0]['telemetry'][0]
   assert t['semantic_template_binding_work']==13 and t['semantic_template_bindings']==3
 print(filename,'PASS')
print(observations,'timed compiler/executable process observations verified')

layout=json.loads((ROOT/'student.tests/pa14/expression-layout.json').read_text())
for row in layout:
 assert shared.sha(row['model_path'])==row['model_sha256']
 assert shared.sha(row['binary_path'])==row['binary_sha256']
 assert shared.sha(row['source_path'])==row['source_sha256']
 values=list(map(int,shared.run([row['binary_path']]).stdout.split()))
 assert values==[row['expression_bytes'],row['entity_bytes']]
assert [row['expression_bytes'] for row in layout]==[36,40,36]
assert all(row['entity_bytes']==112 for row in layout)
print('frozen expression/declaration record sizes verified')

emission=json.loads((ROOT/'student.tests/pa14/emission-layout.json').read_text())
for row in emission:
 for kind in ('model','source','binary'):
  assert shared.sha(row[kind+'_path'])==row[kind+'_sha256']
 assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==row['sizes']==[112,36]
print('frozen emission flags preserve declaration/expression sizes')

preliminary=json.loads((ROOT/'student.tests/pa14/transfer-preliminary-performance.json').read_text())
final=json.loads((ROOT/'student.tests/pa14/transfer-performance.json').read_text())
for name,work in final['workloads'].items():
 previous=preliminary['workloads'][name]
 assert work['source_sha256']==previous['source_sha256']
 assert [out['sha256'] for out in work['outputs']]==[out['sha256'] for out in previous['outputs']]
 if 'runtime' in work:
  assert [out['native']['sha256'] for out in work['outputs']]==[out['native']['sha256'] for out in previous['outputs']]
print('known-deleted fact correction preserves all frozen compiler/native outputs')

first=json.loads((ROOT/'student.tests/pa14/body-preliminary-performance.json').read_text())
for filename in ('body-repeated-performance.json','body-performance.json'):
 last=json.loads((ROOT/'student.tests/pa14'/filename).read_text())
 if filename=='body-repeated-performance.json': assert first['binaries']==last['binaries']
 for name,work in last['workloads'].items():
  previous=first['workloads'][name]
  assert work['source_sha256']==previous['source_sha256']
  assert [out['sha256'] for out in work['outputs']]==[out['sha256'] for out in previous['outputs']]
  if 'runtime' in work:
   assert [out['native']['sha256'] for out in work['outputs']]==[out['native']['sha256'] for out in previous['outputs']]
print('all fixed-body campaigns preserve all compiler/native outputs')


call_data=json.loads((ROOT/'student.tests/pa14/call-performance.json').read_text())
body_data=json.loads((ROOT/'student.tests/pa14/body-performance.json').read_text())
for name,previous in body_data['workloads'].items():
 current=call_data['workloads'][name]
 assert current['source_sha256']==previous['source_sha256']
 assert all(out['sha256']==previous['outputs'][1]['sha256'] for out in current['outputs'])
 if 'runtime' in current:
  assert all(out['native']['sha256']==previous['outputs'][1]['native']['sha256'] for out in current['outputs'])
for work in call_data['workloads'].values():
 if 'runtime' in work: assert work['outputs'][0]['native']['sha256']==work['outputs'][1]['native']['sha256']
print('fixed-call campaign preserves all prior and new compiler/native outputs')

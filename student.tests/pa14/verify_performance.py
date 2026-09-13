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
                 'definition-performance.json','symbolic-preliminary-performance.json','symbolic-context-performance.json','symbolic-performance.json','packing-performance.json','transfer-preliminary-performance.json','transfer-deleted-performance.json','transfer-performance.json','body-preliminary-performance.json','body-repeated-performance.json','body-performance.json','call-performance.json','object-performance.json','object-repeat-preliminary-performance.json','object-repeat-performance.json','object-view-performance.json','dependent-object-performance.json','dependent-object-final-performance.json','dependent-object-repeat-performance.json','prototype-performance.json','dependent-object-packed-performance.json','prototype-packed-performance.json','method-parameter-performance.json','prototype-methods-performance.json'):
 data=json.loads((ROOT/'student.tests/pa14'/filename).read_text())
 graph=filename=='graph-read-performance.json'
 definitions=filename=='definition-performance.json'
 symbolic=filename.startswith('symbolic-')
 packing=filename=='packing-performance.json'
 transfer=filename.startswith('transfer-')
 body=filename.startswith('body-')
 calls=filename=='call-performance.json'
 objects=filename.startswith('object-');repeat=filename.startswith('object-repeat-');preliminary=filename=='object-repeat-preliminary-performance.json'
 views=filename=='object-view-performance.json'
 methods=filename=='method-parameter-performance.json'
 dependent=filename.startswith('dependent-object-') or methods
 packed=filename=='dependent-object-packed-performance.json'
 repeated=filename=='dependent-object-repeat-performance.json' or packed
 prototype=filename.startswith('prototype-')
 harness=ROOT/'student.tests/pa14'/('method_parameter_benchmark.py' if methods else 'prototype_benchmark.py' if prototype else 'dependent_object_packed.py' if packed else 'dependent_object_repeat.py' if repeated else 'dependent_object_benchmark.py' if dependent else 'object_view_benchmark.py' if views else 'object_repeat.py' if preliminary else 'object_repeat_final.py' if repeat else 'object_benchmark.py' if objects else 'call_benchmark.py' if calls else 'body_benchmark.py' if body else 'transfer_benchmark.py' if transfer else 'packing_benchmark.py' if packing else 'symbolic_benchmark.py' if symbolic else 'graph_read_benchmark.py' if graph else 'definition_benchmark.py' if definitions else 'benchmark.py')
 assert shared.sha(harness)==data['harness_sha256']
 if repeat or views or repeated or methods: assert shared.sha(data['parent_path'])==data['parent_sha256']
 if not graph:
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
  assert len(data['workloads'])==(3 if prototype else 10 if methods or repeated else 16 if dependent else 10 if views else 1 if preliminary else 6 if repeat else 38 if objects else 27 if calls else 19 if transfer or body else 4 if packing else 24 if symbolic else 19 if definitions else 16)
 else: assert len(data['workloads'])==4
 for binary in data['binaries']:
  assert shared.sha(binary['path'])==binary['sha256']
  assert shared.text_size(binary['path'])==binary['text_bytes']
 for name,work in data['workloads'].items():
  assert shared.sha(work['source_path'])==work['source_sha256']
  if 'entry_rejection' in work:
   rejected=work['entry_rejection'];assert rejected['exit_code']==1 and shared.sha(rejected['path'])==rejected['sha256']
  if 'entry_failure' in work:
   failed=work['entry_failure'];assert shared.sha(failed['path'])==failed['sha256']
   assert shared.sha(failed['native_path'])==failed['native_sha256'] and failed['exit_status']==1
  for out in work['outputs']:
   assert shared.sha(out['path'])==out['sha256']
   assert Path(out['path']).stat().st_size==out['bytes']
   if 'native' in out:
    native=out['native'];assert shared.sha(native['path'])==native['sha256']
    assert shared.text_size(native['path'])==native['text_bytes'] and native['checked_exit']==0
  if len(work['outputs'])==2 and (not transfer or work['exact_required']): assert work['outputs'][0]['sha256']==work['outputs'][1]['sha256']
  if (objects or dependent) and len(work['outputs'])==2 and 'runtime' in work:
   assert work['outputs'][0]['native']['sha256']==work['outputs'][1]['native']['sha256']
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
  if views:
   a,b=[out['telemetry'][0] for out in work['outputs']]
   for key in ('semantic_entities','semantic_scopes','semantic_object_uses','semantic_candidate_work','semantic_conversion_work','semantic_expression_work','template_occurrences'):
    assert a[key]==b[key]
  if dependent and name.startswith('member-') and name.rsplit('-',1)[1].isdigit():
   a,b=[out['telemetry'][0] for out in work['outputs']];n=int(name.rsplit('-',1)[1])
   assert a['semantic_entities']==b['semantic_entities'] and a['semantic_scopes']==b['semantic_scopes']
   if name.startswith('member-unused-'):
    assert b['semantic_template_object_contexts']==n and b['semantic_template_member_uses']==0
    assert a['semantic_expression_work']==3*n and b['semantic_expression_work']==7*n
    assert b['template_occurrences']==b['template_class_completions']==b['template_body_transitions']==0
   elif name.startswith('member-repeated-'):
    assert b['semantic_template_object_contexts']==1 and b['semantic_template_member_uses']==4
    assert a['semantic_expression_work']==9*n+24 and b['semantic_expression_work']==2*n+20
    assert a['semantic_object_uses']==4*n+12 and b['semantic_object_uses']==12
    assert a['template_occurrences']==b['template_occurrences']==32*n+176
   else:
    assert b['semantic_template_object_contexts']==2 and b['semantic_template_member_uses']==2*n
    assert a['semantic_expression_work']==24*n+4 and b['semantic_expression_work']==13*n+9
    assert a['semantic_object_uses']==10*n and b['semantic_object_uses']==6*n
    assert a['template_occurrences']==b['template_occurrences']==(130 if name.startswith('member-outside-') else 92)*n
  if objects and not views and name.rsplit('-',1)[1].isdigit() and name.startswith(('object-instances-','object-results-','object-unused-')):
   a,b=[out['telemetry'][0] for out in work['outputs']];n=int(name.rsplit('-',1)[1])
   assert a['semantic_entities']==b['semantic_entities'] and a['semantic_scopes']==b['semantic_scopes']
   if name.startswith('object-unused-'):
    assert b['semantic_template_fixed_calls']==2*n and b['semantic_template_fixed_call_uses']==0
    assert b['semantic_object_uses']==5*n and b['template_occurrences']==0
   else:
    assert b['semantic_template_fixed_calls']==2 and b['semantic_template_fixed_call_uses']==2*n
    result=name.startswith('object-results-')
    assert b['semantic_object_uses']==(n+7 if result else 8)
    assert a['semantic_object_uses']==(4*n+3 if result else 5*n+3)
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

object_data=json.loads((ROOT/'student.tests/pa14/object-performance.json').read_text())
view_data=json.loads((ROOT/'student.tests/pa14/object-view-performance.json').read_text())
for name,previous in call_data['workloads'].items():
 current=object_data['workloads'][name]
 assert current['source_sha256']==previous['source_sha256']
 assert all(out['sha256']==previous['outputs'][1]['sha256'] for out in current['outputs'])
for name,current in view_data['workloads'].items():
 previous=object_data['workloads'][name]
 assert current['source_sha256']==previous['source_sha256']
 assert all(out['sha256']==previous['outputs'][-1]['sha256'] for out in current['outputs'])
print('receiver and immutable-view campaigns preserve inherited compiler outputs')

layout=json.loads((ROOT/'student.tests/pa14/object-layout.json').read_text())
for row in layout:
 for header in row['headers']: assert shared.sha(header['path'])==header['sha256']
 for kind in ('source','binary','dump'): assert shared.sha(row[kind+'_path'])==row[kind+'_sha256']
 assert list(map(int,shared.run([row['binary_path']]).stdout.split()))==[112,36,36]
assert [row['temporary_state'] for row in layout]==[28,32]
declaration=next(line for line in (ROOT/'dev/src/lowering/procedural.h').read_text().splitlines() if 'struct TemporaryState :' in line).strip()
assert declaration==layout[1]['temporary_declaration']
print('receiver layouts unchanged; concrete cleanup address costs four bytes')

field_layout=json.loads((ROOT/'student.tests/pa14/dependent-object-layout.json').read_text())
for header in field_layout['headers']:assert shared.sha(header['path'])==header['sha256']
for kind in ('source','binary','dump'):assert shared.sha(field_layout[kind+'_path'])==field_layout[kind+'_sha256']
assert list(map(int,shared.run([field_layout['binary_path']]).stdout.split()))==field_layout['sizes']==[112,36,36,8,12,2]
assert shared.sha(ROOT/'dev/src/semantic/model.h')==field_layout['headers'][0]['sha256']
print('current field-context layouts preserve 112/36/36 byte hot records; new records 8/12/2 bytes')

field_proofs=Path(field_layout['source_path']).parent.parent/'proofs/rejections.json'
rows=json.loads(field_proofs.read_text());assert len(rows)==13
for row in rows:
 assert shared.sha(row['source'])==row['sha256']
 assert [out['exit'] for out in row['outputs']]==[0,1]
 for out in row['outputs']:assert shared.sha(out['log_path'])==out['log_sha256']
print('thirteen new definition-time field-context rejection proofs verified')

proofs=Path(object_data['binaries'][0]['path']).parent/'proofs'
rejections=json.loads((proofs/'rejections.json').read_text())
assert len(rejections)==20 and sum(row['outputs'][0]['exit']==0 for row in rejections)==18
for row in rejections:
 assert shared.sha(row['source'])==row['sha256'] and row['outputs'][1]['exit']==1
identity=json.loads((proofs/'default-identity.json').read_text())
assert [row['native_exit'] for row in identity]==[1,0]
for row in identity:
 for kind in ('source','ir','native'): assert shared.sha(row[kind])==row[kind+'_sha256']
print('eighteen newly rejected errors and the native default-identity proof verified')

field_initial=json.loads((ROOT/'student.tests/pa14/dependent-object-performance.json').read_text())
field_final=json.loads((ROOT/'student.tests/pa14/dependent-object-final-performance.json').read_text())
field_repeat=json.loads((ROOT/'student.tests/pa14/dependent-object-repeat-performance.json').read_text())
for data in [field_final,field_repeat]:
 for name,current in data['workloads'].items():
  prior=field_initial['workloads'][name]
  assert current['source_sha256']==prior['source_sha256']
  assert [out['sha256'] for out in current['outputs']]==[out['sha256'] for out in prior['outputs']]
  if 'runtime' in current:assert [out['native']['sha256'] for out in current['outputs']]==[out['native']['sha256'] for out in prior['outputs']]
assert len(field_repeat['preflight'])==16
for row in field_repeat['preflight']:
 for kind in ['source','native']:
  if kind+'_path' in row:assert shared.sha(row[kind+'_path'])==row[kind+'_sha256']
 assert shared.sha(row['path'])==row['sha256']
 if 'native_path' in row:assert row['native_exit']==0
print('all member campaigns and complete final preflight preserve compiler/native outputs')
parameter_proof=field_proofs.parent/'parameter-shape.json'
rows=json.loads(parameter_proof.read_text());assert [row['exit'] for row in rows]==[1,0]
for row in rows:
 for kind in ['source','binary','log','ir','native']:
  if kind in row:assert shared.sha(row[kind])==row[kind+'_sha256']
assert rows[1]['native_exit']==0
print('prototype-scope entry rejection and corrected native proof verified')

packed=json.loads((ROOT/'student.tests/pa14/dependent-object-packed-performance.json').read_text())
assert len(packed['preflight'])==16
for name,w in packed['workloads'].items():
 previous=field_repeat['workloads'][name]
 assert w['source_sha256']==previous['source_sha256']
 assert [o['sha256'] for o in w['outputs']]==[o['sha256'] for o in previous['outputs']]
 for key in ['semantic_entities','semantic_scopes','semantic_expression_work','semantic_object_uses','template_occurrences']:
  assert w['outputs'][1]['telemetry'][0][key]==previous['outputs'][1]['telemetry'][0][key]
for row in packed['preflight']:
 assert shared.sha(row['source_path'])==row['source_sha256'] and shared.sha(row['path'])==row['sha256']
 if 'native_path' in row:assert shared.sha(row['native_path'])==row['native_sha256'] and row['native_exit']==0
prototype_first=json.loads((ROOT/'student.tests/pa14/prototype-performance.json').read_text())
prototype_final=json.loads((ROOT/'student.tests/pa14/prototype-packed-performance.json').read_text())
for name,w in prototype_final['workloads'].items():
 previous=prototype_first['workloads'][name]
 assert w['source_sha256']==previous['source_sha256'] and w['outputs'][0]['sha256']==previous['outputs'][0]['sha256']
 if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==previous['outputs'][0]['native']['sha256']
print('packed source facts preserve all repeated and prototype compiler/native outputs')

methods=json.loads((ROOT/'student.tests/pa14/method-parameter-performance.json').read_text())
assert len(methods['preflight'])==16
for row in methods['preflight']:
 assert shared.sha(row['source_path'])==row['source_sha256'] and shared.sha(row['path'])==row['sha256']
 name=Path(row['path']).name.removesuffix('-preflight.lowir');previous=field_final['workloads'][name]
 assert row['source_sha256']==previous['source_sha256'] and row['sha256']==previous['outputs'][1]['sha256']
 if 'native_path' in row:assert shared.sha(row['native_path'])==row['native_sha256']==previous['outputs'][1]['native']['sha256'] and row['native_exit']==0
 for key in ['semantic_entities','semantic_scopes','semantic_expression_work','semantic_object_uses','template_occurrences']:
  assert row['telemetry'][0][key]==previous['outputs'][1]['telemetry'][0][key]
proto_methods=json.loads((ROOT/'student.tests/pa14/prototype-methods-performance.json').read_text())
for name,w in proto_methods['workloads'].items():
 previous=prototype_final['workloads'][name]
 assert w['source_sha256']==previous['source_sha256'] and w['outputs'][0]['sha256']==previous['outputs'][0]['sha256']
 if 'runtime' in w:assert w['outputs'][0]['native']['sha256']==previous['outputs'][0]['native']['sha256']
rows=json.loads((field_proofs.parent/'final-methods/rejections.json').read_text());assert len(rows)==14
for row in rows:
 assert shared.sha(row['source'])==row['source_sha256'] and [out['exit'] for out in row['outputs']]==[0,1]
 for out in row['outputs']:
  assert shared.sha(out['binary'])==out['binary_sha256'] and shared.sha(out['log'])==out['log_sha256']
print('final method selection preserves all prior work/output facts; fourteen rejection proofs verified')

field_artifacts=field_proofs.parent.parent
reducers=json.loads((field_artifacts/'reducer-parity-final.json').read_text())
assert len(reducers['reducers'])==4
for row in reducers['reducers']:
 assert shared.sha(row['source'])==row['source_sha256']
 assert len(row['outputs'])==2 and row['outputs'][0]['sha256']==row['outputs'][1]['sha256']
 for out in row['outputs']:
  assert out['exit_code']==0 and shared.sha(out['path'])==out['sha256']
  assert shared.sha(out['binary'])==out['binary_sha256'] and shared.sha(out['log'])==out['log_sha256']
  native=out['native'];assert shared.sha(native['path'])==native['sha256']
  for result in (native['backend'],native['execution']):
   assert result['exit_code']==0 and shared.sha(result['log'])==result['log_sha256']
 assert row['outputs'][0]['native']['sha256']==row['outputs'][1]['native']['sha256']
proof_dir=field_proofs.parent/'final-reducers'
historical=json.loads((proof_dir/'historical-source.json').read_text())
assert shared.sha(historical['source'])==historical['source_sha256']
assert historical['source_sha256']==json.loads(Path(historical['manifest']).read_text())[-1]['source_sha256']
rows=json.loads((proof_dir/'method-proof.json').read_text())
assert [row['exit_code'] for row in rows]==[0,1,0]
for row in rows:
 for kind in ('source','binary','log','ir','native'):
  if kind in row:assert shared.sha(row[kind])==row[kind+'_sha256']
 if 'execution' in row:
  execution=row['execution'];assert execution['exit_code']==0 and shared.sha(execution['log'])==execution['log_sha256']
print('four final frozen reducers preserve sanitizer/native parity; nested method regression and historical source verified')

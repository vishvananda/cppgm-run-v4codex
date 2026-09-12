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
                 'definition-performance.json','symbolic-preliminary-performance.json','symbolic-context-performance.json','symbolic-performance.json','packing-performance.json'):
 data=json.loads((ROOT/'student.tests/pa14'/filename).read_text())
 graph=filename=='graph-read-performance.json'
 definitions=filename=='definition-performance.json'
 symbolic=filename.startswith('symbolic-')
 packing=filename=='packing-performance.json'
 harness=ROOT/'student.tests/pa14'/('packing_benchmark.py' if packing else 'symbolic_benchmark.py' if symbolic else 'graph_read_benchmark.py' if graph else 'definition_benchmark.py' if definitions else 'benchmark.py')
 assert shared.sha(harness)==data['harness_sha256']
 if not graph:
  assert shared.sha(ROOT/'student.tests/pa10/benchmark.py')==data['shared_harness_sha256']
  assert shared.sha(ROOT/'reference-binaries/lowir2native')==data['backend_sha256']
  assert len(data['workloads'])==(4 if packing else 24 if symbolic else 19 if definitions else 16)
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
  if len(work['outputs'])==2: assert work['outputs'][0]['sha256']==work['outputs'][1]['sha256']
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

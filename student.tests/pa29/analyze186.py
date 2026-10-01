#!/usr/bin/env python3
"""Audit frozen image equivalence, complete observations and demand scaling."""
import json,pathlib,hashlib
root=pathlib.Path(__file__).resolve().parents[2];folder=root/'student.tests/pa29/evidence186'
rows=[];observations=0
for name in ['common','owner']:
 d=json.loads((folder/(name+'-performance.json')).read_text());observations+=len(d['runs'])
 assert all(x['status']==0 for x in d['runs'])
 for v in d['binaries'].values():assert hashlib.sha256(pathlib.Path(v['path']).read_bytes()).hexdigest()==v['sha256']
 for key,images in d['images'].items():
  if 'A' in images:
   assert images['A']['object_sha256']==images['B']['object_sha256']
   assert images['A']['executable_sha256']==images['B']['executable_sha256']
   for mode in ['compile','runtime']:
    samples=[x for x in d['runs'] if x['workload']==key and x['mode']==mode]
    assert [x['label'] for x in samples]==list('AAAA'+'ABBA'*6)
  else:
   for x in d['runs']:
    if x['workload']!=key or x['mode']!='compile':continue
    n=d['inputs'][key]['N'];c={k:v for r in x['phase_counters'] for k,v in r.items()}
    expected=dict(parsed_nodes=23*n+323,nodes=67*n+323,semantic_specializations=n,
     template_body_transitions=n,semantic_constant_execution_hits=n,
     semantic_template_default_binding_work=1,template_default_facts=n,
     template_default_argument_work=n,template_default_demands=n,
     semantic_type_query_work=n+3,semantic_value_query_work=n+1,
     instructions=187,operands=314,prepared_instructions=187,prepared_operands=314,native_instructions=805)
    assert all(c[k]==v for k,v in expected.items()),(key,expected)
    rows.append(dict(workload=key,block=x['block'],asserted_counters=expected))
 if name=='owner':
  images=[d['images']['inquiries'+str(n)]['B'] for n in (600,1200,2400)]
  assert all(x==images[0] for x in images)
assert observations==496 and len(rows)==24
(folder/'scaling.json').write_text(json.dumps(dict(observations=observations,equivalent_pairs=8,scaling_checks=rows),indent=2)+'\n')
print('496 observations, eight identical object/executable pairs, 24 scaling checks pass')

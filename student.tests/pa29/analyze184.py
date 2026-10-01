#!/usr/bin/env python3
"""Check frozen binaries, equivalent objects, and every owner work observation."""
import hashlib,json,pathlib
root=pathlib.Path(__file__).resolve().parents[2];out=root/'student.tests/pa29/evidence184'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(n):return json.loads((out/(n+'.json')).read_text())
a=read('common-performance');b=read('owner-performance')
assert a['binaries']==b['binaries']
assert a['flags']==b['flags'] and a['affinity']==b['affinity']==['taskset','-c','0']
assert a['binaries']['B']['sha256']==sha(root/'dev/cppgm++')
for v in a['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
assert len(a['runs'])==224 and len(b['runs'])==104
assert all(r['status']==0 for data in [a,b] for r in data['runs'])
assert all(v['A']['object_sha256']==v['B']['object_sha256'] for v in a['images'].values())
assert b['images']['runtime2400']['A']['object_sha256']==b['images']['runtime2400']['B']['object_sha256']
rows=[]
for r in b['runs']:
 if r['mode']!='compile' or not r['workload'].startswith('constant'):continue
 n=b['inputs'][r['workload']]['N'];c={}
 for data in r['phase_counters']:c.update(data)
 expected=dict(parsed_nodes=27*n+422,nodes=85*n+422,semantic_specializations=n,template_body_transitions=n,semantic_body_checks=n+6,semantic_constant_bodies=n+3,semantic_constant_activations=n+6,semantic_constant_execution_steps=9*n+10,semantic_constant_execution_hits=n,semantic_constant_dependency_work=4*n+7,semantic_constant_work=16*n+14,semantic_conversion_work=14*n+58,semantic_conversions=15*n+61,semantic_constant_object_work=7,semantic_constant_address_work=8,semantic_member_constant_values=2,instructions=91,operands=140,prepared_instructions=91,prepared_operands=140)
 assert all(c[k]==v for k,v in expected.items()),(r['workload'],{k:(c[k],v) for k,v in expected.items() if c[k]!=v})
 assert b['inputs'][r['workload']]['entry_rejection']['status']!=0
 assert b['images'][r['workload']]['B']['text_bytes']==661
 rows.append(dict(workload=r['workload'],sample=r['block'],expected=expected))
assert len(rows)==24
(out/'scaling.json').write_text(json.dumps(dict(compiler_sha256=a['binaries']['B']['sha256'],observations=rows),indent=2)+'\n')
print('328 performance observations verified; 24 exact owner scaling checks; five identical object pairs')

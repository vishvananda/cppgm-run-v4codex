#!/usr/bin/env python3
"""Bind frozen performance, validate exact work slopes and retain entry rejections."""
import hashlib,json,pathlib,resource,statistics,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];scratch=pathlib.Path(sys.argv[1]).resolve();out=root/'student.tests/pa29/evidence183'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(n,v):(out/(n+'.json')).write_text(json.dumps(v,indent=2)+'\n')
a=json.loads((out/'common-performance.json').read_text());b=json.loads((out/'owner-performance.json').read_text())
c=json.loads((out/'common-confirm-performance.json').read_text())
assert a['binaries']['B']['sha256']==b['compiler']['sha256']==sha(root/'dev/cppgm++')
for label,v in a['binaries'].items():assert sha(pathlib.Path(v['path']))==v['sha256']
assert len(a['runs'])==224 and len(c['runs'])==224 and len(b['runs'])==96
assert c['binaries']==a['binaries'] and c['inputs']==a['inputs'] and c['flags']==a['flags']==b['flags']
assert c['affinity']==['taskset','-c','0']
assert all(r['status']==0 for data in [a,b,c] for r in data['runs'])
assert all(v['A']['object_sha256']==v['B']['object_sha256'] for data in [a,c] for v in data['images'].values())
rows=[]
for r in b['runs']:
 if r['mode']!='compile':continue
 n=b['inputs'][r['workload']]['N'];family=b['inputs'][r['workload']]['family'];c={}
 for block in r['phase_counters']:c.update(block)
 if family=='guides':
  expected=dict(semantic_deduction_guides=n,semantic_deduction_guide_parameters=2*n,parsed_nodes=56*n+229,nodes=56*n+229,semantic_specializations=2*n+1,semantic_type_substitution_work=3*n,template_body_transitions=0,semantic_body_checks=3,semantic_type_query_work=6*n+1,semantic_query_edges=5*n,instructions=70,operands=110,prepared_instructions=70,prepared_operands=110)
 else:
  expected=dict(semantic_deduction_guides=0,parsed_nodes=15*n+270,nodes=59*n+270,semantic_specializations=n,semantic_template_default_binding_work=1,template_default_argument_work=n,template_default_facts=n,template_body_transitions=n,semantic_body_checks=n+3,instructions=17*n+71,operands=26*n+112,prepared_instructions=17*n+71,prepared_operands=26*n+112)
 assert all(c[k]==v for k,v in expected.items()),(r['workload'],{k:(c[k],v) for k,v in expected.items() if c[k]!=v})
 rows.append(dict(workload=r['workload'],sample=r['sample'],expected=expected,guide_capacity_bytes=c['semantic_deduction_guide_bytes']))
save('scaling',dict(observations=rows,compiler_sha256=b['compiler']['sha256']))
rejections=[]
for name in b['inputs']:
 src=scratch/'owner'/(name+'.cpp');assert sha(src)==b['inputs'][name]['sha256']
 args=[a['binaries']['A']['path'],'-O0','-c',str(src),'-o',str(scratch/'entry-probe.o')]
 p=subprocess.run(args,capture_output=True,text=True,timeout=60,preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
 assert p.returncode!=0
 rejections.append(dict(command=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr,source_sha256=sha(src)))
save('entry-rejections',dict(compiler_sha256=a['binaries']['A']['sha256'],rows=rejections))
print('scaling:',len(rows),'observations; entry rejection:',len(rejections),'inputs')

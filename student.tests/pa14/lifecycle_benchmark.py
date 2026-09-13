#!/usr/bin/env python3
"""Lifecycle fact ownership; unrelated declarations, owners, fields and uses."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/virtual-demand-performance.json';old=json.loads(parent.read_text())
virtual_parent=ROOT/'student.tests/pa13/final-audit-performance.json';virtual_old=json.loads(virtual_parent.read_text())
names=['virtual-runtime','destructor-runtime','body-run-4000-128','declaration-instances-1000','demand-uses-1000-128-4','demand-runtime','region-runtime','calls-runtime','memory-runtime','floating-runtime']
shapes=[(16000,1,4,1),(64000,1,4,1),(16000,512,4,1),(16000,512,32,1),(16000,1,4,4000)]
def lifecycle_source(n,k,s,q):
 lines=['using U%d=int;'%i for i in range(n)]
 lines+=['int count;template<class T>struct Leaf{T value;~Leaf(){++count;}};']
 for i in range(k):
  lines.append('struct C%d{'%i+' '.join('Leaf<int> field%d;'%j for j in range(s))+'~C%d()=default;};'%i)
  lines.append('void use%d(C%d*p){p->~C%d();}'%(i,i,i))
 lines.append('void repeated(C0*p){'+'use0(p);'*q+'}')
 return '\n'.join(lines)+'\n'
generated={'lifecycle-%d-%d-%d-%d'%shape:shape for shape in shapes}
names+=list(generated)

binaries=[A.resolve(),B.resolve()]
result=dict(protocol='preflight all equivalent correct outputs; one warmup each; four A/A observations and two ABBA blocks; no concurrent builds/tests',cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',parent_path=str(parent),parent_sha256=shared.sha(parent),virtual_parent_path=str(virtual_parent),virtual_parent_sha256=shared.sha(virtual_parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],acceptance='required lifecycle fact/state ownership; one preparation per member and visit per required field; no unused body work; zero generated growth on equivalent correct inputs; no runtime optimization',workloads={})
commands={};runtimes={}
for name in names:
 oldrow=old['workloads'].get(name) or virtual_old['workloads'].get(name)
 src=WORK/(name+'.cpp')
 if oldrow:
  assert shared.sha(oldrow['source_path'])==oldrow['source_sha256']
  src.write_bytes(Path(oldrow['source_path']).read_bytes())
 else:src.write_text(lifecycle_source(*generated[name]))
 outputs=[];commands[name]={};runtimes[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if not b and name in old['workloads']: assert item['sha256']==oldrow['outputs'][1]['sha256'],name
  if oldrow and 'runtime' in oldrow:
   exe=WORK/f'{name}-{b}';shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 if runtimes[name]: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,shape=generated.get(name))
 if name in generated:
  n,k,s,q=generated[name];stats=outputs[1]['telemetry'][0]
  assert stats['semantic_member_demands']==stats['semantic_demand_processed']==k+1
  assert stats['semantic_destruction_actions']==k*s
 print(name,'preflight',flush=True)
OUT.write_text(json.dumps(result,indent=2)+'\n')
if os.environ.get('PREFLIGHT_ONLY'):sys.exit(0)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

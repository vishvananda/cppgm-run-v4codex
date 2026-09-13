#!/usr/bin/env python3
"""Default facts; unrelated declarations, keys, constructor candidates and uses."""
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
shapes=[(16000,1,1,1),(64000,1,1,1),(16000,512,1,1),(16000,1,32,1),(16000,1,32,4000),(16000,1,1,4000)]
def default_source(n,k,m,q):
 lines=['using U%d=int;'%i for i in range(n)]
 lines+=['struct Tag%d;'%i for i in range(m-1)]
 lines+=['struct C{C(int);'+' '.join('C(Tag%d*);'%i for i in range(m-1))+'};']
 lines+=['template<class T>int make(){return 4;}']
 lines+=['template<class T>void f%d(T a=make<int>());'%i for i in range(k)]
 lines+=['void uses(){'+' '.join('f%d<C>();'%i for i in range(k))+'f0<C>();'*(q-1)+'}']
 return '\n'.join(lines)+'\n'
generated={'default-%d-%d-%d-%d'%shape:shape for shape in shapes}
names+=list(generated)+['default-runtime']

binaries=[A.resolve(),B.resolve()]
result=dict(started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),text_metric='compiler .text; sectionless native executable payload after ELF entry; exact whole-executable equality also required',protocol='preflight all equivalent correct outputs; one warmup each; four A/A observations and two ABBA blocks; no concurrent builds/tests',cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',parent_path=str(parent),parent_sha256=shared.sha(parent),virtual_parent_path=str(virtual_parent),virtual_parent_sha256=shared.sha(virtual_parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],acceptance='required default fact/state ownership; one checked conversion per key, one registration per edge, O(call arguments) materialization; no unused body work; zero generated growth on equivalent correct inputs; no runtime optimization',workloads={})
commands={};runtimes={}
for name in names:
 oldrow=old['workloads'].get(name) or virtual_old['workloads'].get(name)
 src=WORK/(name+'.cpp')
 if oldrow:
  assert shared.sha(oldrow['source_path'])==oldrow['source_sha256']
  src.write_bytes(Path(oldrow['source_path']).read_bytes())
 elif name=='default-runtime':
  src.write_text('int count;struct C{int n;C(int v):n(v){}~C(){++count;}};template<class T>int make(){return 3;}template<class T>int f(T a=make<int>()){return a.n;}int main(){volatile int n=3000000;int sum=0;for(int i=0;i<n;++i)sum+=f<C>();return sum!=9000000||count!=3000000;}')
 else:src.write_text(default_source(*generated[name]))
 outputs=[];commands[name]={};runtimes[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if not b and name in old['workloads']: assert item['sha256']==oldrow['outputs'][1]['sha256'],name
  if (oldrow and 'runtime' in oldrow) or name=='default-runtime':
   exe=WORK/f'{name}-{b}';shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 if runtimes[name]: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,shape=generated.get(name))
 if name in generated:
  n,k,m,q=generated[name];stats=outputs[1]['telemetry'][0]
  assert stats['template_default_argument_work']==stats['template_default_facts']==stats['template_default_demands']==k
  assert stats['template_default_dependency_work']==stats['template_default_dependencies']
  assert stats['template_default_dependencies']<=8*k
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
result['finished_utc']=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime())
OUT.write_text(json.dumps(result,indent=2)+'\n')

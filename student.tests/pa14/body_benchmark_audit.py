#!/usr/bin/env python3
"""Body publication and retained statement facts: frozen AA/ABBA evidence."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
from body_compare import compare
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/default-final-performance.json';old=json.loads(parent.read_text())
virtual_parent=parent;virtual_old=old
names=list(old['workloads'])
shapes=[(16000,1,8,1),(64000,1,8,1),(16000,128,8,1),(16000,1,8,4000)]
def default_source(n,k,m,q):
 lines=['using U%d=int;'%i for i in range(n)]
 lines+=['struct C{int n;C(int x):n(x){}operator int()const{return n;}};']
 lines+=['template<class T>int f(C& c,int x){'+''.join('if(x==%d)return c;'%i for i in range(m))+'return c;}']
 lines+=['struct Tag%d;'%i for i in range(k)]
 lines+=['int uses(){C c(3);int s=0;'+''.join('s+=f<Tag%d>(c,2);'%i for i in range(k))+'s+=f<Tag0>(c,2);'*(q-1)+'return s;}']
 return '\n'.join(lines)+'\n'
generated={'return-%d-%d-%d-%d'%shape:shape for shape in shapes}
names+=list(generated)+['return-runtime']

binaries=[A.resolve(),B.resolve()]
result=dict(started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),text_metric='compiler .text; sectionless native executable payload after ELF entry; checked native outcomes and complete executable hashes reported',protocol='preflight all equivalent correct outputs; one warmup each; four A/A observations and two ABBA blocks; no concurrent builds/tests',cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',parent_path=str(parent),parent_sha256=shared.sha(parent),virtual_parent_path=str(virtual_parent),virtual_parent_sha256=shared.sha(virtual_parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],acceptance='one body/lifetime check per function, one return conversion recipe per fixed source and one application per consumed return; no unused body demand; zero generated code growth on comparable inputs; PA14 O0 adds no runtime transform',workloads={})
commands={};runtimes={}
for name in names:
 oldrow=old['workloads'].get(name) or virtual_old['workloads'].get(name)
 src=WORK/(name+'.cpp')
 if oldrow:
  assert shared.sha(oldrow['source_path'])==oldrow['source_sha256']
  src.write_bytes(Path(oldrow['source_path']).read_bytes())
 elif name=='return-runtime':
  src.write_text('int count;struct C{int n;C(int x):n(x){}operator int()const{++count;return n;}};template<class T>int f(C& c,int x){if(x==0)return c;if(x==1)return c;return c;}int main(){volatile int n=8000000;C c(3);int sum=0;for(int i=0;i<n;++i){c.n=i%4;sum+=f<int>(c,i%3);}return sum!=12000000||count!=8000000;}')
 else:src.write_text(default_source(*generated[name]))
 outputs=[];commands[name]={};runtimes[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if not b and name in old['workloads']: assert item['sha256']==oldrow['outputs'][1]['sha256'],name
  if (oldrow and 'runtime' in oldrow) or name=='return-runtime':
   exe=WORK/f'{name}-{b}';shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 comparison=compare(src,outputs,WORK/'comparison',name) if outputs[0]['sha256']!=outputs[1]['sha256'] else None
 if runtimes[name]: assert outputs[0]['native']['text_bytes']==outputs[1]['native']['text_bytes'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,shape=generated.get(name),comparison=comparison)
 if name in generated:
  n,k,m,q=generated[name];stats=outputs[1]['telemetry'][0]
  assert stats['semantic_statement_conversion_work']==m+1
  assert stats['semantic_statement_conversion_uses']==k*(m+1)
  assert stats['semantic_body_checks']==stats['semantic_body_lifetime_checks']
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

#!/usr/bin/env python3
"""Source obligation ownership: frozen AA/ABBA evidence."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
from body_compare import compare
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/destination-performance.json';old=json.loads(parent.read_text())
names=list(old['workloads'])
shapes=[(16000,1,8,1),(64000,1,8,1),(16000,128,8,1),(16000,1,8,4000)]
generated={('%s-%d-%d-%d-%d'%((kind,)+shape)):(kind,shape) for kind in ['source-default','source-operator'] for shape in shapes}
names+=list(generated)+['source-default-runtime','source-operator-runtime']
def source(kind,n,k,m,q):
 lines=['using U%d=int;'%i for i in range(n)]
 if kind=='source-default':
  lines+=['int calls;struct Leaf{int n;Leaf(int x=3):n(x){++calls;}};struct C{Leaf part;};']
  body=''.join('C v%d;'%i for i in range(m))+'return x+'+'+'.join('v%d.part.n'%i for i in range(m))+';'
 else:
  lines+=['int calls;struct C{int n;C(int x):n(x){}int operator()(int x=2){++calls;return n+x;}explicit operator int(){++calls;return n;}};']
  body=''.join('C v%d(x+%d);'%(i,i) for i in range(m))+'return '+'+'.join('v%d(2)+int(v%d)'%(i,i) for i in range(m))+';'
 lines+=['template<class T>int f(int x){'+body+'}']
 lines+=['struct Tag%d;'%i for i in range(k)]
 lines+=['int uses(){int s=0;'+''.join('s+=f<Tag%d>(2);'%i for i in range(k))+'s+=f<Tag0>(2);'*(q-1)+'return s;}']
 return '\n'.join(lines)+'\n'
binaries=[A.resolve(),B.resolve()]
result=dict(started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),text_metric='compiler .text; sectionless native executable payload after ELF entry; checked native outcomes and complete executable hashes reported',protocol='preflight all equivalent correct outputs; one warmup each; four A/A observations and two ABBA blocks; no concurrent builds/tests',cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],acceptance='one source recipe per fixed use; declaration properties separate from bodies/lifetimes; concrete work proportional to instantiated uses; PA14/O0 has no optional runtime transform or numerical latency/RSS gate',workloads={})
commands={};runtimes={}
for name in names:
 oldrow=old['workloads'].get(name)
 src=WORK/(name+'.cpp')
 if oldrow:
  assert shared.sha(oldrow['source_path'])==oldrow['source_sha256']
  src.write_bytes(Path(oldrow['source_path']).read_bytes())
 elif name in ['source-default-runtime','source-operator-runtime']:
  kind=name.rsplit('-',1)[0]
  text=source(kind,0,1,1,1)
  n,total,calls=(4000000,18000000,4000000) if kind=='source-default' else (8000000,40000000,16000000)
  src.write_text(text+'int main(){volatile int n=%d;int sum=0;for(int i=0;i<n;++i)sum+=f<int>(i%%4);return sum!=%d||calls!=%d;}\n'%(n,total,calls))
 else:src.write_text(source(generated[name][0],*generated[name][1]))
 shape=generated[name][1] if name in generated else None
 kind=generated[name][0] if name in generated else None
 outputs=[];commands[name]={};runtimes[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if not b and name in old['workloads']: assert item['sha256']==oldrow['outputs'][1]['sha256'],name
  if (oldrow and 'runtime' in oldrow) or name in ['source-default-runtime','source-operator-runtime']:
   exe=WORK/f'{name}-{b}';shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 comparison=compare(src,outputs,WORK/'comparison',name) if outputs[0]['sha256']!=outputs[1]['sha256'] else None
 if runtimes[name]: assert outputs[0]['native']['text_bytes']==outputs[1]['native']['text_bytes'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,shape=shape,kind=kind,comparison=comparison)
 OUT.write_text(json.dumps(result,indent=2)+'\n')
 if shape:
  n,k,m,q=shape;stats=outputs[1]['telemetry'][0]
  if kind=='source-default':
   assert stats['semantic_default_initialization_work']==m
   assert stats['semantic_default_initialization_uses']==k*m
  else:
   assert stats['semantic_initializer_recipe_work']==m
   assert stats['semantic_initializer_recipe_uses']==k*m
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

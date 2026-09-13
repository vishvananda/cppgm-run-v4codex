#!/usr/bin/env python3
"""Declaration/body/default demand scaling; frozen compiler/native AA/ABBA."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='freeze all inputs/outputs before timing; one warmup per binary; four A/A observations and two ABBA blocks for common-correct inputs; six B observations for new behavior',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['ca42e7064defc8459d99b74e66af28bf1f83c949',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF payload after entry',
 acceptance='O0 required region/default ownership; zero generated-code growth for common-correct reuse; complete-key work/storage; no optional optimizer or mandated numeric compiler gate',workloads={})
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 order=shared.ORDER if len(commands)==2 else [1]*6
 rows=[dict(binary=b,**observe(commands[b])) for b in order]
 data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  data.update(aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
   paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
 return data
def pattern(width):
 return 'template<class T>struct C{int n;C(int x=sizeof(T)):n(x){}int run(int x){return n+x;}int large(int x){'+'x+=sizeof(T);'*width+'return x;}};\n'
corpus=[]
for n,width,demand in ((1000,8,False),(1000,128,False),(4000,128,False),(1000,8,True),(1000,128,True)):
 use='large' if demand else 'run'
 source=pattern(width)+''.join(f'struct Tag{i}{{}};int run{i}(C<Tag{i}>& c,int x){{return c.{use}(x);}}\n' for i in range(n))
 corpus.append((f'body-{use}-{n}-{width}',source,False,True))
for n in (1000,4000):
 source='template<class T>struct C{int f(int n=sizeof(T)+1){return n;}};\n'+''.join(f'struct Tag{i}{{}};C<Tag{i}> c{i};\n' for i in range(n))
 corpus.append((f'default-unused-{n}',source,False,True))
 source='template<class T>int f(int n=sizeof(T)){return n;}\n'+''.join(f'int run{i}(){{return f<int>();}}\n' for i in range(n))
 corpus.append((f'default-repeated-{n}',source,False,True))
 source='template<class T>struct C{int f(int n=T::missing){return n;}};\n'+''.join(f'struct Tag{i}{{}};C<Tag{i}> c{i};\n' for i in range(n))
 corpus.append((f'default-dependent-{n}',source,False,False))
count=12000000
corpus.append(('region-runtime',pattern(128)+f'int main(){{volatile int count={count};C<int> c(7);int sum=0;for(int i=0;i<count;++i)sum=(sum+c.run(4))&65535;return sum!={(11*count)&65535};}}',True,True))
source='int calls=0;int next(){return ++calls;}template<class T>struct C{int n;C(int x=T::missing):n(x){}int f(int x=T::missing){return n+x;}int count(int x=next()){return x;}};'
source+=f'int main(){{volatile int count={count};C<int> c(7);int sum=0;for(int i=0;i<count;++i)sum=(sum+c.f(4)+(c.count()&1))&65535;return sum!={(11*count+count//2)&65535}||calls!=count;}}'
corpus.append(('dependent-default-runtime',source,True,False))
parent=ROOT/'student.tests/pa14/declaration-type-performance.json';old=json.loads(parent.read_text())
result.update(parent_path=str(parent),parent_sha256=shared.sha(parent))
for name in ('declaration-instances-1000','declaration-outside-1000','member-repeated-1000','calls-4','memory-float-1','calls-runtime','memory-runtime','floating-runtime'):
 item=old['workloads'][name];assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item,True))
commands={};runtimes={}
for name,source,executable,common in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands[name]={};runtimes[name]={}
 entry_rejection=None
 if not common:
  log=WORK/(name+'-entry.log')
  import subprocess
  r=subprocess.run([binaries[0],'--emit-lowir','-O0','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True)
  log.write_text(r.stdout+r.stderr);assert r.returncode==1,(name,r.returncode)
  entry_rejection=dict(path=str(log),sha256=shared.sha(log),exit_code=r.returncode)
 for b in ((0,1) if common else (1,)):
  out=WORK/(name+f'-{b}.lowir');command=[binaries[b],'--emit-lowir','-O0','-o',out,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,path=str(out),sha256=shared.sha(out),bytes=out.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  commands[name][b]=command;outputs.append(item)
 if common:
  assert outputs[0]['sha256']==outputs[1]['sha256'],name
  if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode='--emit-lowir',outputs=outputs,exact_required=common,equivalence='byte-identical correct compiler/native output' if common else 'new C++11 default deferral; checked B-only baseline')
 if entry_rejection:item['entry_rejection']=entry_rejection
 result['workloads'][name]=item;print(name,'preflight',flush=True)
OUT.write_text(json.dumps(result,indent=2)+'\n')
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

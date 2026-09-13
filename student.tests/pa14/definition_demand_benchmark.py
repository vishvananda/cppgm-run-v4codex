#!/usr/bin/env python3
"""Matched definition ownership: frozen corpus and source/key/request scaling."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/expression-view-performance.json';old=json.loads(parent.read_text())
binaries=[A.resolve(),B.resolve()]
result=dict(protocol='freeze every input and equivalent output before timing; one warmup each; four A/A observations and two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['167f5f43',shared.run(['git','rev-parse','HEAD']).stdout.strip()],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 acceptance='ordinary source signature work per prototype; one matched definition per demanded member; completed member/head requests; parameter fact substitution; bounded source/key/use storage and zero generated growth',workloads={})
corpus=[]
for name,item in old['workloads'].items():
 assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item))
def pattern(width):
 return ''.join(f'struct Arg{i}{{}};' for i in range(width))+'template<class T> struct Pick {'+''.join(f'static int f(const Arg{i}&,int);' for i in range(width))+'};\n'+''.join(f'template<class T> int Pick<T>::f(const Arg{i}&,int x){{return x+{i}+sizeof(T);}}\n' for i in range(width))
for n,width,requests in ((1000,8,4),(1000,128,4),(4000,8,4),(1000,8,64)):
 source=pattern(width)+''.join(f'struct Tag{i}{{}};int run{i}(int x){{Arg0 a;'+f'x=Pick<Tag{i}>::f(a,x);'*requests+'return x;}\n' for i in range(n))
 corpus.append((f'demand-uses-{n}-{width}-{requests}',source,False))
count=12000000
source=pattern(8)+f'int main(){{volatile int count={count};unsigned long sum=0;Arg0 a;for(int i=0;i<count;++i){{sum=(sum'+ '+Pick<int>::f(a,i)'*4+f')&65535;}}return sum!={(4*count*(count-1)//2+16*count)&65535};}}'
corpus.append(('demand-runtime',source,True))
commands={};runtimes={}
for name,source,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands[name]={};runtimes[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/(name+f'-{b}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if name in old['workloads']:assert item['sha256']==old['workloads'][name]['outputs'][-1]['sha256'],name
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs)
 print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
OUT.write_text(json.dumps(result,indent=2)+'\n')
if os.environ.get('PREFLIGHT_ONLY'):sys.exit(0)
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

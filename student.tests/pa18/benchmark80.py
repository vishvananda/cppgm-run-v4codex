#!/usr/bin/env python3
"""Loop 80 frozen PA18/O0 compiler and checked executable observations: A B WORK OUT [PREFIX]."""
from pathlib import Path
import json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A samples then four ABBA blocks; six final-only samples for new behavior',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 commits=['c5e2c2719f1ef004e6bddb86487e797c37471079',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA18/O0 has no mandated numeric compiler/runtime ceiling. Existing semantics compare exact or executed equivalent outputs; newly required semantics are final-only costs. No optional transform or growth is introduced. Native backend and self-hosting belong to later stages.',
 text_metric='Compiler .text; supplied sectionless ELF payload proxy. Runtime benchmark sources have no static data; scaled frontend-only cases do not emit an executable.',workloads={})
assert not OUT.exists(),'Preserve observations; choose a new output.'
assert not shared.run(['git','diff','HEAD','--','dev']).stdout,'Freeze committed implementation first.'
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands):
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 order=[0]*4+[0,1,1,0]*4 if len(commands)==2 else [1]*6
 rows=[dict(binary=i,**observe(commands[i])) for i in order];data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  aa=[r['wall_s'] for r in rows[:4]];data['aa_range_s']=[min(aa),max(aa)]
  ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
  data['paired_b_over_a']=ratios;data['median_b_over_a']=statistics.median(ratios)
 for i in commands:
  samples=[r for r in rows[(4 if len(commands)==2 else 0):] if r['binary']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in samples),wall_range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['rss_kib'] for r in samples))
 return data
corpus=[]
prior=json.loads((ROOT/'student.tests/pa18/loop66-performance.json').read_text())['workloads']
for name in ('ordering-600','ordering-2400','member-head-600','member-head-2400','common-loop-float-1500','runtime-calls','runtime-memory','runtime-floating'):
 corpus.append((name,prior[name]['source'],True,name.startswith('runtime-')))
changed=set()
for n in (600,2400):
 source='template<class T>int consume(T&&p,int n){return p?n:n+1;}'+''.join('int f'+str(i)+'(int n){return consume((int*)0,n);}' for i in range(n))+'int main(){return f0(7)!=8||f'+str(n-1)+'(8)!=9;}'
 name='pointer-temporaries-'+str(n);changed.add(name);corpus.append((name,source,True,False))
 source='template<int I,class T>auto narrow(T n)->decltype(static_cast<const short&>(n),int()){const short&r=static_cast<const short&>(n);return r;}'+''.join('int f'+str(i)+'(int n){return narrow<'+str(i)+'>(n);}' for i in range(n))+'int main(){return f0(65543)!=7||f'+str(n-1)+'(65545)!=9;}'
 corpus.append(('cast-reference-'+str(n),source,False,False))
 source='template<int N>struct R{static const int value=N;};template<int I>struct V{constexpr operator int()const{return 65536+I;}};template<class T>auto f()->R<static_cast<const short&>(T())>;'+''.join('static_assert(decltype(f<V<'+str(i)+'>>())::value=='+str(i)+',"");' for i in range(n))+'int main(){}'
 corpus.append(('constant-reference-'+str(n),source,False,False))
n=18000000;expected=((n//1024)*sum(range(1,1025))+sum(range(1,n%1024+1)))%65536
source='template<class T>int step(T&&p,int n){return p?n:n+1;}int main(){'+f'volatile int n={n};int s=0;for(int i=0;i<n;++i){{s=(s+step((int*)0,i&1023))&65535;}}return s!={expected};}}'
changed.add('runtime-pointer-temporary');corpus.append(('runtime-pointer-temporary',source,True,True))
n=12000000;expected=((n//1024)*sum(range(1024))+sum(range(n%1024)))%65536
source='template<class T>auto step(T n)->decltype(static_cast<const short&>(n),int()){const short&r=static_cast<const short&>(n);return r;}int main(){'+f'volatile int n={n};int s=0;for(int i=0;i<n;++i){{s=(s+step(65536+(i&1023)))&65535;}}return s!={expected};}}'
corpus.append(('runtime-cast-reference',source,False,True))
if len(sys.argv)>5:corpus=[x for x in corpus if x[0].startswith(sys.argv[5])]
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
empty=WORK/'empty.cpp';empty.write_text('int main(){}')
result['startup']=measure({i:[cc,'--emit-lowir','-O0','-o',WORK/f'empty-{i}.lowir',empty] for i,cc in enumerate((A,B))});save()
for name,source,equivalent,runtime in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=shared.sha(src),comparison=('executed-equivalent' if name in changed else 'exact') if equivalent else 'new-behavior',outputs=[]);result['workloads'][name]=item
 commands={};executables={}
 for i,cc in enumerate((A,B)):
  ir=WORK/(name+f'-{i}.lowir');command=[cc,'--emit-lowir','-O0','-o',ir,src]
  r=subprocess.run([str(x) for x in [*command,'--validate-lowir','--stats']],capture_output=True,text=True,timeout=300)
  if r.returncode:
   assert i==0 and not equivalent,(name,i,r.stderr)
   item['entry_rejection']=dict(exit=r.returncode,diagnostic=r.stderr);continue
  commands[i]=command
  out=dict(binary=i,sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if 'int main(' in source:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(sha256=shared.sha(exe),payload_bytes=shared.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)
 if len(commands)==2:
  if name not in changed:
   assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
   if executables:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
  else:assert executables.keys()=={0,1},name
 else:assert not equivalent and 1 in commands,name
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if runtime:item['runtime']=measure(executables)
 save();print(name,'measured',flush=True)
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

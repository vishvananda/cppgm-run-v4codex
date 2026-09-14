#!/usr/bin/env python3
"""Frozen PA16 checkpoint audit evidence. Run explicitly with A B WORK OUT."""
from pathlib import Path
import json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='frozen inputs/binaries/flags; one warmup each; four A/A then two ABBA blocks; new-correct-only six B samples',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA16/O0 required semantic work; no optional transform or mandated numeric ceiling; 512 active calls and 1000000 executed statement/expression/loop steps per root; diagnostic targets only: common compiler latency +15%, RSS +16MiB, 4x work scaling <=5.5x; investigate avoidable costs, retain all observations',
 text_metric='compiler .text; sectionless native payload after ELF entry including alignment; native workloads have no static data',workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<class T>constexpr int f(T x){return x*3+7;}\n'+''.join(f'static_assert(f({i})=={i*3+7}, "");\n' for i in range(n))
 corpus.append((f'template-{n}',source,'exact',False))
 source=''.join(f'double f{i}(double*p,int n){{double s=0;for(int i=0;i<n;++i)s+=p[i]*2;return s;}}\n' for i in range(n))
 corpus.append((f'memory-float-{n}',source,'exact',False))
 source=''.join(f'struct C{i}{{int x;constexpr C{i}(int n):x(n){{}}constexpr int f()const{{return x;}}}};int f{i}(int x){{return C{i}(x).f();}}\n' for i in range(n))
 corpus.append((f'literal-classes-{n}',source,'exact',False))
 source=''.join(f'struct C{i}{{int x;C{i}()=default;}};static_assert(noexcept(C{i}()), "");\n' for i in range(n))
 corpus.append((f'exception-defaults-{n}',source,'exact',False))
 source='template<class T>void f()noexcept(sizeof(T)>1);\n'+''.join(f'struct C{i}{{char x[{i+2}];}};static_assert(noexcept(f<C{i}>()), "");static_assert(noexcept(f<C{i}>()), "");\n' for i in range(n))
 corpus.append((f'exception-dependent-{n}',source,'exact',False))
for n in [1000,4000]:
 source='template<class T>constexpr double f(T n){return (n+0.5)*2.0;}\n'+''.join(f'static_assert(f({i})=={2*i+1}.0, "");\n' for i in range(n))
 corpus.append((f'floating-values-{n}',source,'exact',False))
 source='template<class T>constexpr int f()noexcept(sizeof(T)>1){return sizeof(T);}\n'+''.join(f'struct C{i}{{char x[{i+2}];}};static_assert(noexcept(f<C{i}>()), "");static_assert(f<C{i}>()=={i+2}, "");static_assert(noexcept(f<C{i}>()), "");\n' for i in range(n))
 corpus.append((f'exception-body-{n}',source,'entry-rejected',False))
for name,source in shared.runtimes(factor=12):corpus.append(('runtime-'+name,source,'exact',True))
n=60000000
expected=(n//1024*sum(range(1024))+sum((i+3)&1023 for i in range(n%1024)))&65535
source=f'struct C{{int x;constexpr C(int n):x(n){{}}constexpr int f()const{{return (x+3)&1023;}}}};int step(int x){{return C(x).f();}}int main(){{volatile int n={n};int sum=0;for(int i=0;i<n;++i)sum=(sum+step(i))&65535;return sum=={expected}?0:1;}}'
corpus.append(('runtime-literal-class',source,'exact',True))
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands):
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in (shared.ORDER if len(commands)==2 else [1]*6)]
 data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  data['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in [4,8]]
 return data
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,comparison,native in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
 common=comparison=='exact';commands={};executables={}
 if not common:
  ir=WORK/(name+'-entry.lowir');r=subprocess.run([A,'--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True)
  item['entry_probe']=dict(exit_code=r.returncode,stderr=r.stderr)
  assert r.returncode!=0,(name,r.stderr)
  if not r.returncode:item['entry_probe'].update(path=str(ir),sha256=shared.sha(ir))
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');command=[[A,B][i],'--emit-lowir','-O0','-o',ir,src];commands[i]=command
  r=shared.run([*command,'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if native:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
 if common:
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

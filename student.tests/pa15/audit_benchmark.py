#!/usr/bin/env python3
"""PA15 cumulative and checkpoint comparisons; frozen A/A, ABBA, absolute new work."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);profile=sys.argv[5]
A,B=A.resolve(),B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(profile=profile,cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),
 source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0']),
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 protocol='single pinned CPU; exact validated output preflight; warmups; four A/A samples then two ABBA blocks; six B samples when A rejects',
 acceptance='PA15/O0: correctness, complete identities, near-linear produced work; no numerical latency/RSS/text gate; no optional optimization',
 text_metric='compiler .text; sectionless supplied-backend payload after entry, no static data',workloads={})
corpus=[]
for n in [1000,4000]:
 if profile=='stage':
  source='template<class T>int f(int n){return n+sizeof(T);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(int n){{return f<Tag{i}>(n);}}\n' for i in range(n))
  corpus.append((f'types-{n}',source,True,False))
 if profile=='checkpoint':
  source='template<class T,T N,T M=N+1>struct C{static const T value=M;};\n'+''.join(f'int run{i}(){{return C<int,{i}>::value;}}\n' for i in range(n))
  corpus.append((f'defaults-{n}',source,True,False))
  source='template<int N>int f(){return N;}\n'+''.join(f'int run{i}(){{return f<{i}>()+f<{i//2}+{i-i//2}>();}}\n' for i in range(n))
  corpus.append((f'values-{n}',source,True,False))
  source='template<int N>struct C;\n'+''.join(f'template<>struct C<{i}>{{static const int v={i};}};int run{i}(){{return C<{i}>::v;}}\n' for i in range(n))
  corpus.append((f'selected-classes-{n}',source,True,False))
  source='int sum(int a,int b){return a+b;}\ntemplate<class U,class...T>int size(T...){return sizeof(U)+sizeof...(T);}\ntemplate<class...>struct L{};\ntemplate<class...U,class...T>int f(L<U...>,T...t){return sum(size<U>(t...)...);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(){{return f(L<char,short>(),Tag{i}());}}\n' for i in range(n))
  corpus.append((f'nested-packs-{n}',source,True,False))
  source='template<class R,class...T>R f(T...){return sizeof...(T);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(){{int(*p)(Tag{i}&,long&)=f;Tag{i} t;long v=0;return p(t,v);}}\n' for i in range(n))
  corpus.append((f'pack-targets-{n}',source,False,False))
  source='template<int...N>int f(){return sizeof...(N);}\n'+''.join(f'template<>int f<{i},2>(){{return {i};}}int run{i}(){{return f<{i},2>();}}\n' for i in range(n))
  corpus.append((f'pack-selections-{n}',source,False,False))
 # Exercise the parameter substitution owner that previously allocated one
 # extra vector per scalar parameter. Output contains real calls at O0.
 source='template<class Tag,class T>int f('+','.join(f'T a{j}' for j in range(16))+'){return a0+a15;}\n'
 source+=''.join(f'struct Tag{i}{{}};int run{i}(int n){{return f<Tag{i}>(n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n);}}\n' for i in range(n))
 corpus.append((f'wide-signatures-{n}',source,True,False))
if profile=='stage':
 corpus.append(('constants',''.join(f'static_assert((({i}&255)+7)=={(i&255)+7},"value");\n' for i in range(12000))+'int main(){return 0;}',True,False))
for name,source in shared.runtimes(factor=10):corpus.append(('runtime-'+name,source,True,True))
if profile=='checkpoint':
 source=dict(shared.runtimes(factor=10))['calls'].replace('int main()','template<class...T>int forward(T...t){return step(t...);}int main()').replace('s+step(i)','s+forward(i)')
 corpus.append(('runtime-pack',source,True,True))
 source=dict(shared.runtimes(factor=10))['calls'].replace('int main()','template<class...T>int forward(T...t){return 0;}template<>int forward<int>(int n){return step(n);}int main()').replace('s+step(i)','s+forward(i)')
 corpus.append(('runtime-selected-pack',source,False,True))
commands={};native={}
for name,source,common,execute in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);commands[name]={};native[name]={}
 item=dict(source_path=str(src),source_sha256=shared.sha(src),common_correct=common,outputs=[]);result['workloads'][name]=item
 if not common:
  r=subprocess.run([A,'--emit-lowir','-O0','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True)
  item['entry_probe']=dict(exit=r.returncode,stderr=r.stderr);assert r.returncode,(name,'baseline unexpectedly accepts')
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');command=[[A,B][i],'--emit-lowir','-O0','-o',ir,src];commands[name][i]=command
  r=shared.run([*command,'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,
    telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if execute:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);native[name][i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
 if common:
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],(name,'LowIR differs')
  if execute:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],(name,'native differs')
 print(name,'preflight',flush=True)
OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command]);rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands):
 warmups=[dict(binary=i,**observe(command)) for i,command in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in (shared.ORDER if len(commands)==2 else [1]*6)]
 data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  data['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in [4,8]]
 return data
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,item in result['workloads'].items():
 item['compiler']=measure(commands[name])
 if native[name]:item['runtime']=measure(native[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
for item in result['workloads'].values():
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256'] # audit/stats separation, determinism
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());OUT.write_text(json.dumps(result,indent=2)+'\n')

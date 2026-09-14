#!/usr/bin/env python3
"""Frozen A/A and ABBA compiler/native measurements, plus new pack work scaling."""
from pathlib import Path
import json, os, platform, statistics, sys, time, subprocess
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A,B=A.resolve(),B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='fixed binaries/inputs/flags; exact output preflight; warmup; four A/A samples; two ABBA blocks; B-only six samples for new behavior',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 entry_commit='19225b3deef4a85e5690c2d72fb7857f5a06721c',implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0']),
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 text_metric='compiler .text; sectionless supplied-backend native payload after entry (no static data)',
 acceptance=dict(stage='PA15 O0',numerical_limits=None,optimization='none',
 requirements='correct equivalent outputs; O(n) or O(n log n) consumed/produced work; complete keys and TU lifetime',
 new_behavior='absolute compiler/runtime/RSS/text only; entry lacks the behavior, so no speedup claim'),workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<class T>int f(int n){return n+sizeof(T);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(int n){{return f<Tag{i}>(n);}}\n' for i in range(n))
 corpus.append((f'types-{n}',source,True,False))
 source='template<class T,T N,T M=N+1>struct C{static const T value=M;};\n'+''.join(f'int run{i}(){{return C<int,{i}>::value;}}\n' for i in range(n))
 corpus.append((f'defaults-{n}',source,True,False))
 source='template<class...T>int f(T...){return sizeof...(T);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(){{return f<Tag{i}>(Tag{i}(),1,2L);}}\n' for i in range(n))
 corpus.append((f'pack-signatures-{n}',source,False,False))
 source='int sum(int a,int b){return a+b;}\ntemplate<class U,class...T>int size(T...){return sizeof(U)+sizeof...(T);}\ntemplate<class...>struct L{};\ntemplate<class...U,class...T>int f(L<U...>,T...t){return sum(size<U>(t...)...);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(){{return f(L<char,short>(),Tag{i}());}}\n' for i in range(n))
 corpus.append((f'nested-packs-{n}',source,False,False))
source=''.join(f'static_assert((({i}&255)+7)=={(i&255)+7},"value");\n' for i in range(12000))+'int main(){return 0;}'
corpus.append(('constants',source,True,False))
for name,source in shared.runtimes(factor=10):corpus.append(('runtime-'+name,source,True,True))
source=dict(shared.runtimes(factor=10))['calls'].replace('int main()', 'template<class...T>int forward(T... t){return step(t...);} int main()').replace('s+step(i)','s+forward(i)')
corpus.append(('runtime-pack-call',source,False,True))
commands={};runtimes={}
for name,source,common,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),common_correct=common,outputs=[])
 result['workloads'][name]=item;commands[name]={};runtimes[name]={}
 if not common:
  entry=WORK/(name+'-entry.lowir');p=subprocess.run([A,'--emit-lowir','-O0','-o',entry,src],capture_output=True,text=True)
  item['entry_probe']=dict(exit=p.returncode,stderr=p.stderr)
  assert p.returncode!=0,(name,'entry unexpectedly accepts new benchmark; verify equivalence before measuring')
 for i in ([0,1] if common else [1]):
  binary=[A,B][i];ir=WORK/(name+f'-{i}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);runtimes[name][i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
  item['outputs'].append(out);commands[name][i]=command
 if common:
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],(name,'LowIR differs')
  if executable:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],(name,'native differs')
 print(name,'preflight',flush=True)
OUT.write_text(json.dumps(result,indent=2)+'\n')
if os.environ.get('PREFLIGHT_ONLY'):sys.exit(0)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def measure(commands):
 warms=[dict(binary=i,**observe(command)) for i,command in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in (shared.ORDER if len(commands)==2 else [1]*6)]
 data=dict(warmups=warms,observations=rows)
 if len(commands)==2:
  data['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in [4,8]]
 return data
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,item in result['workloads'].items():
 item['compiler']=measure(commands[name])
 if runtimes[name]:item['runtime']=measure(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
OUT.write_text(json.dumps(result,indent=2)+'\n')

#!/usr/bin/env python3
"""Frozen PA15/O0 compiler and executable evidence; no optional optimization."""
from pathlib import Path
import json, os, platform, statistics, sys, time, subprocess
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
A,B=A.resolve(),B.resolve()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='preflight equivalence; one warmup each; four A/A observations; two ABBA blocks; new-only absolute observations (six B samples), no speedup claim',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 baseline_commit='8000f3c8ef4647d57f2c0775192585f14cab33d8',implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0']),
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 text_metric='compiler .text, native executable payload after ELF entry (sectionless supplied backend); no static data in executable workloads',
 acceptance=dict(stage='PA15 O0',mandated='correct outputs; linear or n log n consumed work; complete canonical keys; bounded TU lifetime',
 numerical_limits=None,optimizer='none',new_behavior='absolute measurements only: baseline rejects; not an equivalent performance comparison'),workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<class T>int f(int x){return x+sizeof(T);}\n'+''.join(f'struct Tag{i}{{}};int run{i}(int x){{return f<Tag{i}>(x);}}\n' for i in range(n))
 corpus.append((f'types-{n}',source,True,False))
 source='template<int N>int f(int x){return x+N;}\n'+''.join(f'int run{i}(int x){{return f<{i}>(x)+f<{i}+1-1>(x);}}\n' for i in range(n))
 corpus.append((f'values-{n}',source,False,False))
for n in [1000,4000]:
 source='template<class T,T N,T M=N+1>struct C{static const T value=M;};\n'+''.join(f'int run{i}(){{return C<int,{i}>::value;}}\n' for i in range(n))
 corpus.append((f'defaults-{n}',source,False,False))
source=''.join(f'static_assert((({i}&255)+7)=={(i&255)+7},"value");\n' for i in range(12000))+'int main(){return 0;}'
corpus.append(('constants',source,True,False))
for name,source in shared.runtimes(factor=10):corpus.append(('runtime-'+name,source,True,True))
n=60000000
expected=((n//1024)*sum((i*17+3)&1023 for i in range(1024))+sum((i*17+3)&1023 for i in range(n%1024)))&65535
source=f'template<int N>int step(int x){{return (x*N+3)&1023;}} int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s=(s+step<17>(i))&65535;return s=={expected}?0:1;}}'
corpus.append(('runtime-values',source,False,True))
commands={};runtimes={}
for name,source,common,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),common_correct=common,outputs=[])
 result['workloads'][name]=item;commands[name]={};runtimes[name]={}
 if not common:
  p=subprocess.run([str(A),'--emit-lowir','-O0','-o',str(WORK/(name+'-entry.lowir')),str(src)],capture_output=True,text=True)
  assert p.returncode!=0,(name,'baseline unexpectedly accepts')
  item['baseline_rejection']=dict(exit_code=p.returncode,stderr=p.stderr)
 for i in ([0,1] if common else [1]):
  binary=[A,B][i];ir=WORK/(name+f'-{i}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  telemetry=shared.run([*command,'--stats','--validate-lowir'])
  out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in telemetry.stderr.splitlines()])
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

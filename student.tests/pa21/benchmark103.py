#!/usr/bin/env python3
"""Frozen PA21/O0 compiler and native evidence. Run BASE FINAL WORK OUTPUT."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as common
A,B,WORK,OUT = [Path(p).resolve() for p in sys.argv[1:]]
assert not OUT.exists(), 'Preserve observations from earlier runs'
WORK.mkdir(parents=True,exist_ok=True)
cpu = max(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
result = dict(protocol='one warmup each; four A/A observations; four ABBA blocks; six final-only observations for new semantics',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],
 base_commit='f4224e0b73f6eca5e6c607fe119026f76e03e6f5',
 final_commit=common.run(['git','rev-parse','HEAD']).stdout.strip(),
 host=common.run(['g++','--version']).stdout.splitlines()[0],
 binaries=[dict(path=str(p),sha256=common.sha(p),text_bytes=common.text_size(p)) for p in (A,B)],
 harness_sha256=common.sha(__file__),shared_harness_sha256=common.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend=dict(sha256=common.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0']),
 text_metric='compiler .text; supplied sectionless native ELF executable payload proxy includes static data',
 acceptance='spec section 9 PA21/O0; no optional optimization or additional numerical gate; correctness costs reported separately',
 workloads={})
def save(): OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(cmd):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 common.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*cmd])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),
             involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def measure(commands):
 both=len(commands)==2
 warmups=[dict(binary=i,**observe(cmd)) for i,cmd in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in ([0]*4+[0,1,1,0]*4 if both else [1]*6)]
 out=dict(warmups=warmups,observations=rows)
 if both:
  aa=[r['wall_s'] for r in rows[:4]];out['aa_range_s']=[min(aa),max(aa)]
  out['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
 for i in commands:
  values=[r for r in rows[4 if both else 0:] if r['binary']==i]
  out[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in values),wall_range_s=[min(r['wall_s'] for r in values),max(r['wall_s'] for r in values)],peak_rss_kib=max(r['rss_kib'] for r in values))
 return out
old=json.loads((ROOT/'student.tests/pa20/audit101-performance-stage.json').read_text())
sources={name:old['workloads'][name]['source'] for name in ('startup','auto-specializations-9600','runtime-calls','runtime-memory','runtime-floating')}
sources['runtime-reference-captures']='int main(){volatile int n=4000000;int total=0;auto f=[&](int value){total+=value+1;};for(int i=0;i<n;++i)f(i&1);return total!=6000000;}'
new_workloads=set()
for n in (800,3200):
 name='copy-capture-specializations-'+str(n);new_workloads.add(name)
 sources[name]='template<int N>int f(int n){auto g=[n](){return n+N;};return g();}int main(){int sum=0;'+''.join(f'sum+=f<{i}>(1);sum+=f<{i}>(1);' for i in range(n))+f'return sum!={n*(n+1)};}}'
for n in (16,64,256):
 name='capture-fields-'+str(n);new_workloads.add(name)
 sources[name]='int main(){'+''.join(f'int n{i}={i};' for i in range(n))+'auto f=[=](){return '+'+'.join(f'n{i}' for i in range(n))+';};return f()!='+str(n*(n-1)//2)+';}'
for n in (32,4096):
 name='capture-array-'+str(n);new_workloads.add(name)
 sources[name]=f'int main(){{int a[{n}]={{}};a[{n-1}]=7;auto f=[a](){{return a[{n-1}];}};return f()!=7;}}'
sources['runtime-copy-captures']='int main(){volatile int n=4000000;int total=0;for(int i=0;i<n;++i){int value=i&1;auto f=[value](){return value+1;};total+=f();}return total!=6000000;}'
new_workloads.add('runtime-copy-captures')
for name,source in sources.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=common.sha(src),outputs=[],runtime_timing='live checked loop' if name.startswith('runtime-') else 'startup control')
 result['workloads'][name]=item;save();commands={};executables={}
 for i,cc in enumerate((A,B)):
  ir=WORK/(name+f'-{i}.lowir');exe=WORK/(name+f'-{i}')
  cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
  check=subprocess.run([str(x) for x in [*cmd,'--stats','--validate-lowir']],capture_output=True,text=True,timeout=300)
  if check.returncode:
   assert i==0,(name,check.stderr);item['base_rejection']=dict(exit=check.returncode,diagnostic=check.stderr);continue
  common.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
  native=subprocess.run([exe],timeout=60)
  if native.returncode:
   assert i==0,(name,native.returncode);item['base_incorrect']=dict(exit=native.returncode,lowir_sha256=common.sha(ir));continue
  if i==0 and name in new_workloads:
   item['base_excluded']='entry lacks value capture semantics';continue
  item['outputs'].append(dict(binary=i,lowir_sha256=common.sha(ir),lowir_bytes=ir.stat().st_size,
   telemetry=[json.loads(line) for line in check.stderr.splitlines()],
   native=dict(sha256=common.sha(exe),payload_bytes=common.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)))
  commands[i]=cmd;executables[i]=[exe]
 assert 1 in commands
 item['comparison']='equivalent correct source and checked results' if len(commands)==2 else 'new semantics; no A/B benefit claim'
 item['compiler']=measure(commands);item['runtime']=measure(executables)
 for output in item['outputs']:assert common.sha(WORK/(name+f"-{output['binary']}.lowir"))==output['lowir_sha256']
 save();print(name,'complete',flush=True)
assert [common.sha(p) for p in (A,B)]==[x['sha256'] for x in result['binaries']]

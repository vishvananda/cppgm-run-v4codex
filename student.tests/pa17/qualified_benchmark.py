#!/usr/bin/env python3
"""PA17 qualified lookup observations: A B WORK OUT."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A samples followed by four ABBA blocks; final-only six samples',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 source_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA17/O0 has no mandated numerical ceiling. Compare correct equivalent common outputs; final-only semantic costs are not optimization claims. Own native backend and self-hosting are later stages.',
 native_size_metric='Sectionless executable payload after ELF entry; runtime inputs have no static data.',workloads={})
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
mode='qualified'
result['mode']=mode
corpus=[]
prior=json.loads((ROOT/'student.tests/pa17/checkpoint52-cumulative-performance.json').read_text())['workloads']
for name,w in prior.items():
 if name.startswith(('common-partials-','common-loop-float-','member-qualified-','runtime-')):
  corpus.append((name,w['source'],'exact','runtime' in w))
for n in (1000,4000):
 source='template<int N>struct B{using type=int;};template<int N>struct D:B<N>{using value=typename D::type;value n;int f(){return n;}};\n'
 source+=''.join(f'int f{i}(){{D<{i}>d;d.n={i%97};return d.f();}}\n' for i in range(n))+'int main(){return f0();}'
 corpus.append((f'inherited-qualified-{n}',source,'entry-rejected',False))
 source='namespace N{inline namespace V{template<int I>int f(){return I;}\n'
 source+=''.join(f'int g{i}(){{return N::f<{i}>();}}\n' for i in range(n))+'}}int main(){return N::g0();}'
 corpus.append((f'inline-qualified-{n}',source,'entry-rejected',False))
 source='template<int N>struct Tag{};template<class A,class B,class...T>struct Second{using type=B;};template<int N,class...T>typename Second<T...>::type run(){return N;}\n'
 source+=''.join(f'int f{i}(){{return run<{i%97},Tag<{i}>,int,long>();}}\n' for i in range(n))+'int main(){return f0();}'
 corpus.append((f'fixed-pack-head-{n}',source,'entry-rejected',False))
n=18000000
source='template<class T>struct Base{using type=T;};template<class T>struct D:Base<T>{using value=typename D::type;value n;int f(){return (n*17+3)&1023;}};namespace N{inline namespace V{template<class T,class U,class...A>struct Second{using type=U;};template<class...T>typename Second<T...>::type make(int n){return n;}int step(int n){D<int>d;d.n=N::make<char,int,long>(n);return d.f();}}}'
cycle=sum((i*17+3)&1023 for i in range(1024));expected=((n//1024)*cycle+sum((i*17+3)&1023 for i in range(n%1024)))&65535
source+=f'int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s=(s+N::step(i))&65535;return s=={expected}?0:1;}}'
corpus.append(('qualified-runtime',source,'entry-rejected',True))
previous={}
if OUT.exists():
 previous=json.loads(OUT.read_text())
 assert not previous.get('finished_utc'), 'completed campaigns are immutable; choose another output'
 assert previous['binaries']==result['binaries'] and previous['mode']==mode
 assert previous['source_commit']==result['source_commit'] and not previous['source_diff']
 result['resumed_observations_from_sha256']=shared.sha(OUT)
 result['previous_harness_sha256']=previous['harness_sha256']
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
result['started_utc']=previous.get('started_utc',time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime()))
for name,source,comparison,native in corpus:
 common=comparison!='entry-rejected'
 old=previous.get('workloads',{}).get(name)
 if old and 'compiler' in old and (not native or 'runtime' in old):
  assert old['source']==source and old['comparison']==comparison
  for out in old['outputs']:
   assert shared.sha(out['path'])==out['sha256']
   if native:assert shared.sha(out['native']['path'])==out['native']['sha256']
  result['workloads'][name]=old;save();print(name,'preserved',flush=True);continue
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
 commands={};executables={}
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');commands[i]=[[A,B][i],'--emit-lowir','-O0','-o',ir,src]
  r=shared.run([*commands[i],'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if native:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
 if comparison=='exact':
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 if not common:
  r=subprocess.run([A,'--emit-lowir','-O0','--validate-lowir','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True,timeout=30)
  assert r.returncode!=0;item['entry_behavior']=dict(compile_exit=r.returncode,stderr=r.stderr,validation=True)
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

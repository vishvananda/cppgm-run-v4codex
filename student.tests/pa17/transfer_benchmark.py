#!/usr/bin/env python3
"""PA17 transfer observations: A B WORK OUT."""
from pathlib import Path
import json, os, platform, re, statistics, subprocess, sys, time
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
 native_size_metric='Sectionless ELF: payload minus typed global bytes reports code plus alignment; retain payload/data/file bytes too.',workloads={})
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

# Compile-only changed workloads repeat the runtime-checked transfer forms.
corpus=[]
prior=json.loads((ROOT/'student.tests/pa17/checkpoint56-performance.json').read_text())['workloads']
for name in ['common-partials-1500','common-partials-6000','common-loop-float-1500','common-loop-float-6000']:
 corpus.append((name,prior[name]['source'],'exact',False))
for n in (600,2400):
 source='template<int>struct E{};template<int N>struct X{E<N>e;long n;};\n'
 source+=''.join(f'long f{i}(long v){{X<{i}>a,b;a.n=7;b=a;return b.n+v;}}\n' for i in range(n))+'int main(){return f0(3)!=10;}'
 corpus.append((f'specialized-transfer-{n}',source,'equivalent',False))
 source='struct B{int n;int f(){return n;}};struct M:B{};template<int>struct D:M{int g(){return M::f();}};\n'
 source+=''.join(f'int f{i}(){{D<{i}>x;x.n=7;return x.g();}}\n' for i in range(n))+'int main(){return f0()!=7;}'
 corpus.append((f'qualified-path-{n}',source,'equivalent',False))
 source=''.join(f'long f{i}(int j){{long a[2]={{1,2}};return a[j];}}\n' for i in range(n))+'int main(){return f0(1)!=2;}'
 corpus.append((f'wide-array-{n}',source,'equivalent',False))
for name,source in shared.runtimes(factor=12):corpus.append(('runtime-'+name,source,'exact',True))
n=24000000
source='template<class>struct E{};template<class T>struct X{E<T>e;T n;};int step(int v){X<long>a,b;a.n=v;b=a;return b.n;}'
source+=f'int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s=(s+step(i&1023))&65535;return s=={((n//1024)*sum(range(1024))+sum(range(n%1024)))%65536}?0:1;}}'
corpus.append(('runtime-sparse-transfer',source,'equivalent',True))
source='struct B{int n;int f(){return n;}};struct M:B{};template<class T>struct D:M{int g(){return M::f();}};int step(int v){D<int>x;x.n=v;return x.g();}'
source+=f'int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s=(s+step(i&1023))&65535;return s=={((n//1024)*sum(range(1024))+sum(range(n%1024)))%65536}?0:1;}}'
corpus.append(('runtime-qualified-path',source,'equivalent',True))
for count in (2,8,9):
 n=24000000
 source=f'long read(int i){{long a[{count}]={{1,2}};return a[i];}}int main(){{volatile int n={n};long s=0;for(int i=0;i<n;++i)s+=read(i%{count});return s=={n//count*3+(1 if n%count==1 else 3 if n%count>1 else 0)}?0:1;}}'
 corpus.append((f'runtime-wide-array-{count}',source,'equivalent' if count<=8 else 'exact',True))
def native_size(exe,ir):
 # All corpus globals are scalar/array data without symbol aliases. Count only
 # typed data widths; backend alignment remains explicitly in the code metric.
 widths={'ptr':8,'i8':1,'u8':1,'i16':2,'u16':2,'i32':4,'u32':4,'i64':8,'f32':4,'f64':8,'f80':16}
 data_bytes=0;inside=False
 for line in ir.read_text().splitlines():
  if line.startswith('global '):
   inside=line.endswith('{')
   if not inside:
    m=re.search(r': (\w+)',line);data_bytes+=widths[m[1]]
  elif inside:
   if line=='}':inside=False
   else:
    words=line.split();data_bytes+=int(words[1]) if words[0]=='zero' else widths[words[0]]
 payload=shared.text_size(exe)
 return dict(executable_payload_bytes=payload,global_data_bytes=data_bytes,code_and_alignment_bytes=payload-data_bytes,file_bytes=exe.stat().st_size)
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
assert not OUT.exists(), 'Keep completed or interrupted observations; choose a fresh output.'
assert not result['source_diff'], 'Freeze a committed implementation before timing.'
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,comparison,native in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
 commands={};executables={}
 for i,cc in enumerate([A,B]):
  ir=WORK/(name+f'-{i}.lowir');commands[i]=[cc,'--emit-lowir','-O0','-o',ir,src]
  r=shared.run([*commands[i],'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if native:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),**native_size(exe,ir),checked_exit=0)
 if comparison=='exact':
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

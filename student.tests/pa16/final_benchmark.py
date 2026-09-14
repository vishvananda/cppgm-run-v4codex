#!/usr/bin/env python3
"""Final audit frozen compiler/runtime campaign: A B WORK OUT [--continue]."""
from pathlib import Path
import json, os, platform, re, statistics, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A observations then four ABBA blocks; final-only one warmup plus six observations',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 source_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA16/O0 correctness and bounded ownership; no extra percentage/time/size exit gate. Self-hosting and own native backend are later-stage surfaces.',
 native_size_metric='Sectionless ELF payload after entry, with typed global bytes subtracted separately to give code plus alignment.',workloads={})
corpus=[]
for n in [1000,4000]:
 corpus.append((f'template-{n}','template<class T>constexpr int f(T x){return x*3+7;}\n'+''.join(f'static_assert(f({i})=={i*3+7}, "");\n' for i in range(n)),True,False))
 corpus.append((f'memory-float-{n}',''.join(f'double f{i}(double*p,int n){{double s=0;for(int i=0;i<n;++i)s+=p[i]*2;return s;}}\n' for i in range(n)),True,False))
 corpus.append((f'array-sharing-{n}',''.join(f'int f{i}(int i){{constexpr char a[]="012345";constexpr char b[]="012345";return a!=b?a[i]+b[i]:0;}}\n' for i in range(n)),True,False))
 corpus.append((f'final-results-{n}','int made,dead;struct X{X(){++made;}~X(){++dead;}};X make(){return X();}\n'+''.join(f'void f{i}(){{make();}}\n' for i in range(n)),True,False))
 prefix='struct X{unsigned n:3;constexpr X(unsigned v):n(v){}constexpr unsigned f()const{return n;}};constexpr X x(9);constexpr X const*p=&x;template<int N>struct A{char a[N];};\n'
 corpus.append((f'bitfield-queries-{n}',prefix+''.join(f'static_assert(sizeof(A<p->f()+{i}>)=={i+1}, "");\n' for i in range(n)),False,False))
for n in [14,18,22]:
 source='constexpr int n=7;struct L0{int const*p;};constexpr L0 m0(){return L0{&n};}\n'
 source+=''.join(f'struct L{i}{{L{i-1} a,b;}};constexpr L{i} m{i}(){{return L{i}{{m{i-1}(),m{i-1}()}};}}\n' for i in range(1,n+1))
 source+=f'constexpr int consume(L{n} x){{return 7;}}static_assert(consume(m{n}())==7, "");int main(){{return 0;}}'
 corpus.append((f'shared-object-dag-{n}',source,True,False))
for name,source in shared.runtimes(factor=12):corpus.append(('runtime-'+name,source,True,True))
n=24000000
corpus.append(('runtime-results',f'int made,dead;struct X{{X(){{++made;}}~X(){{++dead;}}}};X make(){{return X();}}int main(){{volatile int n={n};for(int i=0;i<n;++i)make();return made=={n}&&dead=={n}?0:1;}}',True,True))
n=8000000
corpus.append(('runtime-copy',f'int read(int i){{int a[40]={{1,2,3}};return a[i];}}int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s+=read(i%40);return s=={n//40*6}?0:1;}}',True,True))
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands):
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 order=[0]*4+[0,1,1,0]*4 if len(commands)==2 else [1]*6
 rows=[dict(binary=i,**observe(commands[i])) for i in order]
 data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  aa=[r['wall_s'] for r in rows[:4]];data['aa_range_s']=[min(aa),max(aa)]
  data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in range(4,len(rows),4)]
 return data
def native_size(exe,ir):
 widths={'ptr':8,'i8':1,'u8':1,'i16':2,'u16':2,'i32':4,'u32':4,'i64':8,'f32':4,'f64':8,'f80':16}
 data_bytes=0;inside=False
 for line in ir.read_text().splitlines():
  if line.startswith('global '):
   inside=line.endswith('{')
   if not inside:data_bytes+=widths[re.search(r': (\w+)',line)[1]]
  elif inside:
   if line=='}':inside=False
   else:
    words=line.split();data_bytes+=int(words[1]) if words[0]=='zero' else widths[words[0]]
 payload=shared.text_size(exe)
 return dict(executable_payload_bytes=payload,global_data_bytes=data_bytes,code_and_alignment_bytes=payload-data_bytes,file_bytes=exe.stat().st_size)
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
if '--continue' in sys.argv:
 previous=json.loads(OUT.read_text());assert previous['binaries']==result['binaries'] and previous['harness_sha256']==result['harness_sha256'];result=previous
else:result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,common,native in corpus:
 if name in result['workloads'] and 'compiler' in result['workloads'][name] and (not native or 'runtime' in result['workloads'][name]):continue
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=shared.sha(src),comparison='exact' if common else 'entry-rejected',outputs=[]);result['workloads'][name]=item
 commands={};executables={}
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');commands[i]=[[A,B][i],'--emit-lowir','-O0','-o',ir,src]
  r=shared.run([*commands[i],'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if native:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),**native_size(exe,ir),checked_exit=0)
 if common:
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 else:
  r=subprocess.run([A,'--emit-lowir','-O0','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True,timeout=30)
  assert r.returncode!=0;item['entry_behavior']=dict(compile_exit=r.returncode,stderr=r.stderr)
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

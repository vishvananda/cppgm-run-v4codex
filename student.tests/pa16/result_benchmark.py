#!/usr/bin/env python3
"""Frozen PA16 result/member-pointer campaign: A B WORK OUT; preserves every observation."""
from pathlib import Path
import json,os,platform,re,statistics,struct,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='frozen A/B and sources; one warmup each; four A/A then two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA16/O0 result lifetime and member-pointer constants. Work follows evaluated expressions/emitted instructions, existing constexpr call/step bounds unchanged. No new optimizer, cloning, or mandatory numeric ceiling. Check new semantic costs and avoidable common regressions.',
 native_size_metric='Sectionless ELF executable payload after entry, including global data; also report payload minus typed LowIR global bytes (code plus alignment padding). This is explicit because these ELF files have no .text section.',
 workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<class T>constexpr int f(T x){return x*3+7;}\n'+''.join(f'static_assert(f({i})=={i*3+7}, "");\n' for i in range(n))
 corpus.append((f'template-{n}',source,'exact',False))
 source=''.join(f'double f{i}(double*p,int n){{double s=0;for(int i=0;i<n;++i)s+=p[i]*2;return s;}}\n' for i in range(n))
 corpus.append((f'memory-float-{n}',source,'exact',False))
 source='int made,dead;struct X{X(){++made;}~X(){++dead;}};X make(){return X();}\n'+''.join(f'void f{i}(){{make();}}\n' for i in range(n))
 corpus.append((f'final-results-{n}',source,'equivalent',False))
 source='struct X{int a,b;constexpr int f(int n)const{return b+n;}};constexpr X x={2,7};constexpr int X::*p[40]={&X::a,&X::b};constexpr int(X::*q)(int)const=&X::f;\n'+''.join(f'static_assert(x.*p[1]+(x.*q)({i})=={i+14}, "");\n' for i in range(n))
 corpus.append((f'member-constants-{n}',source,'entry-incorrect',False))
for name,source in shared.runtimes(factor=12):corpus.append(('runtime-'+name,source,'exact',True))
n=4000000
source=f'int made,dead;struct X{{X(){{++made;}}~X(){{++dead;}}}};X make(){{return X();}}int main(){{volatile int n={n};for(int i=0;i<n;++i)make();return made=={n}&&dead=={n}?0:1;}}'
corpus.append(('runtime-final-results',source,'equivalent',True))
n=24000000
source=f'struct X{{int a,b;constexpr int f(int k)const{{return b+k;}}}};int call(int i){{int X::*p[40]={{&X::a,&X::b}};int(X::*q[2])(int)const={{&X::f}};X x={{2,7}};return x.*p[i%2]+(x.*q[0])(i%2);}}int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s+=call(i);return s=={n*12}?0:1;}}'
corpus.append(('runtime-members',source,'equivalent',True))
# Retain the short result loop, then add a longer execution and a focused
# noise repeat for the 4000-function compiler outlier; never replace samples.
source='int made,dead;struct X{X(){++made;}~X(){++dead;}};X make(){return X();}\n'+''.join(f'void f{i}(){{make();}}\n' for i in range(4000))
corpus.append(('final-results-4000-repeat',source,'equivalent',False))
n=24000000
source=f'int made,dead;struct X{{X(){{++made;}}~X(){{++dead;}}}};X make(){{return X();}}int main(){{volatile int n={n};for(int i=0;i<n;++i)make();return made=={n}&&dead=={n}?0:1;}}'
corpus.append(('runtime-final-results-long',source,'equivalent',True))
for n in [1000,4000]:
 source='struct X{constexpr int f()const{return 7;}};template<int N>struct P{static constexpr int(X::*p[2])()const={&X::f};};template<int N>constexpr int(X::*P<N>::p[2])()const;constexpr X x={};\n'+''.join(f'int call{i}(){{return (x.*P<{i}>::p[0])();}}\n' for i in range(n))
 corpus.append((f'template-member-storage-{n}',source,'entry-incorrect',False))
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands,blocks=2):
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in ([0]*4+[0,1,1,0]*blocks if len(commands)==2 else [1]*6)]
 data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  data['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in range(4,len(rows),4)]
 return data
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
if '--continue' in sys.argv:
 previous=json.loads(OUT.read_text())
 assert previous['binaries']==result['binaries']
 previous['continuation_harness_sha256']=shared.sha(__file__)
 result=previous
else:result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,comparison,native in corpus:
 if name in result['workloads'] and 'compiler' in result['workloads'][name] and (not native or 'runtime' in result['workloads'][name]):continue
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_path=str(src),source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
 common=comparison!='entry-incorrect';commands={};executables={}
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');command=[[A,B][i],'--emit-lowir','-O0','-o',ir,src];commands[i]=command
  r=shared.run([*command,'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if native:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),**native_size(exe,ir),checked_exit=0)
 if comparison=='exact':
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 elif comparison=='equivalent':
  assert name.startswith(('final-results','runtime-final-results','runtime-members'))
 if comparison=='entry-incorrect':
  ir=WORK/(name+'-entry.lowir')
  r=subprocess.run([A,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
  assert r.returncode!=0
  item['entry_behavior']=dict(compile_exit=r.returncode,diagnostic=r.stderr)
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands,4 if name.endswith('-repeat') else 2)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

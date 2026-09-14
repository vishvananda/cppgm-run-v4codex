#!/usr/bin/env python3
"""Frozen PA16 storage campaign: A B WORK OUT; preserves every observation."""
from pathlib import Path
import json,os,platform,re,statistics,struct,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='frozen A/B and sources; one warmup each; four A/A then two ABBA blocks; B-only six samples for previously incorrect storage',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA16/O0 semantic storage and mandated constexpr-array copies/interner; no optional transform or mandated numeric ceiling. Diagnostic investigations: common compiler latency +15%, RSS +16MiB, 4x source work <=5.5x. Resolve avoidable costs; compare correct output only.',
 native_size_metric='Sectionless ELF executable payload after entry, including global data; also report payload minus typed LowIR global bytes (code plus alignment padding). This is explicit because these ELF files have no .text section.',
 workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<class T>constexpr int f(T x){return x*3+7;}\n'+''.join(f'static_assert(f({i})=={i*3+7}, "");\n' for i in range(n))
 corpus.append((f'template-{n}',source,'exact',False))
 source=''.join(f'double f{i}(double*p,int n){{double s=0;for(int i=0;i<n;++i)s+=p[i]*2;return s;}}\n' for i in range(n))
 corpus.append((f'memory-float-{n}',source,'exact',False))
 source=''.join(f'int f{i}(int i){{constexpr char a[]="012345";constexpr char b[]="012345";return a!=b?a[i]+b[i]:0;}}\n' for i in range(n))
 corpus.append((f'array-sharing-{n}',source,'equivalent',False))
 source=''.join(f'int f{i}(int i){{constexpr int a[40]={{1,2,3}};return a[i];}}\n' for i in range(n))
 corpus.append((f'array-copy-{n}',source,'equivalent',False))
 source='int get(int n){return n;}\n'+''.join(f'int f{i}(int n){{static int a=get(n);return ++a;}}\n' for i in range(n))
 corpus.append((f'static-{n}',source,'entry-incorrect',False))
for name,source in shared.runtimes(factor=12):corpus.append(('runtime-'+name,source,'exact',True))
n=24000000
corpus.append(('runtime-sharing',f'int read(int i){{constexpr char a[]="012345";constexpr char b[]="012345";return a!=b?a[i]-b[i]:1;}}int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s+=read(i%7);return s;}}','equivalent',True))
n=8000000
corpus.append(('runtime-copy',f'int read(int i){{constexpr int a[40]={{1,2,3}};return a[i];}}int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s+=read(i%40);return s=={n//40*6}?0:1;}}','equivalent',True))
n=24000000
corpus.append(('runtime-static',f'int calls;int get(int n){{++calls;return n;}}int read(int n){{static int a=get(n);return a;}}int main(){{volatile int n={n};int s=0;for(int i=0;i<n;++i)s|=read(i);return s || calls!=1;}}','entry-incorrect',True))
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
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,comparison,native in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
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
  # Compiler-only arrays differ only in initial stores/copies and deduplicated
  # data. The fixed native sharing/copy workloads exercise those same forms.
  assert name.startswith(('array-sharing','array-copy','runtime-sharing','runtime-copy'))
 if comparison=='entry-incorrect' and native:
  ir=WORK/(name+'-entry.lowir');exe=WORK/(name+'-entry');shared.run([A,'--emit-lowir','-O0','-o',ir,src]);shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
  r=subprocess.run([exe],capture_output=True,text=True,timeout=120);assert r.returncode!=0
  item['entry_behavior']=dict(exit_code=r.returncode,ir_sha256=shared.sha(ir),native_sha256=shared.sha(exe))
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

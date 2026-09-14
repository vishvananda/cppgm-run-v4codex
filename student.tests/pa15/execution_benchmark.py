#!/usr/bin/env python3
"""Frozen PA15 execution/body/storage evidence. Run explicitly with A B WORK OUT."""
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
 acceptance='PA15/O0 required semantic work, no optional optimization or mandated numeric performance ceiling; constant execution limited to 512 active calls and 1000000 expression steps per root; unavailable results not memoized as semantic failure',
 text_metric='compiler .text; sectionless native payload after ELF entry; native workloads have no static data',workloads={})
corpus=[]
for n in [1000,4000]:
 source=''.join(f'struct A{i}{{int f(int n){{return n+1;}}}};int run{i}(int n){{A{i} a;return a.f(n);}}\n' for i in range(n))
 corpus.append((f'ordinary-{n}',source,'exact',False))
 source=''.join(f'struct A{i}{{int f(int n){{static_assert(sizeof(int)==4,"");return n+1;}}}};\n' for i in range(n))
 corpus.append((f'dormant-{n}',source,'exact',False))
 source='template<int N>int f(int x){return N+x;}\n'+''.join(f'int run{i}(int n){{return f<{i}>(n);}}\n' for i in range(n))
 corpus.append((f'template-{n}',source,'exact',False))
 source='constexpr int adjust(int n){return n*3+7;}\n'+''.join(f'static_assert(adjust({i})=={i*3+7},"");\nstatic_assert(adjust({i})=={i*3+7},"");\n' for i in range(n))
 corpus.append((f'calls-{n}',source,'entry-rejected',False))
 source='template<int N>struct K{constexpr operator int()const{return N;}};template<int N>struct C{static const int n=N;};\n'
 source+=''.join(f'static_assert(C<K<{i}>{{}}>::n=={i},"");\n' for i in range(n))
 corpus.append((f'receivers-{n}',source,'entry-rejected',False))
 source='template<int N>struct K{static const int n=N;int unused(){return missing(N);}};template<int N>const int K<N>::n;\n'
 source+=''.join(f'K<{i}> k{i};\n' for i in range(n))
 corpus.append((f'storage-{n}',source,'entry-missing-storage',False))
for name,source in shared.runtimes(factor=10):corpus.append(('runtime-'+name,source,'exact',True))
n=60000000;expected=sum((i+23)&1023 for i in range(n%1024))+n//1024*sum(range(1024));expected&=65535
source=f'''constexpr int adjust(int x){{return x+7;}}
int step(int x){{constexpr int bias=adjust(16);return (x+bias)&1023;}}int main(){{volatile int n={n};int sum=0;
for(int i=0;i<n;++i)sum=(sum+step(i))&65535;return sum=={expected}?0:1;}}'''
corpus.append(('runtime-constant',source,'entry-rejected',True))
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
  assert (r.returncode!=0)==(comparison=='entry-rejected'),(name,r.stderr)
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
 if comparison=='entry-missing-storage':
  count=int(name.split('-')[1]);assert sum(s.startswith('global ') for s in ir.read_text().splitlines())==2*count
  assert sum(s.startswith('global ') for s in Path(item['entry_probe']['path']).read_text().splitlines())==count
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(executables)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

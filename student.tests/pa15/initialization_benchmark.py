#!/usr/bin/env python3
"""Frozen compiler and native evidence for PA15 initialization ownership."""
from pathlib import Path
import json,os,platform,statistics,struct,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='frozen binaries/flags/inputs; native checked results or exact LowIR equivalence; one warmup each; four A/A then two ABBA blocks; new-correct-only six B samples',cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',implementation_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),acceptance='PA15/O0 correctness and bounded work/lifetimes; no mandated numeric latency/RSS/text ceilings; constant backing is required output policy, no optional optimizer',text_metric='compiler .text; sectionless native payload from entry to first trailing data byte, including alignment padding; trailing array data separately verified byte for byte',workloads={})
corpus=[]
for n in [1000,4000]:
 source='template<int N>int f(int k){constexpr int a[4]={N,N+1,N+2,N+3};return a[k];}\n'
 source+=''.join(f'int run{i}(int n){{return f<{i}>(n);}}\n' for i in range(n))
 reps=60000000//n;expected=((n*(n-1)//2+3*n)*reps)&0xffffffff
 source+=f'int main(){{unsigned sum=0;volatile int repeat={reps};for(int k=0;k<repeat;++k){{'+''.join(f'sum+=run{i}(3);' for i in range(n))+f'}}return sum=={expected}U?0:1;}}'
 data=b''.join(struct.pack('<4i',i,i+1,i+2,i+3) for i in range(n))
 corpus.append((f'arrays-{n}',source,'native-equivalent',data))
 source='struct Base{int x;};template<int N>struct D:Base{int y;};\n'+''.join(f'int run{i}(){{D<{i}> d{{}};return d.x+d.y;}}\n' for i in range(n))
 corpus.append((f'derived-{n}',source,'entry-missing-base-zero',None))
 source='template<int N>struct C{static int n;int f(){return ++n;}};template<int N>int C<N>::n=N;\n'+''.join(f'int run{i}(){{C<{i}> c;return c.f();}}\n' for i in range(n))
 corpus.append((f'updates-{n}',source,'entry-rejected',None))
 source='template<class Tag,class T>int f('+','.join(f'T a{j}' for j in range(16))+'){return a0+a15;}\n'
 source+=''.join(f'struct Tag{i}{{}};int run{i}(int n){{return f<Tag{i}>(n,n,n,n,n,n,n,n,n,n,n,n,n,n,n,n);}}\n' for i in range(n))
 corpus.append((f'wide-signatures-{n}',source,'exact',None))
for name,source in shared.runtimes(factor=10):corpus.append(('runtime-'+name,source,'exact',b''))
for n in [4,64]:
 values=[3+17*i for i in range(n)];iterations=30000000
 expected=((iterations//n)*sum(values)+sum(values[:iterations%n]))&65535
 source='int fetch(int k){constexpr int a['+str(n)+']={'+','.join(map(str,values))+'};return a[k];}\n'
 source+=f'int main(){{volatile int n={iterations};int sum=0;for(int i=0;i<n;++i)sum=(sum+fetch(i&{n-1}))&65535;return sum=={expected}?0:1;}}'
 corpus.append((f'runtime-array-{n}',source,'native-equivalent',struct.pack('<'+'i'*n,*values)))
def native_size(path,trailing):
 data=path.read_bytes();entry,phoff=struct.unpack_from('<QQ',data,24)
 kind,flags,offset,address,_,filesz,memsz,align=struct.unpack_from('<IIQQQQQQ',data,phoff)
 assert kind==1 and flags&1 and offset==0 and filesz==len(data)
 if trailing:assert data.endswith(trailing),(path,'native data tail mismatch')
 return dict(text_bytes=filesz-len(trailing)-(entry-address),data_bytes=len(trailing),file_bytes=len(data))
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
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,comparison,data in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),comparison=comparison,outputs=[]);result['workloads'][name]=item
 common=comparison in ['exact','native-equivalent'];commands={};native={}
 if not common:
  r=subprocess.run([A,'--emit-lowir','-O0','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True)
  item['entry_probe']=dict(exit=r.returncode,stderr=r.stderr)
  assert (r.returncode!=0)==(comparison=='entry-rejected')
 for i in ([0,1] if common else [1]):
  ir=WORK/(name+f'-{i}.lowir');command=[[A,B][i],'--emit-lowir','-O0','-o',ir,src];commands[i]=command
  r=shared.run([*command,'--stats','--validate-lowir']);out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if data is not None:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);native[i]=[exe]
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),checked_exit=0,**native_size(exe,data if i else b''))
 if comparison=='exact':
  assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256'],name
  if native:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if native:item['runtime']=measure(native)
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());OUT.write_text(json.dumps(result,indent=2)+'\n')

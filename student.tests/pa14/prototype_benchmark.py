#!/usr/bin/env python3
"""Necessary prototype-scope work; the frozen entry rejects these valid inputs."""
from pathlib import Path
import json,os,platform,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one B warmup and six B-only observations; A rejection retained, no optimization profit comparison',cpu=cpu,platform=platform.platform(),
 flags=['--emit-lowir','-O0'],backend_flags=['-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p.resolve()),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
 acceptance='Required C++11 prototype scope; no optional transform or invented numeric gate; runtime and code size are semantic baselines',workloads={})
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,inv,vol=usage.read_text().split()
 return dict(binary=1,wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(inv),voluntary=int(vol),checked_exit=0)
def campaign(command):return dict(warmups=[observe(command)],observations=[observe(command) for _ in range(6)])
pattern='template<class T>struct Box{int n;static int f(int a,decltype(a) b);int f(long,long);};template<class U>int Box<U>::f(int a,decltype(a) b){return a+b;}template<class U>int Box<U>::f(long a,long b){return n+a+b;}'
corpus=[]
for n in (1000,4000):
 uses=''.join(f'struct Tag{i}{{}};int run{i}(Box<Tag{i}>& b){{return b.f(2L,3L)+Box<Tag{i}>::f(2,3);}}\n' for i in range(n))
 corpus.append((f'prototype-instances-{n}',pattern+uses,False))
count=3000000;expected=(count*15)&65535
corpus.append(('prototype-runtime',pattern+f'int main(){{volatile int count={count};Box<int> b;b.n=4;int sum=0;for(int i=0;i<count;++i)sum=(sum+b.f(2L,(long)(i&7))+Box<int>::f(2,i&7))&65535;return sum!={expected};}}',True))
for name,source,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);ir=WORK/(name+'.lowir');command=[B,'--emit-lowir','-O0','-o',ir,src]
 rejected=subprocess.run([str(A),'--emit-lowir','-O0','-o',str(WORK/'entry.lowir'),str(src)],capture_output=True,text=True);assert rejected.returncode==1
 log=WORK/(name+'-entry.log');log.write_text(rejected.stderr)
 stats=shared.run([*command,'--stats','--validate-lowir']);out=dict(binary=1,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
 if executable:
  exe=WORK/name;shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode='--emit-lowir',outputs=[out],exact_required=False,equivalence='correct B only; A rejects prior parameter names in concrete signatures',entry_rejection=dict(exit_code=1,path=str(log),sha256=shared.sha(log)),compiler=campaign(command))
 if executable:item['runtime']=campaign([exe])
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

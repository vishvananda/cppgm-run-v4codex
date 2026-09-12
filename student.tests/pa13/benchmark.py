#!/usr/bin/env python3
"""Frozen PA13 O0 evidence; A/A, ABBA and absolute-only new virtual behavior."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]); binaries=[A.resolve(),B.resolve()]
WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='warmup; A/A four observations then two ABBA blocks; B-only new semantics six observations',
 cpu=cpu,platform=platform.platform(),compiler_flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 build_flags='g++ -std=gnu++11 -Wall -O3; course TEST_RUNNER_ENABLE',
 commits=['823e929cdc74fedbb487973dcad33b8a4dce18f4',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(b),sha256=shared.sha(b),text_bytes=shared.text_size(b)) for b in binaries],
 text_metric='compiler .text; sectionless executable payload after entry, including support and data',
 acceptance='O0 bounded necessary semantics; no optional transform or speed claim; no self-selected numeric exit gate',workloads={})
def observe(cmd):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M','-o',usage,*cmd])
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()),checked_exit=0)
def campaign(cmds):
 warmups=[dict(binary=b,**observe(c)) for b,c in cmds.items()]
 rows=[dict(binary=b,**observe(cmds[b])) for b in (shared.ORDER if len(cmds)==2 else [1]*6)]
 out=dict(warmups=warmups,observations=rows)
 if len(cmds)==2:
  out['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  out['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
 return out
corpus=[(name,src,mode,True,False) for name,src,mode,scale in shared.workloads()]
classbody='struct S{int a;S(int n):a(n){} int f(int n)const{return a+n;} }; int run(int n){S s(n);return s.f(n);}'
virtualbody='struct A{virtual int f(int n)const noexcept{return n;} virtual ~A()noexcept{}};struct B:A{int x;B(int n):x(n){}int f(int n)const noexcept override{return x+n;}};int run(int n){B b(n);A& a=b;return a.f(n);}'
for label,body,common in [('class',classbody,True),('virtual',virtualbody,False)]:
 for n in (1000,4000): corpus.append((label+'-'+str(n),''.join('namespace N'+str(i)+'{'+body+'}\n' for i in range(n)),'--emit-lowir',common,False))
 n=12000000;expected=((n//1024)*sum(2*i for i in range(1024))+sum(2*i for i in range(n%1024)))&65535
 source=body+f'int main(){{volatile int n={n};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
 corpus.append((label+'-runtime',source,'--emit-lowir',common,True))
for name,src in shared.runtimes(16):corpus.append((name+'-runtime',src,'--emit-lowir',True,True))
corpus.insert(0,('startup','int main(){return 0;}','--emit-lowir',True,True))
for name,source,mode,common,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);labels=(0,1) if common else (1,)
 commands={};runtimes={};outputs=[]
 for b in labels:
  ir=WORK/(name+f'-{b}.out');cmd=[binaries[b],mode,*(['-O0'] if mode=='--emit-lowir' else []),'-o',ir,src]
  stats=shared.run([*cmd,'--stats',*(['--validate-lowir'] if mode=='--emit-lowir' else [])])
  out=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(l) for l in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  outputs.append(out);commands[b]=cmd
 if common:
  assert outputs[0]['sha256']==outputs[1]['sha256'],(name,'common output changed')
  if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 entry=dict(source_path=str(src),source_sha256=shared.sha(src),mode=mode,outputs=outputs,
  equivalence='byte-identical correct A/B output' if common else 'correct B only; A lacks virtual semantics',compiler=campaign(commands))
 if runtimes:entry['runtime']=campaign(runtimes)
 result['workloads'][name]=entry;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
assert [shared.sha(b) for b in binaries]==[b['sha256'] for b in result['binaries']]

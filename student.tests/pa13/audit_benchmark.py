#!/usr/bin/env python3
"""Frozen PA13 final audit: unchanged programs A/B, repaired behavior B-only."""
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
 commits=['cf1b9621',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
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
# Reuse every fixed stage workload; A already implements these correctly.
prior=json.loads((ROOT/'student.tests/pa13/performance.json').read_text())
corpus=[(name,Path(w['source_path']).read_text(),w['mode'],True,'runtime' in w)
        for name,w in prior['workloads'].items()]
result['prior_report_sha256']=shared.sha(ROOT/'student.tests/pa13/performance.json')
for name,w in prior['workloads'].items():
 assert shared.sha(w['source_path'])==w['source_sha256'],name
bodies={
 'array': 'struct B{B(){}virtual int f(){return 7;}virtual ~B(){}};int run(int n){int count=(n&3)+1;B*p=new B[count];int sum=0;for(int i=0;i<count;++i)sum+=p[i].f();delete[]p;if(sum!=7*count)__builtin_abort();return n*2+1;}',
 'destructor': 'int trace;struct B{virtual ~B(){++trace;}};struct D:B{~D()override{trace+=2;}};int run(int n){trace=0;B*p=new D;p->~B();::operator delete(p);if(trace!=3)__builtin_abort();return n*2+1;}',
 'global-delete': 'int trace,local;struct B{virtual ~B(){++trace;}};struct D:B{~D()override{trace+=2;}static void operator delete(void*p)noexcept{++local;::operator delete(p);}};int run(int n){trace=local=0;B*p=new D;::delete p;if(trace!=3||local)__builtin_abort();return n*2+1;}',
 'conversion': 'struct B{virtual operator int()const{return 1;}virtual operator long()const{return 2;}};struct D:B{int x;D(int n):x(n){}operator int()const override{return x*2+1;}operator long()const override{return x;}};int run(int n){D d(n);B&b=d;int i=b;long l=b;if(l!=n)__builtin_abort();return i;}',
 'operator': 'struct B{int x;virtual B&operator=(int n){x=n;return *this;}virtual int operator()()const{return 0;}};struct D:B{D&operator=(int n)override{x=n*2+1;return *this;}int operator()()const override{return x;}};int run(int n){D d;B&b=d;b=n;return b();}'
}
prefix='void operator delete(void*)noexcept;'
for name,body in bodies.items():
 for count in (400,1600):
  corpus.append((name+'-'+str(count),prefix+''.join('namespace N'+str(i)+'{'+body+'}' for i in range(count)),'--emit-lowir',False,False))
 count=80000 if name in ('array','destructor','global-delete') else 12000000
 expected=((count//1024)*sum(2*i+1 for i in range(1024))+sum(2*i+1 for i in range(count%1024)))&65535
 source=prefix+body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
 corpus.append((name+'-runtime',source,'--emit-lowir',False,True))
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
  equivalence='byte-identical correct A/B output' if common else 'correct B only; reduced audit controls prove A incorrect',compiler=campaign(commands))
 if runtimes:entry['runtime']=campaign(runtimes)
 result['workloads'][name]=entry;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
assert [shared.sha(b) for b in binaries]==[b['sha256'] for b in result['binaries']]

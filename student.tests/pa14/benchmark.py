#!/usr/bin/env python3
"""Frozen O0 PA14 evidence: common-correct A/A+ABBA, new semantics B-only."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A, B, WORK, OUT = map(Path,sys.argv[1:5])
binaries = [A.resolve(),B.resolve()]
WORK.mkdir(parents=True,exist_ok=True)
cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
result = dict(protocol='one warmup per binary; four A/A observations; two ABBA blocks; new semantics six B-only observations',
 cpu=cpu, platform=platform.platform(), host=shared.run(['g++','--version']).stdout.splitlines()[0],
 flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 commits=['8af3c149454e4e43e441206e6978f4d1300e079b',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF payload after entry (includes support/data)',
 acceptance='O0 semantic implementation, no optional optimization, no self-imposed numeric exit gate; disclose and investigate common-correct regressions',
 workloads={})
def observe(command):
 usage=WORK/'usage.txt'; started=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-started)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(c)) for b,c in commands.items()]
 rows=[dict(binary=b,**observe(commands[b])) for b in (shared.ORDER if len(commands)==2 else [1]*6)]
 out=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  out['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
  out['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
 return out
corpus=[(name,src,mode,True,False) for name,src,mode,scale in shared.workloads()]
for n in (1000,4000):
 source='template<class T>T identity(T n){return n;}\n'+''.join(f'int f{i}(int n){{return identity(n);}}\n' for i in range(n))
 corpus.append((f'template-repeat-{n}',source,'--emit-lowir',False,False))
for n in (250,1000):
 source='template<class T>struct Box{T data;int get(){return data.value;}};\n'
 source+=''.join(f'namespace N{i}{{struct Object{{int value;}};int run(int n){{Box<Object> b;b.data.value=n;return b.get();}}}}\n' for i in range(n))
 corpus.append((f'class-instances-{n}',source,'--emit-lowir',False,False))
for name,src in shared.runtimes(16): corpus.append((name+'-runtime',src,'--emit-lowir',True,True))
n=16000000
expected=((n//1024)*sum(range(1024))+sum(range(n%1024)))&65535
source='template<class T>struct Box{T value;void add(T n){value=(value+n)&65535;}};template<class T>void step(Box<T>& b,T n){b.add(n);}'
source+=f'int main(){{Box<int> b;b.value=0;volatile int n={n};for(int i=0;i<n;++i)step(b,i&1023);return b.value!={expected};}}'
corpus.append(('template-runtime',source,'--emit-lowir',False,True))
for name,source,mode,common,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source); outputs=[];commands={};runtimes={}
 for b in ((0,1) if common else (1,)):
  ir=WORK/(name+f'-{b}.out'); command=[binaries[b],mode,*(['-O0'] if mode=='--emit-lowir' else []),'-o',ir,src]
  stats=shared.run([*command,'--stats',*(['--validate-lowir'] if mode=='--emit-lowir' else [])])
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  outputs.append(item);commands[b]=command
 if common:
  assert outputs[0]['sha256']==outputs[1]['sha256'],(name,'common correct output changed')
  if executable: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 if name.startswith('template-repeat-'):
  assert outputs[0]['telemetry'][0]['template_body_transitions']==1
 if name.startswith('class-instances-'):
  assert outputs[0]['telemetry'][0]['template_class_completions']==int(name.rsplit('-',1)[1])
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode=mode,outputs=outputs,
  equivalence='byte-identical correct A/B output' if common else 'correct B only; A lacks required template bodies/completion',compiler=campaign(commands))
 if runtimes:item['runtime']=campaign(runtimes)
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
assert [shared.sha(p) for p in binaries]==[b['sha256'] for b in result['binaries']]

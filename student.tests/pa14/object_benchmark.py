#!/usr/bin/env python3
"""PA14 shared object receiver facts: common-correct A/A+ABBA compiler/native evidence."""
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
 commits=['d6891c36',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
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
# Frozen common compiler and runtime inputs from the preceding campaign.
prior=json.loads((ROOT/'student.tests/pa14/call-performance.json').read_text())
corpus=[]
for name in prior['workloads']:
 w=prior['workloads'][name]
 assert shared.sha(w['source_path'])==w['source_sha256']
 corpus.append((name,Path(w['source_path']).read_text(),w['mode'],True,'runtime' in w,True))
objects='struct Fixed{int n;Fixed(int n):n(n){}virtual int get(int x)const{return n+x;}int grow(int x){n+=x;return n;}};'
body='v.n+=n;int x=v.grow(n);return x+v.get(n);'
pattern='template<class T>int compute(Fixed& v,int n){'+body+'}\n'
values='int live=0;struct Value{int n;Value(int n):n(n){++live;}Value(const Value& v):n(v.n){++live;}~Value(){--live;}int get()const{return n;}};struct Factory{int n;Factory(int n):n(n){}Value value(int x)const{return Value(n+x);}};'
valuepattern='template<class T>int compute(const Factory& v,int n){return v.value(n).get();}\n'
for n in (1000,4000):
 uses=''.join(f'struct Tag{i}{{}};int run{i}(Fixed& v,int n){{return compute<Tag{i}>(v,n);}}\n' for i in range(n))
 corpus.append((f'object-instances-{n}',objects+pattern+uses,'--emit-lowir',True,False,True))
 corpus.append((f'object-unused-{n}',objects+''.join(f'template<class T>int unused{i}(Fixed& v,int n){{'+body+'}\n' for i in range(n)),'--emit-lowir',True,False,True))
 uses=''.join(f'struct Tag{i}{{}};int run{i}(const Factory& v,int n){{return compute<Tag{i}>(v,n);}}\n' for i in range(n))
 corpus.append((f'object-results-{n}',values+valuepattern+uses,'--emit-lowir',True,False,True))
count=3000000;value=1;expected=0
for i in range(count):
 value+=2*(i&127);expected=(expected+2*value+(i&127)+1)&65535
source=objects+'struct Derived:Fixed{Derived(int n):Fixed(n){}int get(int x)const override{return n+x+1;}};'+pattern+f'int main(){{volatile int n={count};Derived value(1);int sum=0;for(int i=0;i<n;++i)sum=(sum+compute<int>(value,i&127))&65535;return sum!={expected};}}'
corpus.append(('object-runtime',source,'--emit-lowir',True,True,True))
count=6000000;expected=((count//1024)*sum(n+7 for n in range(1024))+sum(n+7 for n in range(count%1024)))&65535
source=values+valuepattern+f'int main(){{volatile int n={count};Factory value(7);int sum=0;for(int i=0;i<n;++i)sum=(sum+compute<int>(value,i&1023))&65535;return sum!={expected}||live;}}'
corpus.append(('object-results-runtime',source,'--emit-lowir',True,True,True))
for name,source,mode,common,executable,exact_required in corpus:
 src=WORK/(name+'.cpp');src.write_text(source); outputs=[];commands={};runtimes={}
 for b in ((0,1) if common else (1,)):
  ir=WORK/(name+f'-{b}.out'); command=[binaries[b],mode,*(['-O0'] if mode=='--emit-lowir' else []),'-o',ir,src]
  stats=shared.run([*command,'--stats',*(['--validate-lowir'] if mode=='--emit-lowir' else [])])
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  outputs.append(item);commands[b]=command
 if common and exact_required:
  assert outputs[0]['sha256']==outputs[1]['sha256'],(name,'common correct output changed')
  if executable: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 proof=('byte-identical correct A/B output' if exact_required else
        'both execute the same checked sum; reference-copy representations preserve the same bound address' if common else
        'correct B only; A performs an extra move and fails the required move count')
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode=mode,outputs=outputs,
  exact_required=exact_required,equivalence=proof,compiler=campaign(commands))
 if not executable and not exact_required:
  item['equivalence']='same constructor/binding operations repeated across independent types; corresponding reference-move-runtime checks both native outputs'
 if runtimes:item['runtime']=campaign(runtimes)
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
assert [shared.sha(p) for p in binaries]==[b['sha256'] for b in result['binaries']]

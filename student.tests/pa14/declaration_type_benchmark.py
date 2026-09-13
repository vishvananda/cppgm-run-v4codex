#!/usr/bin/env python3
"""Frozen type-substitution compiler/native comparison with AA and ABBA blocks."""
from pathlib import Path
import json, os, platform, statistics, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]); WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
cpu=min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
result=dict(protocol='freeze all inputs/outputs before timing; one warmup per binary; four A/A observations; two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['5b1afe54263e19367359803bd1d2fd4415450523',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF payload after entry',
 acceptance='O0 required type/context ownership; zero generated-code growth for common-correct reuse; no optional optimizer or mandated numeric compiler gate',workloads={})
def observe(command):
 usage=WORK/'usage.txt'; start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in (0,1)]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,
  aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
corpus=[]
pattern='template<class T>int f(T* input){using P=T*;using C=const T*;P a=input;C b=a;using R=decltype((a));R r=a;'
pattern+=''.join(f'P p{i}=r;C c{i}=p{i};' for i in range(12))
pattern+='return a==b;}\n'
outside='template<class T>struct C{using V=T;V value;V get()const;static V echo(V);};template<class U>typename C<U>::V C<U>::get()const{V copy=value;using R=decltype((copy));R r=copy;return r;}template<class W>typename C<W>::V C<W>::echo(W v){const V copy=v;return copy;}\n'
for n in (1000,4000):
 tags=''.join(f'struct Tag{i}{{}};int run{i}(Tag{i}* p){{return f(p);}}\n' for i in range(n))
 corpus.append((f'declaration-instances-{n}',pattern+tags,False))
 classes=''.join(f'struct Tag{i}{{int n;}};int run{i}(C<Tag{i}>& c){{return c.get().n+C<Tag{i}>::echo(c.value).n;}}\n' for i in range(n))
 corpus.append((f'declaration-outside-{n}',outside+classes,False))
 unused=''.join(pattern.replace('int f(',f'int f{i}(') for i in range(n))
 corpus.append((f'declaration-unused-{n}',unused,False))
count=3000000
corpus.append(('declaration-runtime',pattern+outside+f'int main(){{volatile int count={count};int sum=0;C<int> c;c.value=7;for(int i=0;i<count;++i)sum=(sum+f(&c)+c.get()+C<int>::echo(3))&65535;return sum!={(11*count)&65535};}}',True))
# Preserve inherited frontend/native coverage and the unresolved ordinary-call
# memory observation. Inputs and outputs are checked against frozen evidence.
parent=ROOT/'student.tests/pa14/dependent-object-final-performance.json'
old=json.loads(parent.read_text()); result.update(parent_path=str(parent),parent_sha256=shared.sha(parent))
for name in ('member-instances-1000','member-repeated-1000','member-runtime'):
 item=old['workloads'][name]; assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item))
old=json.loads((ROOT/'student.tests/pa14/object-performance.json').read_text())
for name in ('calls-1','calls-4','memory-float-1','calls-runtime','memory-runtime','floating-runtime'):
 item=old['workloads'][name]; assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item))
commands={}; runtimes={}
for name,source,executable in corpus:
 src=WORK/(name+'.cpp'); src.write_text(source); outputs=[]; commands[name]={}; runtimes[name]={}
 for b in (0,1):
  out=WORK/(name+f'-{b}.lowir'); command=[binaries[b],'--emit-lowir','-O0','-o',out,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,path=str(out),sha256=shared.sha(out),bytes=out.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}'); shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]); shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0); runtimes[name][b]=[exe]
  commands[name][b]=command; outputs.append(item)
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 if executable: assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),mode='--emit-lowir',outputs=outputs,exact_required=True,equivalence='byte-identical correct compiler/native output')
 print(name,'preflight',flush=True)
OUT.write_text(json.dumps(result,indent=2)+'\n')
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]: item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n'); print(name,'measured',flush=True)
for binary in result['binaries']: assert shared.sha(binary['path'])==binary['sha256']

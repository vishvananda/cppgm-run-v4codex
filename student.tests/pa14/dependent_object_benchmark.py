#!/usr/bin/env python3
"""Frozen PA14 template-owned field facts: compiler AA/ABBA and native evidence."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup per binary; four A/A observations; two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['33b791da4e1df961dd47d46aaf0c0eb842a767be',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF payload after entry',
 acceptance='O0 required semantic work; zero generated-code growth on common-correct inputs; no optional optimizer or invented numeric compiler gate',workloads={})
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in (0,1)]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
body='n+=k;n^=3;n+=this->n;return (*this).n+k;'
pattern='template<class T>struct Box{T padding;int n;int run(int k){'+body+'}int get()const{return n+1;}};'
outside='template<class T>struct Box{T padding;int n;int run(int);int get()const;};template<class U>int Box<U>::run(int k){'+body+'}template<class U>int Box<U>::get()const{return n+1;}'
corpus=[]
for count in (1000,4000):
 uses=''.join(f'struct Tag{i}{{char pad[{i%8+1}];}};int run{i}(Box<Tag{i}>& b,int k){{return b.run(k)+b.get();}}\n' for i in range(count))
 corpus.append((f'member-instances-{count}',pattern+uses,False))
 corpus.append((f'member-outside-{count}',outside+uses,False))
 corpus.append((f'member-unused-{count}',''.join(f'template<class T>struct Unused{i}{{int n;int run(int k){{'+body+'}};\n' for i in range(count)),False))
 repeat='template<class T>struct Box{T padding;int n;int run(int k){'+('n+=k;'*count)+'return n;}};'
 uses=''.join(f'struct Tag{i}{{char pad[{i+1}];}};int run{i}(Box<Tag{i}>& b,int k){{return b.run(k);}}\n' for i in range(4))
 corpus.append((f'member-repeated-{count}',repeat+uses,False))
count=3000000;expected=0
for i in range(count):
 n=i&255;k=i&7;n+=k;n^=3;n+=n
 expected=(expected+n+k+n+1)&65535
runtime=f'int main(){{volatile int count={count};Box<int> b;int sum=0;for(int i=0;i<count;++i){{b.n=i&255;sum=(sum+b.run(i&7)+b.get())&65535;}}return sum!={expected};}}'
corpus.extend([('member-runtime',pattern+runtime,True),('member-outside-runtime',outside+runtime,True)])
prior=json.loads((ROOT/'student.tests/pa14/object-performance.json').read_text())
for name in ('calls-4','memory-float-4','object-instances-4000','object-unused-4000','call-materializations-runtime','default-identities-runtime'):
 old=prior['workloads'][name];assert shared.sha(old['source_path'])==old['source_sha256']
 corpus.append((name,Path(old['source_path']).read_text(),'runtime' in old))
for name,source,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands={};runtimes={}
 for b in (0,1):
  out=WORK/(name+f'-{b}.lowir');command=[binaries[b],'--emit-lowir','-O0','-o',out,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,path=str(out),sha256=shared.sha(out),bytes=out.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[b]=[exe]
  commands[b]=command;outputs.append(item)
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 item=dict(source_path=str(src),source_sha256=shared.sha(src),mode='--emit-lowir',outputs=outputs,exact_required=True,equivalence='byte-identical correct compiler/native output',compiler=campaign(commands))
 if runtimes:item['runtime']=campaign(runtimes)
 result['workloads'][name]=item;OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

#!/usr/bin/env python3
"""Final method-declarator ownership: common AA/ABBA compiler/native evidence."""
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
pattern='int one(int n){return n+1;}int two(double n){return n+2;}template<class T>struct C{int n;int (*f(double))(int);static int (*f(int))(double);};template<class U>int (*C<U>::f(double))(int){++n;return one;}template<class U>int (*C<U>::f(int))(double){return two;}'
corpus=[]
for n in (1000,4000):
 uses=''.join(f'struct Tag{i}{{}};int run{i}(C<Tag{i}>& c){{return c.f(1.0)(2)+C<Tag{i}>::f(1)(2.0);}}\n' for i in range(n))
 corpus.append((f'method-instances-{n}',pattern+uses,False))
count=3000000;expected=(7*count)&65535
corpus.append(('method-runtime',pattern+f'int main(){{volatile int count={count};C<int> c;c.n=0;int sum=0;for(int i=0;i<count;++i)sum=(sum+c.f(1.0)(2)+C<int>::f(1)(2.0))&65535;return sum!={expected}||c.n!=count;}}',True))
parent=ROOT/'student.tests/pa14/dependent-object-packed-performance.json'
prior=json.loads(parent.read_text());full=json.loads((ROOT/'student.tests/pa14/dependent-object-final-performance.json').read_text())
result.update(parent_path=str(parent),parent_sha256=shared.sha(parent),preflight=[])
for name,old in full['workloads'].items():
 expected=old['outputs'][1];out=WORK/(name+'-preflight.lowir')
 stats=shared.run([B,old['mode'],'-O0','--stats','--validate-lowir','-o',out,old['source_path']]);assert shared.sha(out)==expected['sha256']
 row=dict(source_path=old['source_path'],source_sha256=old['source_sha256'],path=str(out),sha256=shared.sha(out),telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
 if 'native' in expected:
  exe=WORK/(name+'-preflight');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,out]);shared.run([exe]);assert shared.sha(exe)==expected['native']['sha256'];row.update(native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0)
 result['preflight'].append(row)
for name in ('member-instances-1000','member-outside-1000','member-unused-1000','member-repeated-1000','member-runtime','call-materializations-runtime'):
 old=full['workloads'][name];assert shared.sha(old['source_path'])==old['source_sha256'];corpus.append((name,Path(old['source_path']).read_text(),'runtime' in old))
old=json.loads((ROOT/'student.tests/pa14/object-performance.json').read_text())['workloads']['calls-1'];assert shared.sha(old['source_path'])==old['source_sha256'];corpus.append(('calls-1',Path(old['source_path']).read_text(),False))
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

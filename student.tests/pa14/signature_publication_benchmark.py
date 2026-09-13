#!/usr/bin/env python3
"""Signatures, enums and complete-class source uses: frozen complete corpus."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
import special_signature_compare as comparison
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/declaration-fact-performance.json';old=json.loads(parent.read_text())
binaries=[A.resolve(),B.resolve()]
result=dict(protocol='freeze every input and equivalent output before timing; one warmup each; four A/A observations and two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['97006205',shared.run(['git','rev-parse','HEAD']).stdout.strip()],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),comparison_harness_sha256=shared.sha(comparison.__file__),comparison_adapter_sha256=shared.sha(ROOT/'student.tests/pa14/special_signature_compare.pl'),comparison_sha256=shared.sha(ROOT/'scripts/compare_results_common.pl'),contract_sha256=shared.sha(ROOT/'pa8/lowir.md'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 acceptance='source signature/enum identity, raw parameter facts and typed complete-class uses; source/key/use storage and zero generated growth',workloads={})
corpus=[]
for name,item in old['workloads'].items():
 assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item))
def signature_source(width,requests):
 source='int step(int value){return value+1;}template<class T>struct Signatures{static int seed(){return 1;}\n'
 for j in range(width):
  source+=f'enum E{j}:unsigned char{{mode{j}=1}};int data{j}=seed();\n'
  source+=f'int run{j}(E{j} mode,const typename T::word first,decltype(first) second,typename T::word values[3],typename T::word fn(typename T::word),typename T::word extra=seed()){{return fn(values[0])+first+second+data{j}+extra+int(mode);}}\n'
 return source+'};\n'
def signature_use(tag,width,requests,name):
 source=f'int {name}(int x){{Signatures<{tag}> object;int values[3]={{x,2,3}};int result=0;\n'
 for repeat in range(requests):
  for j in range(width):source+=f'result+=object.run{j}(Signatures<{tag}>::mode{j},x,x,values,step);\n'
 return source+'return result;}\n'
for n,width,requests in ((1000,4,4),(1000,32,4),(4000,4,4),(1000,4,64)):
 source=signature_source(width,requests)
 for i in range(n):
  source+=f'struct Tag{i}{{typedef int word;}};\n'+signature_use(f'Tag{i}',width,requests,f'use{i}')
 corpus.append((f'signature-uses-{n}-{width}-{requests}',source,False))
count=3000000
expected=(48*count*(count-1)//2+64*count)&65535
source=signature_source(4,4)+'struct Tag{typedef int word;};\n'+signature_use('Tag',4,4,'use')
source+=f'int main(){{volatile int count={count};unsigned sum=0;for(int i=0;i<count;++i)sum=(sum+use(i))&65535;return sum!={expected};}}'
corpus.append(('signature-runtime',source,True))
commands={};runtimes={}
for name,source,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands[name]={};runtimes[name]={}
 effective=binaries
 for b,binary in enumerate(effective):
  ir=WORK/(name+f'-{b}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if b==0 and name in old['workloads']:assert item['sha256']==old['workloads'][name]['outputs'][-1]['sha256'],name
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 compared=None
 if outputs[0]['sha256']!=outputs[1]['sha256']:
  compared=comparison.compare(src,outputs,WORK/'comparisons',name)
 if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs)
 if compared:result['workloads'][name]['comparison']=compared
 print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
OUT.write_text(json.dumps(result,indent=2)+'\n')
if os.environ.get('PREFLIGHT_ONLY'):sys.exit(0)
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']


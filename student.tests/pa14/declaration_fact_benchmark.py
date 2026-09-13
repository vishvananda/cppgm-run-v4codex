#!/usr/bin/env python3
"""Concrete declaration bindings and sparse facts: frozen complete corpus."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
import special_signature_compare as comparison
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/special-signature-performance.json';old=json.loads(parent.read_text())
binaries=[A.resolve(),B.resolve()]
result=dict(protocol='freeze every input and equivalent output before timing; one warmup each; four A/A observations and two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['fda0a178',shared.run(['git','rev-parse','HEAD']).stdout.strip()],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),comparison_harness_sha256=shared.sha(comparison.__file__),comparison_adapter_sha256=shared.sha(ROOT/'student.tests/pa14/special_signature_compare.pl'),comparison_sha256=shared.sha(ROOT/'scripts/compare_results_common.pl'),contract_sha256=shared.sha(ROOT/'pa8/lowir.md'),binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 acceptance='source declaration identity and per-frame concrete publication; sparse stable Fact records; one local view per publication; source/key/use storage and zero generated growth',workloads={})
corpus=[]
for name,item in old['workloads'].items():
 assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item))
def local_function(width,requests):
 source='template<class T>int local_use(int x){\n'
 for j in range(width):
  source+=f'struct Cell{j}{{T value;}};typedef Cell{j} Alias{j};Cell{j} object{j};object{j}.value.n=x;\n'
  source+=f'enum Extent{j}{{Count{j}=sizeof(Cell{j}),Step{j}=Count{j}+1}};int storage{j}[Step{j}];storage{j}[0]=x;\n'
  source+=(f'x+=sizeof(Alias{j})+sizeof(decltype(object{j}))+(storage{j}[0]&1);\n')*requests
 return source+'return x;}\n'
for n,width,requests in ((1000,4,4),(1000,32,4),(4000,4,4),(1000,4,64)):
 source=local_function(width,requests)+''.join(f'struct Tag{i}{{int n;}};int run{i}(int x){{return local_use<Tag{i}>(x);}}\n' for i in range(n))
 corpus.append((f'local-facts-{n}-{width}-{requests}',source,False))
count=3000000
expected=(count*(count-1)//2+128*count+16*(count//2))&65535
source=local_function(4,4)+f'struct Tag{{int n;}};int main(){{volatile int count={count};unsigned sum=0;for(int i=0;i<count;++i)sum=(sum+local_use<Tag>(i))&65535;return sum!={expected};}}'
corpus.append(('local-runtime',source,True))
commands={};runtimes={}
for name,source,executable in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands[name]={};runtimes[name]={}
 effective=binaries
 for b,binary in enumerate(effective):
  ir=WORK/(name+f'-{b}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if name in old['workloads']:assert item['sha256']==old['workloads'][name]['outputs'][-1]['sha256'],name
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


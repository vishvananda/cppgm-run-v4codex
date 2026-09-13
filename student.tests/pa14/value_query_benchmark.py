#!/usr/bin/env python3
"""Typed value work: fixed common corpus, source/key scaling, checked native loops."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/region-performance.json';old=json.loads(parent.read_text())
binaries=[A.resolve(),B.resolve()]
result=dict(protocol='freeze all inputs/outputs before timing; one warmup each, four A/A and two ABBA blocks; six B-only samples for new correct behavior',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['c78e8d3b79659a853e2a288ef8285e33317ab37d',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),
 shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 text_metric='compiler .text; supplied sectionless ELF executable payload',
 acceptance='zero common-correct generated growth; value work per canonical query, conversion variants at most four per source operation; source/key/occurrence storage; O0 has no mandated numeric compiler ceiling',workloads={})
corpus=[]
for name,item in old['workloads'].items():
 assert shared.sha(item['source_path'])==item['source_sha256']
 corpus.append((name,Path(item['source_path']).read_text(),'runtime' in item,True))
for n,width in ((1000,8),(1000,128),(4000,8)):
 source='template<class T>unsigned long f(unsigned long x){'+'x+=1+sizeof(T);'*width+'return x;}\n'
 source+=''.join(f'struct Tag{i}{{}};unsigned long run{i}(unsigned long x){{return f<Tag{i}>(x);}}\n' for i in range(n))
 corpus.append((f'value-offset-{n}-{width}',source,False,True))
for n in (1000,4000):
 source='template<class T>int f(int(&a)[static_cast<int>(sizeof(T))+1]){return a[sizeof(T)];}\n'
 source+=''.join(f'struct Tag{i}{{}};int run{i}(int(&a)[2]){{return f<Tag{i}>(a);}}\n' for i in range(n))
 corpus.append((f'value-bound-{n}',source,False,False))
count=12000000
source='template<class T>unsigned long f(unsigned long x){return x+1+sizeof(T);}'
source+=f'int main(){{volatile int count={count};unsigned long sum=0;for(int i=0;i<count;++i)sum=(sum+f<int>(i))&65535;return sum!={(count*(count-1)//2+count*5)&65535};}}'
corpus.append(('value-runtime',source,True,True))
source='template<class T>int f(int(&a)[sizeof(T)?3:1/0]){return a[1];}'
source+=f'int main(){{volatile int count={count};int a[3]={{1,2,3}};int sum=0;for(int i=0;i<count;++i){{a[1]=i&7;sum=(sum+f<int>(a))&65535;}}return sum!={(count//8*28)&65535};}}'
corpus.append(('bound-runtime',source,True,False))
commands={};runtimes={}
for name,source,executable,common in corpus:
 src=WORK/(name+'.cpp');src.write_text(source);outputs=[];commands[name]={};runtimes[name]={}
 rejection=None
 if not common:
  import subprocess
  log=WORK/(name+'-entry.log');r=subprocess.run([binaries[0],'--emit-lowir','-O0','-o',WORK/(name+'-entry.lowir'),src],capture_output=True,text=True,timeout=120)
  log.write_text(r.stdout+r.stderr);assert r.returncode==1,(name,r.returncode,r.stderr)
  rejection=dict(path=str(log),sha256=shared.sha(log),exit_code=r.returncode)
 for b in ((0,1) if common else (1,)):
  ir=WORK/(name+f'-{b}.lowir');command=[binaries[b],'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if name in old['workloads']:assert item['sha256']==old['workloads'][name]['outputs'][-1]['sha256'],name
  if executable:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 if common:
  assert outputs[0]['sha256']==outputs[1]['sha256'],name
  if executable:assert outputs[0]['native']['sha256']==outputs[1]['native']['sha256'],name
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,exact_required=common)
 if rejection:result['workloads'][name]['entry_rejection']=rejection
 print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 rows=[dict(binary=b,**observe(commands[b])) for b in (shared.ORDER if len(commands)==2 else [1]*6)]
 item=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  item.update(aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
 return item
OUT.write_text(json.dumps(result,indent=2)+'\n')
if os.environ.get('PREFLIGHT_ONLY'):sys.exit(0)
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

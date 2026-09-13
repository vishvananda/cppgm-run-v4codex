#!/usr/bin/env python3
"""Isolate cached topology from the already-correct region/default implementation."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/region-standard-performance.json'
old=json.loads(parent.read_text());binaries=[A.resolve(),B.resolve()]
result=dict(protocol='freeze all correct inputs/outputs; one warmup each; four A/A and two ABBA blocks',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],backend_flags=['-O0'],
 commits=['03e76021',shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 parent_path=str(parent),parent_sha256=shared.sha(parent),harness_sha256=shared.sha(__file__),
 shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 acceptance='source topology built once per demanded source region; zero common-correct generated-code growth; bounded source index and demanded occurrence storage',workloads={})
assert result['binaries'][0]['sha256']==old['binaries'][1]['sha256']
commands={};runtimes={}
for name in ('body-run-4000-128','body-large-1000-128','default-unused-4000','region-runtime'):
 previous=old['workloads'][name];assert shared.sha(previous['source_path'])==previous['source_sha256']
 src=WORK/(name+'.cpp');src.write_bytes(Path(previous['source_path']).read_bytes())
 commands[name]={};runtimes[name]={};outputs=[]
 for b,binary in enumerate(binaries):
  ir=WORK/(name+f'-{b}.lowir');command=[binary,'--emit-lowir','-O0','-o',ir,src]
  stats=shared.run([*command,'--stats','--validate-lowir'])
  assert shared.sha(ir)==previous['outputs'][-1]['sha256'],name
  item=dict(binary=b,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
  if 'runtime' in previous:
   exe=WORK/(name+f'-{b}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe])
   assert shared.sha(exe)==previous['outputs'][-1]['native']['sha256']
   item['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0);runtimes[name][b]=[exe]
  outputs.append(item);commands[name][b]=command
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs,exact_required=True)
 print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def campaign(commands):
 warmups=[dict(binary=b,**observe(commands[b])) for b in commands]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 return dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
OUT.write_text(json.dumps(result,indent=2)+'\n')
for name,item in result['workloads'].items():
 item['compiler']=campaign(commands[name])
 if runtimes[name]:item['runtime']=campaign(runtimes[name])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

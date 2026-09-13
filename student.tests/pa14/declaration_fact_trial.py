#!/usr/bin/env python3
"""Frozen sparse-fact diagnostic: no builds or validation run during timing."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
parent=ROOT/'student.tests/pa14/special-signature-performance.json'
old=json.loads(parent.read_text());binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),parent_path=str(parent),parent_sha256=shared.sha(parent),
 shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),cpu=cpu,platform=platform.platform(),
 flags=['--emit-lowir','-O0'],protocol='freeze equal validated outputs; one warmup each, four A/A samples, two ABBA blocks',
 binaries=[dict(path=str(b),sha256=shared.sha(b),text_bytes=shared.text_size(b)) for b in binaries],workloads={})
commands={}
for name in ('body-large-1000-128','special-uses-1000-128-4','input-uses-1000-128'):
 previous=old['workloads'][name];source=WORK/(name+'.cpp');source.write_bytes(Path(previous['source_path']).read_bytes())
 assert shared.sha(source)==previous['source_sha256']
 row=dict(source_path=str(source),source_sha256=shared.sha(source),outputs=[]);commands[name]={}
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,source]
  checked=shared.run([*command,'--stats','--validate-lowir']);commands[name][b]=command
  log=WORK/f'{name}-{b}.log';log.write_text(checked.stdout+checked.stderr)
  row['outputs'].append(dict(path=str(ir),sha256=shared.sha(ir),log=str(log),log_sha256=shared.sha(log),
   telemetry=[json.loads(line) for line in checked.stderr.splitlines()]))
 assert row['outputs'][0]['sha256']==row['outputs'][1]['sha256']==previous['outputs'][-1]['sha256'],name
 result['workloads'][name]=row;print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),
  involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
for name,row in result['workloads'].items():
 warmups=[dict(binary=b,**observe(commands[name][b])) for b in (0,1)]
 samples=[dict(binary=b,**observe(commands[name][b])) for b in shared.ORDER]
 row['compiler']=dict(warmups=warmups,observations=samples,
  aa_range_s=[min(r['wall_s'] for r in samples[:4]),max(r['wall_s'] for r in samples[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in samples[k:k+4] if r['binary']==1)/
   statistics.mean(r['wall_s'] for r in samples[k:k+4] if r['binary']==0) for k in (4,8)])
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'measured',flush=True)
for binary in result['binaries']:assert shared.sha(binary['path'])==binary['sha256']

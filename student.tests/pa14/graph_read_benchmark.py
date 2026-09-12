#!/usr/bin/env python3
"""Isolate the source-view fast path using the already frozen correct corpus."""
from pathlib import Path
import json, os, platform, statistics, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
B,C,WORK,OUT=map(Path,sys.argv[1:5]);binaries=[B.resolve(),C.resolve()]
WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
prior=json.loads((ROOT/'student.tests/pa14/preliminary-performance.json').read_text())
result=dict(protocol='warmup, A/A four observations, two ABBA blocks; batched small compiles with every constituent sample retained',
 cpu=cpu,platform=platform.platform(),harness_sha256=shared.sha(__file__),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],
 acceptance=dict(phase='compiler source-graph access',per_read_work='O(1), no new allocation',generated_lowir_growth_bytes=0,
  note='Host C++ inlining changes compiler text; no target optimization pass. Preserve compiler text cost and require measured benefit on affected common-correct work.'),workloads={})
def observe(command,repeat):
 rows=[]
 for i in range(repeat):
  usage=WORK/'usage.txt';start=time.perf_counter_ns()
  shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
  rss,user,system,involuntary,voluntary=usage.read_text().split()
  rows.append(dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0))
 return dict(samples=rows,wall_s=sum(r['wall_s'] for r in rows),rss_kib=max(r['rss_kib'] for r in rows))
for name,repeat in [('calls-4',1),('memory-float-4',1),('template-semantics-4',4),('class-instances-1000',8)]:
 old=prior['workloads'][name];src=Path(old['source_path']);assert shared.sha(src)==old['source_sha256']
 commands={};outputs=[]
 for b,binary in enumerate(binaries):
  ir=WORK/(name+f'-{b}.out');mode=old['mode']
  command=[binary,mode,*(['-O0'] if mode=='--emit-lowir' else []),'-o',ir,src]
  shared.run(command);outputs.append(dict(path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size));commands[b]=command
 assert outputs[0]['sha256']==outputs[1]['sha256'],name
 warmups=[dict(binary=b,**observe(c,repeat)) for b,c in commands.items()]
 rows=[dict(binary=b,**observe(commands[b],repeat)) for b in shared.ORDER]
 pairs=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
 result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),mode=mode,repeat=repeat,outputs=outputs,warmups=warmups,observations=rows,paired_c_over_b=pairs)
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,pairs,flush=True)
assert [shared.sha(p) for p in binaries]==[b['sha256'] for b in result['binaries']]

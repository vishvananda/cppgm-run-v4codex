#!/usr/bin/env python3
"""Focused repeat of noisy compiler samples; frozen final campaign is the input."""
from pathlib import Path
import hashlib,json,os,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
record=json.loads((ROOT/'student.tests/pa16/initialization-performance.json').read_text())
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
OUT=Path(sys.argv[2]);os.sched_setaffinity(0,{record['cpu']})
result=dict(protocol='one warmup batch per binary; four A/A batches then four ABBA blocks; retain each invocation; batch tiny startup-dominated source 32 times',
 campaign_sha256=shared.sha(ROOT/'student.tests/pa16/initialization-performance.json'),harness_sha256=shared.sha(__file__),
 cpu=record['cpu'],binaries=record['binaries'],workloads={})
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
for name,repeats in [('memory-float-1000',1),('runtime-copy-8',32)]:
 source=record['workloads'][name];src=Path(source['source_path']);assert shared.sha(src)==source['source_sha256']
 item=dict(source_path=str(src),source_sha256=source['source_sha256'],repeats=repeats,warmups=[],observations=[])
 result['workloads'][name]=item
 def batch(b):
  values=[]
  for i in range(repeats):
   ir=WORK/(name+'.lowir');usage=WORK/'usage.txt';start=time.perf_counter_ns()
   shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,result['binaries'][b]['path'],'--emit-lowir','-O0','-o',ir,src])
   rss,user,system,iv,v=usage.read_text().split()
   values.append(dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0))
   assert shared.sha(ir)==source['outputs'][b]['sha256']
  return dict(binary=b,wall_s=sum(x['wall_s'] for x in values),rss_kib=max(x['rss_kib'] for x in values),invocations=values)
 item['warmups']=[batch(b) for b in [0,1]]
 for b in [0]*4+[0,1,1,0]*4:
  item['observations'].append(batch(b));OUT.write_text(json.dumps(result,indent=2)+'\n')
 rows=item['observations'];item['aa_range_s']=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])]
 item['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary']==1)/statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary']==0) for k in [4,8,12,16]]
 print(name,item['paired_b_over_a'],flush=True)
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());OUT.write_text(json.dumps(result,indent=2)+'\n')

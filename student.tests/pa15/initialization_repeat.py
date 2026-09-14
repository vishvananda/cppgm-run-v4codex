#!/usr/bin/env python3
"""Resolve the final array compiler outlier without dropping any observation."""
from pathlib import Path
import json,os,statistics,subprocess,sys,time,hashlib
ROOT=Path(__file__).resolve().parents[2]
source=Path(sys.argv[1]);out=Path(sys.argv[2]);d=json.loads(source.read_text())
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
os.sched_setaffinity(0,{d['cpu']})
order=[0,0,0,0,0,1,1,0,0,1,1,0]
result=dict(reason='arrays-4000 final campaign has B/A pairs 0.995 and 1.184 with near-equal medians; repeat the same frozen compiler workload to distinguish a repeatable regression from stalls',campaign_sha256=sha(source),harness_sha256=sha(__file__),timing_scope='subprocess only; usage parsing and output SHA verification excluded',cpu=d['cpu'],binaries=d['binaries'],workloads={})
for name in ['arrays-4000','wide-signatures-4000']:
 w=d['workloads'][name];src=Path(w['source_path']);assert sha(src)==w['source_sha256']
 commands={};outputs={}
 for i,b in enumerate(d['binaries']):
  assert sha(b['path'])==b['sha256']
  target=src.parent/(name+f'-repeat-{i}.lowir');outputs[i]=target
  commands[i]=[b['path'],'--emit-lowir','-O0','-o',str(target),str(src)]
 def observe(i):
  usage=src.parent/'repeat-usage.txt';start=time.perf_counter_ns()
  r=subprocess.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',str(usage),*commands[i]],capture_output=True,text=True);assert not r.returncode,r.stderr
  elapsed=(time.perf_counter_ns()-start)/1e9
  rss,user,system,iv,v=usage.read_text().split()
  assert sha(outputs[i])==w['outputs'][i]['sha256']
  return dict(binary=i,wall_s=elapsed,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
 warmups=[observe(i) for i in [0,1]];rows=[observe(i) for i in order]
 result['workloads'][name]=dict(source_sha256=sha(src),warmups=warmups,observations=rows,paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in [4,8]])
 out.write_text(json.dumps(result,indent=2)+'\n');print(name,result['workloads'][name]['paired_b_over_a'],flush=True)

#!/usr/bin/env python3
"""Explicit frozen B/C follow-up for scalar path cost and observed timing spikes."""
import json,os,statistics,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
prior,final,output=map(Path,sys.argv[1:4]);a=json.loads(prior.read_text());b=json.loads(final.read_text())
work=output.parent/'scalar-fast-control';work.mkdir(exist_ok=True)
os.sched_setaffinity(0,{b['cpu']})
binaries=[Path(x['binaries'][1]['path']) for x in [a,b]]
for i,p in enumerate(binaries):assert shared.sha(p)==[a,b][i]['binaries'][1]['sha256']
result=dict(protocol='one warmup each; four A/A; four ABBA blocks; all observations retained',cpu=b['cpu'],inputs=[str(prior),str(final)],input_hashes=[shared.sha(p) for p in [prior,final]],binaries=[x['binaries'][1] for x in [a,b]],workloads={})
for name in ['template-4000','literal-classes-4000','object-values-1000','object-values-4000','sparse-value-keys-1000','sparse-value-keys-4000']:
 w=b['workloads'][name];src=Path(w['source_path']);assert shared.sha(src)==w['source_sha256']
 item=dict(source=w['source_path'],sha256=w['source_sha256'],warmups=[],observations=[]);result['workloads'][name]=item
 def observe(i):
  ir=work/f'{name}-{i}.lowir';usage=work/'usage';start=time.perf_counter_ns()
  shared.run(['/usr/bin/time','-f','%M %U %S %c','-o',usage,binaries[i],'--emit-lowir','-O0','-o',ir,src])
  wall=(time.perf_counter_ns()-start)/1e9;rss,user,system,switch=usage.read_text().split()
  assert shared.sha(ir)==w['outputs'][-1]['sha256']
  return dict(binary=i,wall_s=wall,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(switch),checked_exit=0)
 item['warmups']=[observe(i) for i in [0,1]]
 item['observations']=[observe(i) for i in [0]*4+[0,1,1,0]*4]
 rows=item['observations'];item['paired_C_over_B']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in [4,8,12,16]]
 output.write_text(json.dumps(result,indent=2)+'\n');print(name,item['paired_C_over_B'],flush=True)

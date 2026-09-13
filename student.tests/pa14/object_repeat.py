#!/usr/bin/env python3
"""Retain a focused repeat of noisy/common-correct receiver campaign cases."""
from pathlib import Path
import json,os,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
WORK,OUT=map(Path,sys.argv[1:3]);WORK.mkdir(parents=True,exist_ok=True)
parent=ROOT/'student.tests/pa14/object-performance.json'
prior=json.loads(parent.read_text());os.sched_setaffinity(0,{prior['cpu']})
result={k:v for k,v in prior.items() if k!='workloads'}
result.update(harness_sha256=shared.sha(__file__),parent_path=str(parent),parent_sha256=shared.sha(parent),
 purpose='Repeat wall/CPU outliers and large common-correct call cases; preserve initial observations',workloads={})
for b in prior['binaries']:assert shared.sha(b['path'])==b['sha256']
def observe(command):
 usage=WORK/'usage.txt';started=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-started)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
for name in ('memory-float-1','template-semantics-4','call-instances-4000','call-materializations-4000','object-instances-4000','object-results-1000'):
 old=prior['workloads'][name];assert shared.sha(old['source_path'])==old['source_sha256']
 commands={};outputs=[]
 for b in (0,1):
  out=WORK/(name+f'-{b}.lowir')
  command=[prior['binaries'][b]['path'],old['mode'],'-O0','-o',out,old['source_path']]
  shared.run(command);assert shared.sha(out)==old['outputs'][b]['sha256']
  outputs.append(dict(old['outputs'][b],path=str(out)));commands[b]=command
 warmups=[dict(binary=b,**observe(commands[b])) for b in (0,1)]
 rows=[dict(binary=b,**observe(commands[b])) for b in shared.ORDER]
 campaign=dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)])
 result['workloads'][name]=dict(old,outputs=outputs,compiler=campaign)
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,flush=True)
for b in prior['binaries']:assert shared.sha(b['path'])==b['sha256']

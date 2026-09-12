#!/usr/bin/env python3
"""Repeat noisy large-class measurements using the same frozen experiment."""
from pathlib import Path
import hashlib,json,os,statistics,subprocess,time
root=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
report=root/'student.tests/pa13/final-audit-performance.json'
data=json.loads(report.read_text());os.sched_setaffinity(0,{data['cpu']})
out=dict(source_report_sha256=sha(report),harness_sha256=sha(__file__),cpu=data['cpu'],
 protocol='warmups; four A/A observations; four ABBA blocks',workloads={})
for binary in data['binaries']:assert sha(binary['path'])==binary['sha256']
for name in ('class-4000','virtual-4000'):
 w=data['workloads'][name];src=Path(w['source_path']);assert sha(src)==w['source_sha256']
 commands=[[b['path'],w['mode'],'-O0','-o',w['outputs'][i]['path'],str(src)] for i,b in enumerate(data['binaries'])]
 def observe(i):
  usage=src.parent/'noise-usage.txt';start=time.perf_counter_ns()
  r=subprocess.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',str(usage),*commands[i]],capture_output=True,text=True,timeout=60)
  elapsed=(time.perf_counter_ns()-start)/1e9
  assert r.returncode==0 and not r.stderr,r.stderr
  rss,user,system,involuntary,voluntary=usage.read_text().split()
  assert sha(w['outputs'][i]['path'])==w['outputs'][i]['sha256']
  return dict(binary=i,wall_s=elapsed,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
 warmups=[observe(i) for i in (0,1)]
 rows=[observe(i) for i in [0]*4+[0,1,1,0]*4]
 out['workloads'][name]=dict(source_path=str(src),source_sha256=sha(src),output_hashes=[o['sha256'] for o in w['outputs']],warmups=warmups,observations=rows,
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8,12,16)])
 (root/'student.tests/pa13/final-noise-performance.json').write_text(json.dumps(out,indent=2)+'\n')
 print(name,flush=True)

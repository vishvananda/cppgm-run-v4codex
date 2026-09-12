#!/usr/bin/env python3
"""CPU-accounted follow-up for noisy corrected-path scaling observations."""
from pathlib import Path
import hashlib,json,os,subprocess,time
root=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
p=root/'student.tests/pa13/final-audit-performance.json';d=json.loads(p.read_text())
os.sched_setaffinity(0,{d['cpu']});binary=d['binaries'][1];assert sha(binary['path'])==binary['sha256']
result=dict(source_report_sha256=sha(p),harness_sha256=sha(__file__),cpu=d['cpu'],
 protocol='correct B only; both scales warmed; alternating 400/1600 four times; no A/B speed claim',workloads={})
for group in ('array','destructor','conversion'):
 def observe(scale):
  name=group+'-'+str(scale);w=d['workloads'][name];source=Path(w['source_path']);output=w['outputs'][0]
  assert sha(source)==w['source_sha256'];usage=source.parent/'scaling-usage.txt'
  start=time.perf_counter_ns()
  r=subprocess.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',str(usage),binary['path'],w['mode'],'-O0','-o',output['path'],str(source)],capture_output=True,text=True,timeout=60)
  elapsed=(time.perf_counter_ns()-start)/1e9
  assert r.returncode==0 and not r.stderr,r.stderr
  assert sha(output['path'])==output['sha256']
  rss,user,system,involuntary,voluntary=usage.read_text().split()
  return dict(scale=scale,wall_s=elapsed,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
 result['workloads'][group]=dict(warmups=[observe(s) for s in (400,1600)],observations=[observe(s) for s in (400,1600)*4])
 (root/'student.tests/pa13/final-scaling-performance.json').write_text(json.dumps(result,indent=2)+'\n')
 print(group,flush=True)

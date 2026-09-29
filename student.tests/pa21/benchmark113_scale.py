#!/usr/bin/env python3
"""Interleave final-only 512/2048-function compilation to diagnose timing drift."""
from pathlib import Path
import hashlib,json,os,statistics,subprocess,sys,time
CC,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
sizes=(512,2048)
sources=[WORK/('contextual-new-%d-final.cpp'%n) for n in sizes]
commands=[[str(CC),'--emit-lowir','-O0','-o',str(p.with_suffix('.lowir')),str(p)] for p in sources]
def observe(i):
 usage=WORK/'scale-time.txt';start=time.perf_counter_ns()
 p=subprocess.run(['/usr/bin/time','-f','%e %M %U %S %c %w','-o',str(usage),*commands[i]],capture_output=True,text=True,timeout=30)
 assert p.returncode==0,p.stderr
 wall=(time.perf_counter_ns()-start)/1e9
 elapsed,rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(size=sizes[i],wall_s=wall,command_elapsed_s=float(elapsed),rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary))
result=dict(protocol='final-only different-input growth diagnostic, warmups then four A/A observations and four small/large/large/small blocks; no optimization speedup claim',cpu=cpu,compiler_sha256=sha(CC),inputs=[dict(path=str(p),sha256=sha(p)) for p in sources],commands=commands)
result['warmups']=[observe(i) for i in (0,1)]
result['observations']=[observe(i) for i in [0]*4+[0,1,1,0]*4]
rows=result['observations'];result['paired_large_over_small']=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['size']==2048)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['size']==512) for j in range(4,len(rows),4)]
result['summary']={n:dict(median_wall_s=statistics.median(r['wall_s'] for r in rows[4:] if r['size']==n),wall_range_s=[min(r['wall_s'] for r in rows[4:] if r['size']==n),max(r['wall_s'] for r in rows[4:] if r['size']==n)],peak_rss_kib=max(r['rss_kib'] for r in rows[4:] if r['size']==n)) for n in sizes}
OUT.write_text(json.dumps(result,indent=2)+'\n')
print(result['paired_large_over_small'])

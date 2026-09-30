#!/usr/bin/env python3
"""Diagnose the observed O0 scalar-loop regression using fixed object code."""
from pathlib import Path
import hashlib,json,os,statistics,subprocess,time
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-119/placement');WORK.mkdir(exist_ok=True)
OUT=ROOT/'student.tests/pa22/placement119.json';assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(c):
 p=subprocess.run([str(x) for x in c],capture_output=True,text=True);assert p.returncode==0,(c,p.stderr);return p.stdout
objects=[Path('/tmp/pa22-119/performance-affected/runtime-conditional%d.o'%i) for i in (0,1)]
result=dict(protocol='same frozen objects; four A/A observations then four ABBA blocks per text address; runtime only, diagnostic, no compiler-speed claim',
 cpu=cpu,objects=[dict(path=str(p),sha256=sha(p)) for p in objects],placements=[])
result['original_binaries']=[]
for name in ('runtime-conditional','runtime-member-selection'):
 for lane in (0,1):
  exe=Path('/tmp/pa22-119/performance-affected')/(name+str(lane))
  result['original_binaries'].append(dict(workload=name,lane=lane,sha256=sha(exe),
   main_disassembly=run(['objdump','-d','--disassemble=main',exe])))
def observe(exe):
 start=time.perf_counter_ns();run([exe]);return dict(wall_s=(time.perf_counter_ns()-start)/1e9,checked_exit=0)
for address in (0x402000,0x402010,0x402020,0x402030):
 row=dict(address=hex(address),binaries=[]);exes=[]
 for lane,obj in enumerate(objects):
  exe=WORK/('%x-%d'%(address,lane));cmd=['g++','-no-pie','-Wl,-Ttext='+hex(address),obj,'-o',exe];run(cmd);run([exe]);exes.append(exe)
  dis=run(['objdump','-d','--disassemble=main',exe])
  row['binaries'].append(dict(command=[str(x) for x in cmd],sha256=sha(exe),main_disassembly=dis))
 row['warmups']=[dict(lane=i,**observe(x)) for i,x in enumerate(exes)]
 samples=[dict(lane=i,**observe(exes[i])) for i in [0]*4+[0,1,1,0]*4];row['observations']=samples
 row['aa_range_s']=[min(x['wall_s'] for x in samples[:4]),max(x['wall_s'] for x in samples[:4])]
 row['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in samples[j:j+4] if x['lane']==1)/statistics.mean(x['wall_s'] for x in samples[j:j+4] if x['lane']==0) for j in range(4,20,4)]
 result['placements'].append(row);OUT.write_text(json.dumps(result,indent=2)+'\n');print(hex(address),row['paired_b_over_a'],flush=True)
assert [sha(p) for p in objects]==[r['sha256'] for r in result['objects']]

#!/usr/bin/env python3
"""Repeat the wide local-declaration observation without changing the compiler."""
from pathlib import Path
import json,os,platform,shutil,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
PARENT,WORK,OUT=map(Path,sys.argv[1:4]);WORK.mkdir(parents=True,exist_ok=True)
parent=json.loads(PARENT.read_text());cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
binaries=[Path(b['path']) for b in parent['binaries']]
result=dict(parent_path=str(PARENT.resolve()),parent_sha256=shared.sha(PARENT),harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(shared.__file__),
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags=parent['build_flags'],binaries=parent['binaries'],
 reason='Retain the full campaign; its wide local case shifted from 11-second A/A samples to 25-31-second A/B samples. Repeat unchanged binaries and input to distinguish a persistent regression from that unisolated transition.',workloads={})
name='local-facts-1000-32-4';old=parent['workloads'][name];src=WORK/(name+'.cpp');shutil.copyfile(old['source_path'],src)
assert shared.sha(src)==old['source_sha256'];commands={};outputs=[]
for b,binary in enumerate(binaries):
 assert shared.sha(binary)==parent['binaries'][b]['sha256']
 ir=WORK/f'{name}-{b}.lowir';command=[binary,'--emit-lowir','-O0','-o',ir,src]
 stats=shared.run([*command,'--stats','--validate-lowir'])
 item=dict(binary=b,binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
 assert item['sha256']==old['outputs'][b]['sha256'];outputs.append(item);commands[b]=command
result['workloads'][name]=dict(source_path=str(src),source_sha256=shared.sha(src),outputs=outputs)
OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'preflight',flush=True)
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
c=dict(warmups=[],observations=[]);result['workloads'][name]['compiler']=c
# Persist each completed observation so even an interrupted repeat is retained.
for b in (0,1):
 c['warmups'].append(dict(binary=b,**observe(commands[b])));OUT.write_text(json.dumps(result,indent=2)+'\n')
for b in shared.ORDER:
 c['observations'].append(dict(binary=b,**observe(commands[b])));OUT.write_text(json.dumps(result,indent=2)+'\n')
rows=c['observations'];c['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
c['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
OUT.write_text(json.dumps(result,indent=2)+'\n')
for b,binary in enumerate(binaries):assert shared.sha(binary)==parent['binaries'][b]['sha256']
print(name,'14 observations complete',flush=True)

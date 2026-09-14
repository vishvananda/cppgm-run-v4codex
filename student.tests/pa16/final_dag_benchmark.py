#!/usr/bin/env python3
"""Amplify the frozen audit DAG controls above compiler startup: A B WORK OUT."""
from pathlib import Path
import json, os, platform, statistics, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
original=json.loads((ROOT/'student.tests/pa16/final-performance.json').read_text())
record=dict(protocol='one warmup each; four A/A observations then four ABBA blocks',cpu=cpu,platform=platform.platform(),
 source_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),flags=['--emit-lowir','-O0'],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in [A,B]],
 translation_units=32,workloads={},started_utc=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime()))
assert [b['sha256'] for b in record['binaries']]==[b['sha256'] for b in original['binaries']]
def save():OUT.write_text(json.dumps(record,indent=2)+'\n')
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
for depth in [14,18,22]:
 name=f'shared-object-dag-{depth}-32tu'
 source=original['workloads'][f'shared-object-dag-{depth}']['source'].removesuffix('int main(){return 0;}')
 paths=[]
 for i in range(32):
  p=WORK/f'{name}-{i}.cpp';p.write_text(source);paths.append(p)
 item=dict(source=source,source_sha256=shared.sha(paths[0]),input_paths=[str(p) for p in paths],comparison='exact',outputs=[])
 record['workloads'][name]=item;commands={}
 for i,binary in enumerate([A,B]):
  ir=WORK/f'{name}-{i}.lowir';commands[i]=[binary,'--emit-lowir','-O0','-o',ir,*paths]
  r=shared.run([*commands[i],'--stats','--validate-lowir'])
  item['outputs'].append(dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]))
 assert item['outputs'][0]['sha256']==item['outputs'][1]['sha256']
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 rows=[dict(binary=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
 aa=[r['wall_s'] for r in rows[:4]]
 item['compiler']=dict(warmups=warmups,observations=rows,aa_range_s=[min(aa),max(aa)],
  paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in range(4,len(rows),4)])
 for out in item['outputs']:assert shared.sha(out['path'])==out['sha256']
 save();print(name,'measured',flush=True)
for b in record['binaries']:assert shared.sha(b['path'])==b['sha256']
record['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

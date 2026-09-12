#!/usr/bin/env python3
"""Recheck noisy compiler groups using the original frozen binaries and inputs."""
from pathlib import Path
import json,os,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
source=json.loads((ROOT/'student.tests/pa13/performance.json').read_text())
out=Path(sys.argv[1]);work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{source['cpu']})
order=[0]*4+[0,1,1,0]*4
report=dict(protocol='same frozen A/B inputs and flags; warmups, A/A four rows, four ABBA blocks',
 source_report_sha256=shared.sha(ROOT/'student.tests/pa13/performance.json'),harness_sha256=shared.sha(__file__),
 cpu=source['cpu'],binaries=source['binaries'],compiler_flags=source['compiler_flags'],workloads={})
for b in source['binaries']:assert shared.sha(b['path'])==b['sha256']
for name in ('class-4000','template-semantics-4'):
 w=source['workloads'][name];assert shared.sha(w['source_path'])==w['source_sha256']
 commands={}
 for b in (0,1):
  output=work/(name+f'-{b}.out')
  commands[b]=[source['binaries'][b]['path'],w['mode'],*(['-O0'] if w['mode']=='--emit-lowir' else []),'-o',output,w['source_path']]
  shared.run(commands[b]);assert shared.sha(output)==w['outputs'][b]['sha256']
 def observe(b):
  usage=work/'usage';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M','-o',usage,*commands[b]])
  return dict(binary=b,wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()),checked_exit=0)
 warmups=[observe(b) for b in (0,1)];rows=[observe(b) for b in order]
 ratios=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8,12,16)]
 report['workloads'][name]=dict(source_path=w['source_path'],source_sha256=w['source_sha256'],warmups=warmups,observations=rows,paired_b_over_a=ratios,
  output_hashes=[shared.sha(work/(name+f'-{b}.out')) for b in (0,1)])
 out.write_text(json.dumps(report,indent=2)+'\n');print(name,ratios,flush=True)

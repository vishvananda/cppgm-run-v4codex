#!/usr/bin/env python3
"""Repeat only the noisy template compiler comparison, retaining every sample."""
from pathlib import Path
import json,os,statistics,subprocess,time
ROOT=Path(__file__).resolve().parents[2]
import sys
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import sha,run
ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa23-121';prior=ROOT/'student.tests/pa23/performance121-common.json'
d=json.loads(prior.read_text());name='auto-specializations-9600';w=d['workloads'][name]
os.sched_setaffinity(0,{d['cpu']});src=ART/'performance-common'/(name+'.cpp');assert sha(src)==w['source_sha256']
commands=[]
for i,b in enumerate(d['binaries']):
 assert sha(b['path'])==b['sha256'];commands.append([b['path'],'--emit-lowir','-O0','-o',str(ART/'performance-common'/(name+str(i)+'.lowir')),str(src)])
def observe(i):
 usage=ART/'remeasure-usage';start=time.perf_counter_ns();run(['/usr/bin/time','-f','%M','-o',usage,*commands[i]])
 assert sha(commands[i][-2])==w['outputs'][i]['lowir_sha256']
 return dict(lane=i,wall_s=(time.perf_counter_ns()-start)/1e9,peak_rss_kib=int(usage.read_text()))
result=dict(reason='A/A spread and ABBA dispersion in sealed full run; no virtual or RTTI work on this input',prior=str(prior.relative_to(ROOT)),prior_sha256=sha(prior),binaries=d['binaries'],cpu=d['cpu'],source_sha256=sha(src),warmups=[observe(0),observe(1)],observations=[observe(i) for i in [0]*4+[0,1,1,0]*8])
rows=result['observations'];result['aa_range_s']=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])]
result['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['lane']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['lane']==0) for j in range(4,len(rows),4)]
for i in (0,1):
 lane=[r for r in rows[4:] if r['lane']==i];result[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in lane),range_wall_s=[min(r['wall_s'] for r in lane),max(r['wall_s'] for r in lane)],peak_rss_kib=max(r['peak_rss_kib'] for r in lane))
(ROOT/'student.tests/pa23/performance121-template-repeat.json').write_text(json.dumps(result,indent=2)+'\n')
print(result['0'],result['1'],result['paired_b_over_a'])

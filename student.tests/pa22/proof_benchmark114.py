#!/usr/bin/env python3
"""Measure the full bounded proof against a frozen analysis-disabled compiler."""
from pathlib import Path
import json,os,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
A,B,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:5]];WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each, four A/A observations, four ABBA blocks',cpu=cpu,
 experiment='A skips zero-adjustment analysis and consumption; both retain identical access/write checking',
 flags=['--emit-lowir','-O0'],binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (A,B)],
 source_protocol_sha256=sha(__file__),workloads={})
def observe(command):
 usage=WORK/'usage';start=time.perf_counter_ns();run(['/usr/bin/time','-f','%M','-o',usage,*command])
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,peak_rss_kib=int(usage.read_text()),checked_exit=0)
def measure(commands):
 warm=[dict(lane=i,**observe(c)) for i,c in enumerate(commands)]
 rows=[dict(lane=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
 data=dict(warmups=warm,observations=rows)
 data['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==1)/statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==0) for j in range(4,len(rows),4)]
 for i in (0,1):
  samples=[x for x in rows[4:] if x['lane']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(x['wall_s'] for x in samples),range_wall_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
 return data
prior=json.loads((ROOT/'student.tests/pa22/performance114.json').read_text())
for name in ('runtime-member','member-functions-2048'):
 source=prior['workloads'][name]['source'];src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source_sha256=sha(src),outputs=[]);commands=[];executables=[];result['workloads'][name]=item
 for i,cc in enumerate((A,B)):
  ir=WORK/(name+str(i)+'.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('')
  command=[cc,'--emit-lowir','-O0','-o',ir,src];commands.append(command)
  stat=run([*command,'--stats','--validate-lowir'])
  run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe]);executables.append([exe])
  item['outputs'].append(dict(lowir_sha256=sha(ir),telemetry=[json.loads(s) for s in stat.stderr.splitlines()],text_bytes=text_size(exe),runtime_exit=0))
 item['compiler']=measure(commands);item['runtime']=measure(executables)
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'complete',flush=True)
assert [sha(p) for p in (A,B)]==[x['sha256'] for x in result['binaries']]

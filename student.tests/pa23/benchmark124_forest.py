#!/usr/bin/env python3
"""PA23 shared-view compilation workload long enough to dominate startup."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
A,B,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:5]];blocks=8 if len(sys.argv)>5 else 4
WORK.mkdir(parents=True,exist_ok=True);assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='warmup each lane; four A/A observations; %d ABBA blocks; separate checked execution; all lanes are correct on these inputs'%blocks,cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],implementation=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (A,B)],backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),host=run(['g++','--version']).stdout.splitlines()[0],workloads={})
def observe(command):
 usage=WORK/'usage';start=time.perf_counter_ns();run(['/usr/bin/time','-f','%M','-o',usage,*command]);return dict(wall_s=(time.perf_counter_ns()-start)/1e9,peak_rss_kib=int(usage.read_text()),checked_exit=0)
def measure(commands):
 warm=[dict(lane=i,**observe(c)) for i,c in enumerate(commands)]
 order=[0]*4+([0,1,1,0]*blocks if len(commands)==2 else [0]*8)
 rows=[dict(lane=i,**observe(commands[i])) for i in order]
 data=dict(warmups=warm,observations=rows,aa_range_s=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])])
 if len(commands)==2:data['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==1)/statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==0) for j in range(4,len(rows),4)]
 for i in range(len(commands)):
  samples=[x for x in rows[4:] if x['lane']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(x['wall_s'] for x in samples),range_wall_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
 return data
source=''
for lane in range(512):
 source+='namespace N%d{struct V{virtual int f(){return 7;}};struct A0:virtual V{};struct B0:virtual V{};'%lane
 for i in range(1,9):source+='struct A%d:virtual A%d,virtual B%d{};struct B%d:virtual A%d,virtual B%d{};'%(i,i-1,i-1,i,i-1,i-1)
 source+='int call(A8&a){return a.f();}}'
sources={'shared-forest-512-depth8':source+'int main(){return 0;}'}
for name,source in sources.items():
 src=WORK/(name+'.cpp');src.write_text(source);final_only=name.startswith('virtual-declarations-');compilers=(B,) if final_only else (A,B)
 item=dict(source=source,source_sha256=sha(src),final_only=final_only,outputs=[]);commands=[];exes=[];result['workloads'][name]=item
 for i,cc in enumerate(compilers):
  ir=WORK/(name+str(i)+'.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('')
  command=[cc,'--emit-lowir','-O0','-o',ir,src];commands.append(command);run(command);plain=sha(ir)
  stats=run([*command,'--stats','--validate-lowir']);assert sha(ir)==plain
  run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe]);exes.append([exe])
  section=ir.with_suffix('.text');run(['objcopy','-O','binary','--only-section=.text',exe,section])
  item['outputs'].append(dict(compiler_sha256=sha(cc),lowir_sha256=plain,lowir_bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in stats.stderr.splitlines()],text_bytes=text_size(exe),text_sha256=sha(section),exe_sha256=sha(exe),checked_exit=0))
 item['compiler']=measure(commands);item['runtime']=measure(exes)
 if len(compilers)==2:item['identical_text']=item['outputs'][0]['text_sha256']==item['outputs'][1]['text_sha256']
 if name.startswith('runtime-'):item['compiler_note']='small startup-sensitive TU; no latency speed claim'
 else:item['runtime_note']='checked native emission; startup-sensitive, no runtime speed claim'
 if final_only:item['acceptance']='new required semantics; entry is incorrect on shared final overriders, so no A/B speed claim'
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'complete',flush=True)
assert [sha(p) for p in (A,B)]==[x['sha256'] for x in result['binaries']]

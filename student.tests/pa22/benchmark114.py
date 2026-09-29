#!/usr/bin/env python3
"""Frozen PA22 entry/final and proof-off compiler/runtime evidence."""
from pathlib import Path
import hashlib,json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
A,B,C,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:6]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup per lane, four A/A observations, four ABBA blocks; checked hosted execution separately',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],stage_base='a8482d768bd2dcede42ea63ef39e39cf3245c380',
 final_commit=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),
 binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (A,B,C)],
 backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),host=run(['g++','--version']).stdout.splitlines()[0],
 acceptance='PA22/O0 correctness: bounded 64-node proof with conservative fallback, constant IR per member operation; no added numeric exit gate',
 workloads={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(cmd,repetitions=1):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();maxrss=0
 for i in range(repetitions):
  run(['/usr/bin/time','-f','%M','-o',usage,*cmd]);maxrss=max(maxrss,int(usage.read_text()))
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,peak_rss_kib=maxrss,repetitions=repetitions,checked_exit=0)
def measure(commands,repetitions=1):
 warm=[dict(lane=i,**observe(c,repetitions)) for i,c in enumerate(commands)]
 rows=[dict(lane=i,**observe(commands[i],repetitions)) for i in [0]*4+[0,1,1,0]*4]
 data=dict(warmups=warm,observations=rows,aa_range_s=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])])
 data['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==1)/statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==0) for j in range(4,len(rows),4)]
 for i in (0,1):
  values=[x for x in rows[4:] if x['lane']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(x['wall_s'] for x in values),wall_range_s=[min(x['wall_s'] for x in values),max(x['wall_s'] for x in values)],peak_rss_kib=max(x['peak_rss_kib'] for x in values))
 return data
def build(cc,name,source):
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('')
 cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
 run(cmd);plain=sha(ir);stats=run([*cmd,'--stats','--validate-lowir']);assert sha(ir)==plain
 run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe])
 return cmd,[exe],dict(source_sha256=sha(src),lowir_sha256=plain,lowir_bytes=ir.stat().st_size,
  telemetry=[json.loads(s) for s in stats.stderr.splitlines()],native=dict(sha256=sha(exe),text_bytes=text_size(exe),checked_exit=0,metric='hosted ELF .text'))
prior=json.loads((ROOT/'student.tests/pa21/performance104.json').read_text())['workloads']
sources={n:prior[n]['source'] for n in ('auto-specializations-9600','runtime-calls','runtime-memory','runtime-floating')}
member=(ROOT/'student.tests/pa22/member-runtime.cpp').read_text().replace('4000000','40000000').replace('26000000','260000000')
sources['runtime-member']=member
sources['member-functions-2048']='struct C{int x;int f(int v)const{return x+v;}};\n'+''.join('int f%d(C&c){int(C::*p)(int)const=&C::f;return (c.*p)(3);}\n'%i for i in range(2048))+'int main(){C c;c.x=4;return f0(c)!=7;}\n'
for name,source in sources.items():
 item=dict(source=source,comparison='entry/final equivalent checked results on common supported subset',outputs=[]);result['workloads'][name]=item
 commands=[];exes=[]
 for i,cc in enumerate((A,B)):
  cmd,exe,data=build(cc,name+'-'+str(i),source);commands.append(cmd);exes.append(exe);item['outputs'].append(data)
 item['compiler']=measure(commands)
 item['runtime']=measure(exes)
 if name.startswith('runtime-'):item['compiler_note']='startup-sensitive small TU; no latency improvement claim'
 if name in ('runtime-member','member-functions-2048'):
  cmd,exe,data=build(C,name+'-proof-off',source)
  item['proof_off']=data;item['proof_b_over_c_compiler']=measure([cmd,commands[1]])
  item['proof_b_over_c_runtime']=measure([exe,exes[1]])
 save();print(name,'complete',flush=True)
assert [sha(p) for p in (A,B,C)]==[x['sha256'] for x in result['binaries']]
save()

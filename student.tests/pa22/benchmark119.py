#!/usr/bin/env python3
"""PA22 constant receiver evidence: frozen common and affected workloads."""
from pathlib import Path
import hashlib,json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
A,B,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:5]];mode=sys.argv[5]
WORK.mkdir(parents=True,exist_ok=True);assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='warmup per lane; four A/A observations; four ABBA blocks, frozen flags and inputs; checked execution separately',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],mode=mode,
 implementation=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),
 binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (A,B)],
 backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),host=run(['g++','--version']).stdout.splitlines()[0],workloads={})
def observe(command):
 usage=WORK/'usage';start=time.perf_counter_ns()
 run(['/usr/bin/time','-f','%M','-o',usage,*command])
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,peak_rss_kib=int(usage.read_text()),checked_exit=0)
def measure(commands):
 warm=[dict(lane=i,**observe(c)) for i,c in enumerate(commands)]
 rows=[dict(lane=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
 data=dict(warmups=warm,observations=rows,aa_range_s=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])])
 data['paired_b_over_a']=[statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==1)/statistics.mean(x['wall_s'] for x in rows[j:j+4] if x['lane']==0) for j in range(4,len(rows),4)]
 for i in (0,1):
  samples=[x for x in rows[4:] if x['lane']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(x['wall_s'] for x in samples),range_wall_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
 return data
prior=json.loads((ROOT/'student.tests/pa22/performance114.json').read_text())
if mode=='common':sources={n:w['source'] for n,w in prior['workloads'].items()}
else:
 sources={}
 flag='template<bool B>struct Flag{static const bool value=B;constexpr operator bool()const{return B;}};template<bool B>const bool Flag<B>::value;\n'
 sources['runtime-conditional']=flag+'volatile long iterations=40000000;int main(){long sum=0;for(long i=0;i<iterations;++i)sum+=Flag<false>()?i*2:i;return sum!=799999980000000L;}\n'
 sources['runtime-member-selection']=flag+'struct C{int x;int good()const{return x;}int bad()const{return x+1;}};typedef int(C::*P)()const;volatile int iterations=40000000;int main(){C c;c.x=7;long sum=0;for(int i=0;i<iterations;++i){P p=Flag<false>()?&C::bad:&C::good;sum+=(c.*p)();}return sum!=280000000;}\n'
 for count in (512,2048):
  sources['receiver-functions-%d'%count]=flag+''.join('int f%d(int v){return Flag<false>()?v+%d:v;}\n'%(i,i) for i in range(count))+'int main(){return f0(7)!=7;}\n'
for name,source in sources.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=sha(src),outputs=[]);commands=[];exes=[];result['workloads'][name]=item
 for i,cc in enumerate((A,B)):
  ir=WORK/(name+str(i)+'.lowir');obj=ir.with_suffix('.o');exe=ir.with_suffix('')
  command=[cc,'--emit-lowir','-O0','-o',ir,src];commands.append(command)
  run(command);plain=sha(ir);stats=run([*command,'--stats','--validate-lowir']);assert sha(ir)==plain
  run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe]);exes.append([exe])
  item['outputs'].append(dict(lowir_sha256=plain,lowir_bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in stats.stderr.splitlines()],text_bytes=text_size(exe),exe_sha256=sha(exe),checked_exit=0))
 item['compiler']=measure(commands);item['runtime']=measure(exes)
 item['identical_lowir']=item['outputs'][0]['lowir_sha256']==item['outputs'][1]['lowir_sha256']
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'complete',flush=True)
assert [sha(p) for p in (A,B)]==[x['sha256'] for x in result['binaries']]

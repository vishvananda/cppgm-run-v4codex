#!/usr/bin/env python3
"""PA23 implementation 125: frozen correct A/B and required new semantic costs."""
from pathlib import Path
import json,os,platform,statistics,sys,time
ROOT=Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
A,B,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:5]];blocks=8 if len(sys.argv)>5 else 4
WORK.mkdir(parents=True,exist_ok=True);assert not OUT.exists()
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='warmup each lane; four A/A observations; %d ABBA blocks; separate checked execution; paired lanes are semantically equivalent and correct; new semantics use a separately marked final-only lane'%blocks,cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],implementation=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (A,B)],backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),host=run(['g++','--version']).stdout.splitlines()[0],workloads={})
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
sources={n:w['source'] for n,w in json.loads((ROOT/'student.tests/pa22/performance114.json').read_text())['workloads'].items()}
sources['runtime-virtual-single']='struct V{virtual int f(){return 1;}};struct D:virtual V{int f(){return 3;}};int call(V&v){return v.f();}volatile int iterations=10000000;int main(){D d;long sum=0;for(int i=0;i<iterations;++i)sum+=call(d);return sum!=30000000;}'
sources['runtime-nonpoly-single']='struct V{int x;};struct B:virtual V{};int read(B&b){return b.x;}volatile int iterations=15000000;int main(){B b;b.x=3;long sum=0;for(int i=0;i<iterations;++i)sum+=read(b);return sum!=45000000;}'
previous=json.loads((ROOT/'student.tests/pa23/performance121-common.json').read_text())['workloads']
previous.update(json.loads((ROOT/'student.tests/pa23/performance121-views.json').read_text())['workloads'])
for name in ('runtime-this-downcast','runtime-secondary','runtime-crosscast'):
 sources[name]=previous[name]['source']
for count in (256,1024,4096):
 source='struct C0{int x;};'+''.join('struct C%d:C%d{};'%(i,i-1) for i in range(1,count+1))
 source+=''.join('C0* get%d(C%d*p){return p;}'%(i,i) for i in range(count,0,-1))
 sources['base-paths-%d'%count]=source+'int main(){C%d c;c.x=7;return get%d(&c)->x!=7;}'%(count,count)
for depth in (4,8,12,16):
 source='struct V{virtual int f(){return 7;}};struct A0:virtual V{};struct B0:virtual V{};'
 for i in range(1,depth+1):source+='struct A%d:virtual A%d,virtual B%d{};struct B%d:virtual A%d,virtual B%d{};'%(i,i-1,i-1,i,i-1,i-1)
 sources['shared-depth-%d'%depth]=source+'int call(A%d&a){return a.f();}int main(){return 0;}'%depth
if len(sys.argv)>5:sources={n:s for n,s in sources.items() if n in ('auto-specializations-9600','base-paths-4096','runtime-this-downcast')}
sources['new-runtime-member-pointer']='struct A{virtual int a(){return 1;}};struct B{virtual int b(){return 2;}};struct D:A,B{int b(){return 7;}};int call(D&d,int(D::*p)()){return (d.*p)();}volatile int iterations=12000000;int main(){D d;int(D::*p)()=&B::b;long sum=0;for(int i=0;i<iterations;++i)sum+=call(d,p);return sum!=84000000;}'
sources['new-runtime-lifecycle']='int live;struct V{V(){++live;}~V(){--live;}virtual int f(){return 3;}};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};volatile int iterations=1500000;int main(){long sum=0;for(int i=0;i<iterations;++i){D d;if(live!=1)return 1;sum+=d.f();}return live!=0||sum!=4500000;}'
sources['new-runtime-value-abi']='struct V{int x;V():x(3){}};struct A:virtual V{};int read(A a){return a.x;}volatile int iterations=12000000;int main(){A a;a.x=7;long sum=0;for(int i=0;i<iterations;++i)sum+=read(a);return sum!=84000000;}'
for depth in (4,8,12):
 source='struct V{int x;V():x(7){}virtual int f(){return x;}};struct A0:virtual V{};struct B0:virtual V{};'
 for i in range(1,depth+1):source+='struct A%d:virtual A%d,virtual B%d{};struct B%d:virtual A%d,virtual B%d{};'%(i,i-1,i-1,i,i-1,i-1)
 sources['new-construct-depth-%d'%depth]=source+'int main(){A%d d;V&v=d;return v.f()!=7;}'%depth
for name,source in sources.items():
 src=WORK/(name+'.cpp');src.write_text(source);final_only=name.startswith(('virtual-declarations-','new-'));compilers=(B,) if final_only else (A,B)
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
 if 'runtime-' in name:item['compiler_note']='small startup-sensitive TU; no latency speed claim'
 else:item['runtime_note']='checked native emission; startup-sensitive, no runtime speed claim'
 if final_only:item['acceptance']='new required semantics/ABI; no comparison against an incorrect or contract-incomplete entry implementation'
 OUT.write_text(json.dumps(result,indent=2)+'\n');print(name,'complete',flush=True)
assert [sha(p) for p in (A,B)]==[x['sha256'] for x in result['binaries']]

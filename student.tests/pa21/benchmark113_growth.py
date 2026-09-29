#!/usr/bin/env python3
"""Frozen PA21/O0 compiler and executable evidence; ENTRY FINAL WORK OUTPUT."""
from pathlib import Path
import hashlib,json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run,sha,text_size
ENTRY,FINAL,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
assert not OUT.exists()
WORK.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each, four A/A observations, four ABBA blocks; separate compiler and runtime timings',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],entry='f3af4630',
 final_parent=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),
 implementation_diff_sha256=hashlib.sha256(run(['git','diff','f3af4630','--','dev']).stdout.encode()).hexdigest(),
 binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (ENTRY,FINAL)],
 host=run(['g++','--version']).stdout.splitlines()[0],object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),
 freestanding_backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
 acceptance='PA21/O0 spec section 9: required PA21 ownership; O(actions + saved operands) typed cleanup and bounded array loops; no new numeric exit gate',
 workloads={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(cmd):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 run(['/usr/bin/time','-f','%e %M %U %S %c %w','-o',usage,*cmd])
 elapsed,rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,command_elapsed_s=float(elapsed),rss_kib=int(rss),user_s=float(user),system_s=float(system),
             involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def measure(commands):
 warm=[dict(binary=i,**observe(c)) for i,c in enumerate(commands)]
 rows=[dict(binary=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
 data=dict(warmups=warm,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])])
 data['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
 for i in (0,1):
  values=[r for r in rows[4:] if r['binary']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in values),wall_range_s=[min(r['wall_s'] for r in values),max(r['wall_s'] for r in values)],peak_rss_kib=max(r['rss_kib'] for r in values))
 return data
def build(cc,name,source,hosted):
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('')
 cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
 stats=run([*cmd,'--stats','--validate-lowir'])
 if hosted:
  obj=src.with_suffix('.o');run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe])
 else:run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
 run([exe])
 data=dict(source_sha256=sha(src),lowir_sha256=sha(ir),lowir_bytes=ir.stat().st_size,
  telemetry=[json.loads(s) for s in stats.stderr.splitlines()],
  native=dict(sha256=sha(exe),text_bytes=text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0,
   metric='ELF .text excluding shared runtime' if hosted else 'sectionless ELF payload proxy including data and EH tables'))
 return cmd,[exe],data
sources={}
for depth in (32,128):
 source='int live;struct M{M(){++live;}~M(){--live;}};'
 for i in range(depth):source+='struct A%d{int n;%s m;};'%(i,'M' if not i else 'A%d'%(i-1))
 source+='A%d make(){return A%d{7};}int main(){{A%d a=make();if(a.n!=7||live!=1)return 1;}return live;}'%(depth-1,depth-1,depth-1)
 sources['nested-helper-'+str(depth)]=source
prefix='int live,attempts,guards;struct G{G(){++guards;}~G(){--guards;}};struct M{M(){if(++attempts==3)throw 7;++live;}~M(){--live;}};'
body='G g;attempts=0;try{M*p=new M[9];delete[]p;}catch(int n){return n!=7||live||guards!=1;}return 1;'
for n in (512,2048):
 source=prefix+''.join('int f%d(){'%i+body+'}' for i in range(n))
 source+='int main(){return f0()||f%d()||guards;}'%(n-1)
 sources['contextual-new-'+str(n)]=source
for name,source in sources.items():
 item=dict(source=source,comparison='final-only correctness cost and growth; entry cannot produce a valid program',outputs=[]);result['workloads'][name]=item
 src=WORK/(name+'-entry.cpp');src.write_text(source)
 before=subprocess.run([ENTRY,'--emit-lowir','-O0','--validate-lowir','-o',src.with_suffix('.lowir'),src],capture_output=True,text=True)
 item['entry_failure']=dict(exit=before.returncode,stderr=before.stderr)
 assert before.returncode!=0,item
 cmd,exe,data=build(FINAL,name+'-final',source,True);item['outputs'].append(data)
 item['compiler_final_AA']=measure([cmd,cmd]);item['runtime_final_AA']=measure([exe,exe])
 save();print(name,'complete',flush=True)
assert [sha(p) for p in (ENTRY,FINAL)]==[b['sha256'] for b in result['binaries']]

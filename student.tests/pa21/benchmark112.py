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
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],entry='9457e2fca6bdb876eaee509b0481f631b13587b4',
 final_parent=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),
 implementation_diff_sha256=hashlib.sha256(run(['git','diff','9457e2fc','--','dev']).stdout.encode()).hexdigest(),
 binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (ENTRY,FINAL)],
 host=run(['g++','--version']).stdout.splitlines()[0],object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),
 freestanding_backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
 acceptance='PA21/O0 spec section 9: required O0 dispatch/region shape; O(1) parent summary per context and one additional pass over checked handlers; no new numeric exit gate',
 workloads={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(cmd):
 usage=WORK/'usage.txt';start=time.perf_counter_ns()
 run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*cmd])
 rss,user,system,involuntary,voluntary=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),
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
prior=json.loads((ROOT/'student.tests/pa21/performance104.json').read_text())['workloads']
sources={name:(prior[name]['source'],False) for name in ('startup','auto-specializations-9600','runtime-calls','runtime-memory','runtime-floating')}
guard='int trace;struct G{int n;G(int x):n(x){}~G(){trace=trace*10+n;}};'
body='try{try{throw 7;}catch(...){G g(1);try{throw 9L;}catch(int){return 2;}}}catch(long n){return n!=9;}return 3;'
for n in (512,2048):
 source=guard+''.join('int f%d(){'%i+body+'}' for i in range(n))
 source+='int main(){return f0()||f%d()||trace!=11;}'%(n-1)
 sources['catch-functions-'+str(n)]=(source,True)
# Runtime is separately timed, with a volatile trip count and checked handler
# and destructor results. The actual throwing work must dominate startup.
sources['runtime-handlers']=(guard+'int f(){'+body+'}int main(){volatile int count=120000;for(int i=0;i<count;++i){trace=0;if(f()||trace!=1)return 1;}return 0;}',True)
for name,(source,hosted) in sources.items():
 item=dict(source=source,comparison='equivalent checked outputs',outputs=[]);result['workloads'][name]=item
 commands=[];exes=[]
 for i,cc in enumerate((ENTRY,FINAL)):
  cmd,exe,data=build(cc,name+'-'+str(i),source,hosted);commands.append(cmd);exes.append(exe);item['outputs'].append(data)
 item['compiler']=measure(commands);item['runtime']=measure(exes)
 for i,data in enumerate(item['outputs']):assert sha(WORK/(name+'-'+str(i)+'.lowir'))==data['lowir_sha256']
 save();print(name,'complete',flush=True)
assert [sha(p) for p in (ENTRY,FINAL)]==[b['sha256'] for b in result['binaries']]

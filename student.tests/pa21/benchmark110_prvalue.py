#!/usr/bin/env python3
"""PA21 prvalue-list helper cost; ENTRY FINAL WORK OUTPUT."""
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
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],entry='d9528e585fd2823d25f9f3a3abaa8f0755f5832a',
 final_parent=run(['git','rev-parse','HEAD']).stdout.strip(),harness_sha256=sha(__file__),
 implementation_diff_sha256=hashlib.sha256(run(['git','diff','d9528e58','--','dev']).stdout.encode()).hexdigest(),
 binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (ENTRY,FINAL)],
 host=run(['g++','--version']).stdout.splitlines()[0],object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),
 freestanding_backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
 acceptance='PA21/O0 spec section 9: required semantics, no new numeric exit gate; expansion at most eight then loops',
 workloads={},new_semantics={})
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
result['prior_campaign']=dict(path='student.tests/pa21/performance110-supplement.json',sha256=sha(ROOT/'student.tests/pa21/performance110-supplement.json'),reason='Aggregate variable initializers stay in initializer_plan; only class prvalue list conversions use the changed helper owner. Require observed helper bodies in final output.')
sources={}
sources['runtime-prvalue-helper']='int live,made;struct M{M(){++live;++made;}~M(){--live;}};struct A{int n;M m;};A make(){return A{1};}int f(int n){A a=make();return a.n+live+(n&3);}int main(){volatile int count=4000000;int sum=0;for(int i=0;i<count;++i)sum+=f(i);return sum!=14000000||made!=4000000||live;}'
for n in (256,1024):
 source='int live;struct M{M(){++live;}~M(){--live;}};struct A{int n;M m;};'
 source+=''.join('A f%d(){return A{1};}'%i for i in range(n))
 source+='int main(){{A a=f0();A b=f%d();if(a.n!=1||b.n!=1||live!=2)return 1;}return live;}'%(n-1)
 sources['prvalue-functions-'+str(n)]=source
for name,source in sources.items():
 item=dict(source=source,comparison='equivalent checked outputs',outputs=[]);result['workloads'][name]=item
 commands=[];exes=[]
 for i,cc in enumerate((ENTRY,FINAL)):
  cmd,exe,data=build(cc,name+'-'+str(i),source,True);commands.append(cmd);exes.append(exe);item['outputs'].append(data)
  data['aggregate_helper_bodies']=(WORK/(name+'-'+str(i)+'.lowir')).read_text().count('function @__aggregate_')
  assert data['aggregate_helper_bodies']>0 if i else data['aggregate_helper_bodies']==0
 item['compiler']=measure(commands);item['runtime']=measure(exes)
 for i,data in enumerate(item['outputs']):assert sha(WORK/(name+'-'+str(i)+'.lowir'))==data['lowir_sha256']
 save();print(name,'complete',flush=True)
assert [sha(p) for p in (ENTRY,FINAL)]==[b['sha256'] for b in result['binaries']]

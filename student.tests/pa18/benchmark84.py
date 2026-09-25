#!/usr/bin/env python3
"""Implementation 84 constructor/value frozen compiler/runtime evidence: A B WORK OUT A_REV [PREFIX]."""
from pathlib import Path
import json,os,platform,statistics,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A samples then four ABBA blocks; six final-only samples for new behavior',
 cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
 commits=[sys.argv[5],shared.run(['git','rev-parse','HEAD']).stdout.strip()],
 harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
 backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
 acceptance='PA18/O0 has no mandated numeric compiler/runtime ceiling. Required constructor-entry propagation, empty-copy legality and O0 scalar widening are semantic/contract repairs. Empty aggregate helpers with no actions are omitted directly under PA11. Each delegation-entry bit propagates at most once per selected edge; there is no optional pass or code clone. Equivalent executed outputs permit cost comparison; incorrect/rejected baseline programs receive final-only costs. Existing initializer action bounds remain unchanged. Native optimization and self-hosting belong to later stages.',
 text_metric='Compiler .text; supplied sectionless ELF payload proxy. Sectionless native payload includes code and any static data; all sources with main receive checked native preflights.',workloads={})
assert not OUT.exists(),'Preserve observations; choose a new output.'
assert not shared.run(['git','diff','HEAD','--','dev']).stdout,'Freeze committed implementation first.'
def observe(command):
 usage=WORK/'usage.txt';start=time.perf_counter_ns();shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
 rss,user,system,iv,v=usage.read_text().split()
 return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)
def measure(commands):
 warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
 order=[0]*4+[0,1,1,0]*4 if len(commands)==2 else [1]*6
 rows=[dict(binary=i,**observe(commands[i])) for i in order];data=dict(warmups=warmups,observations=rows)
 if len(commands)==2:
  aa=[r['wall_s'] for r in rows[:4]];data['aa_range_s']=[min(aa),max(aa)]
  ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
  data['paired_b_over_a']=ratios;data['median_b_over_a']=statistics.median(ratios)
 for i in commands:
  samples=[r for r in rows[(4 if len(commands)==2 else 0):] if r['binary']==i]
  data[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in samples),wall_range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['rss_kib'] for r in samples))
 return data
# Compare affected owners and frozen unchanged template/loop/call/memory/float work.
prior=json.loads((ROOT/'student.tests/pa18/loop82-performance.json').read_text())['workloads']
sources={name:prior[name]['source'] for name in ('ordering-600','ordering-2400','common-loop-float-1500','runtime-calls','runtime-memory','runtime-floating')}
for n in (600,2400):
 head='struct E{};int take(E,int n){return n+1;}'
 sources['empty-aggregate-'+str(n)]=head+''.join('int use'+str(i)+'(){return take(E{},'+str(i)+');}' for i in range(n))+'int main(){return use0()!=1||use'+str(n-1)+'()!='+str(n)+';}'
for n in (150,600):
 head='template<int N>struct B{int n;B():n(N){}B(int):B(){}};template<int N>struct D:B<N>{D():B<N>(1){}};'
 sources['delegation-'+str(n)]=head+''.join('int use'+str(i)+'(){D<'+str(i)+'>d;return d.n;}' for i in range(n))+'int main(){return use0()!=0||use'+str(n-1)+'()!='+str(n-1)+';}'
sources['runtime-empty-aggregate']='struct E{};int take(E,int n){return n+1;}int main(){volatile int n=24000000;int s=0;for(int i=0;i<n;++i)s=(s+take(E{},i&31))&65535;return s!=31488;}'
sources['runtime-delegation']='struct B{int n;B():n(3){}B(int):B(){}};struct D:B{D():B(1){}};int step(int n){D d;return n+d.n;}int main(){volatile int n=12000000;int s=0;for(int i=0;i<n;++i)s=(s+step(i&31))&65535;return s!=29568;}'
sources['empty-lifetime-600']='int hits;struct E{~E(){++hits;}};int take(E){return hits;}'+''.join('int use'+str(i)+'(){E e{};return take(e);}' for i in range(600))+'int main(){int a=use0();int b=use599();return a!=0||b!=2||hits!=4;}'
corpus=[(name,source,False,name.startswith('runtime-') or name.endswith('-runtime')) for name,source in sources.items()]
if len(sys.argv)>6:corpus=[x for x in corpus if x[0].startswith(sys.argv[6])]
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
empty=WORK/'empty.cpp';empty.write_text('int main(){}')
result['startup']=measure({i:[cc,'--emit-lowir','-O0','-o',WORK/f'empty-{i}.lowir',empty] for i,cc in enumerate((A,B))});save()
for name,source,equivalent,runtime in corpus:
 src=WORK/(name+'.cpp');src.write_text(source)
 item=dict(source=source,source_sha256=shared.sha(src),outputs=[]);result['workloads'][name]=item
 commands={};executables={}
 for i,cc in enumerate((A,B)):
  ir=WORK/(name+f'-{i}.lowir');command=[cc,'--emit-lowir','-O0','-o',ir,src]
  r=subprocess.run([str(x) for x in [*command,'--validate-lowir','--stats']],capture_output=True,text=True,timeout=300)
  if r.returncode:
   assert i==0,(name,i,r.stderr)
   item['entry_rejection']=dict(exit=r.returncode,diagnostic=r.stderr);continue
  commands[i]=command
  out=dict(binary=i,sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
  if 'int main(' in source:
   exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
   check=subprocess.run([exe],capture_output=True,timeout=30)
   if check.returncode:
    assert i==0,(name,i,check.returncode)
    item['entry_incorrect_execution']=dict(exit=check.returncode,ir_sha256=shared.sha(ir),native_sha256=shared.sha(exe),telemetry=out['telemetry'])
    del commands[i];item['outputs'].pop();continue
   executables[i]=[exe]
   out['native']=dict(sha256=shared.sha(exe),payload_bytes=shared.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)
 if len(commands)==2:
  same=item['outputs'][0]['sha256']==item['outputs'][1]['sha256']
  item['comparison']='exact' if same else 'executed-equivalent'
  if same and executables:assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256'],name
  if not same:assert executables.keys()=={0,1},name
 else:
  assert 1 in commands,name
  item['comparison']='new-behavior'
 save();print(name,'preflight',flush=True)
 item['compiler']=measure(commands)
 if runtime:item['runtime']=measure(executables)
 save();print(name,'measured',flush=True)
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()

#!/usr/bin/env python3
"""Final audit 90 frozen compiler/runtime evidence: A B WORK OUT A_REV [PREFIX]."""
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
 acceptance='PA18/O0: no mandated numerical latency/RSS ceiling. Nonfinal function/template-list pack deduction and mapped default positions are required semantics; no optional optimization or code growth policy is added. Existing work/depth bounds and canonical type-based class result ABI remain. Incorrect/rejecting entry compilers receive final-only costs. Native host varargs and try/catch belong to later stages.',
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
# Retain audit-82 signature/conversion/summary and audit-89 cumulative inputs.
sources={}
for record in ('loop82-performance.json','loop89-performance-complete.json'):
 for name,item in json.loads((ROOT/'student.tests/pa18'/record).read_text())['workloads'].items():
  if name in sources:assert sources[name]==item['source'],name
  sources[name]=item['source']
# Nonfinal packs consume only explicit lanes; defaults follow concrete positions.
for n in (150,600,2400):
 sources['nonfinal-'+str(n)]='template<int N>struct Tag{};template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}' + ''.join('int u'+str(i)+'(){return f<Tag<'+str(i)+'>,long>(Tag<'+str(i)+'>(),2L);}' for i in range(n)) + 'int main(){return u0()!=9||u'+str(n-1)+'()!=9;}'
 sources['nonfinal-target-'+str(n)]='template<int N>struct Tag{};template<class...T>int f(T...v,int x){return sizeof...(v)+x;}' + ''.join('int u'+str(i)+'(){int(*p)(Tag<'+str(i)+'>,long,int)=f<Tag<'+str(i)+'>,long>;return p(Tag<'+str(i)+'>(),2L,7);}' for i in range(n)) + 'int main(){return u0()!=9||u'+str(n-1)+'()!=9;}'
for n in (150,600,2400):
 sources['nonfinal-list-'+str(n)]='template<int N>struct Tag{};template<class...T>struct L{};template<class...T>int f(L<T...,int>,int x){return sizeof...(T)+x;}' + ''.join('int u'+str(i)+'(){return f<Tag<'+str(i)+'>>(L<Tag<'+str(i)+'>,int>(),7);}' for i in range(n)) + 'int main(){return u0()!=8||u'+str(n-1)+'()!=8;}'
sources['runtime-nonfinal-list']='template<class...T>struct L{};template<class...T>int f(L<T...,long>,int x){return sizeof...(T)+x;}int main(){volatile int n=8000000;int sum=0;for(int i=0;i<n;++i)sum=(sum+f<int>(L<int,long>(),i))&65535;return sum!=2304;}'
sources['runtime-nonfinal']='template<class...T>int f(T...v,int x=7){return sizeof...(v)+x;}int main(){volatile int n=8000000;int sum=0;for(int i=0;i<n;++i)sum=(sum+f<int>(i))&65535;return sum!=36864;}'
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
   exe=WORK/(name+f'-{i}')
   native=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=60)
   if native.returncode:
    assert i==0,(name,i,native.stderr)
    item['entry_backend_rejection']=dict(exit=native.returncode,diagnostic=native.stderr,ir_sha256=shared.sha(ir))
    del commands[i];item['outputs'].pop();continue
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

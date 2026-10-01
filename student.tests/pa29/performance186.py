#!/usr/bin/env python3
"""Audit owner ABBA comparisons and corrected-only default/overflow scaling."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measure(name,mode,label,args,block):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 r['runs'].append(dict(workload=name,mode=mode,label=label,block=block,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]));save()
def image(binary,source,name,label):
 obj=out/(name+label+'.o');exe=out/(name+label)
 run([binary,*r['flags'],source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
 text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
 return exe,dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text)
def summary(name,mode,paired):
 rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];s={}
 for label in ('AB' if paired else 'B'):
  samples=[x for x in rows if x['label']==label and (not paired or x['block'])]
  s[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
 if paired:
  ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==i and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==i and x['label']=='A') for i in range(1,7)]
  aa=[x['wall_s'] for x in rows if not x['block']]
  s.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
 r['summary'].setdefault(name,{})[mode]=s;save()
for i in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'true']);r['launchers'].append(dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text())))
for stage,name in [(183,'guides2400'),(183,'defaults2400'),(184,'runtime2400'),(185,'widths2400')]:
 d=json.loads((root/f'student.tests/pa29/evidence{stage}/owner-performance.json').read_text());v=d['inputs'][name]
 src=out/(name+'.cpp');src.write_text(v['source']);assert sha(src)==v['sha256'];r['inputs'][name]=v
 executables={};images={}
 for label in 'AB':executables[label],images[label]=image(bins[label],src,name,label)
 r['images'][name]=images;assert images['A']['object_sha256']==images['B']['object_sha256'],name
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:measure(name,mode,label,[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [executables[label],'7'],block)
  summary(name,mode,True)
 print(name,json.dumps(r['summary'][name]),flush=True)
iterations=60000;state=7;total=0;overflows=0;mask=(1<<93)-1
for i in range(iterations):
 p=state*1009;overflows+=p>mask;state=p&mask
 p=state+(i&127);overflows+=p>mask;state=p&mask
 total+=state%1009
runtime=f'''using U=unsigned _BitInt(93);
U step(U x,long i,int& over){{U product=0,result=0;over+=__builtin_mul_overflow(x,U(1009),&product);over+=__builtin_add_overflow(product,U(i&127),&result);return result;}}
int main(int argc,char**argv){{U state=argc>1?argv[1][0]-'0':1;long total=0;int over=0;
for(long i=0;i<{iterations};++i){{state=step(state,i,over);total+=long(state%U(1009));}}
return total=={total}&&over=={overflows}?0:1;}}
'''
for n in [600,1200,2400]:
 name='inquiries'+str(n)
 source='template<int N> constexpr int probe(int p,int x=decltype(p)(N%101)){return x;}\n'
 source+=''.join(f'static_assert(probe<{i}>(0)=={i%101} && probe<{i}>(0)=={i%101},"inquiry");\n' for i in range(n))+runtime
 src=out/(name+'.cpp');src.write_text(source)
 reject=run([bins['A'],*r['flags'],src,'-o',out/'rejected.o'],False);assert reject.returncode
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=7,expected_total=total,expected_overflows=overflows,entry_rejection=dict(status=reject.returncode,stderr=reject.stderr))
 exe,im=image(bins['B'],src,name,'B');r['images'][name]={'B':im}
 host=out/(name+'-host');run(['clang++','-std=c++11','-O0',src,'-o',host]);run([host,'7'])
 for mode in ['compile','runtime']:
  for trial in range(8):measure(name,mode,'B',[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7'],trial)
  summary(name,mode,False)
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items());save()

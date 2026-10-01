#!/usr/bin/env python3
"""Corrected-only complex demand scaling; equivalent A/B uses PA27's common suite.
Usage: PERF_CPU=0 performance187.py OUT ENTRY_COMPILER FINAL_COMPILER
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[],host_versions={h:run([h,'--version']).stdout for h in ['g++','clang++']})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measure(name,mode,args,trial):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 r['runs'].append(dict(workload=name,mode=mode,trial=trial,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]));save()
def summary(name,mode):
 rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
 r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
for i in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'true']);r['launchers'].append(dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text())))
iterations=2000000;real=7;imag=0;total_real=0;total_imag=0
# Independent integer model: rotating by i and adding a small real integer is
# exact in double for these bounded values. Every iteration contributes.
for i in range(iterations):
 real,imag=-imag+(i%7-3),real
 total_real+=real;total_imag+=imag
runtime=f'''D step(D x,int i){{return x*__builtin_complex(0.0,1.0)+double(i%7-3);}}
int main(int argc,char**argv){{D x=__builtin_complex(double(argc>1?argv[1][0]-'0':1),0.0);long r=0,s=0;
for(int i=0;i<{iterations};++i){{x=step(x,i);r+=long(__real__ x);s+=long(__imag__ x);}}
return r=={total_real}&&s=={total_imag}&&__real__ x=={real}&&__imag__ x=={imag}?0:1;}}
'''
for n in [600,1200,2400]:
 name='complex'+str(n)
 source='using D=_Complex double;\ntemplate<int N> constexpr D pair(){return __builtin_complex(double(N),double(1-N));}\n'
 source+=''.join(f'static_assert(__real__ pair<{i}>()=={i} && __imag__ pair<{i}>()=={1-i},"pair");\n' for i in range(n))+runtime
 src=out/(name+'.cpp');src.write_text(source)
 reject=run([bins['A'],*r['flags'],src,'-o',out/'rejected.o'],False);assert reject.returncode
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=7,expected=[total_real,total_imag,real,imag],entry_rejection=dict(status=reject.returncode,stderr=reject.stderr))
 obj=out/(name+'.o');exe=out/name
 run([bins['B'],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
 text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
 r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text)
 host=out/(name+'-host');run(['clang++','-std=c++11','-O0',src,'-o',host]);run([host,'7'])
 for mode in ['compile','runtime']:
  for trial in range(8):measure(name,mode,[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7'],trial)
  summary(name,mode)
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items());save()

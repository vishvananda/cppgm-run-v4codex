#!/usr/bin/env python3
"""Corrected-only contextual grammar scaling; A/B common suite is run separately."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measure(name,mode,args,trial,label="B",block=None):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 r['runs'].append(dict(workload=name,mode=mode,trial=trial,label=label,block=block,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]));save()
for i in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'true']);r['launchers'].append(dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text())))
iterations=3000000;seed=7;state=seed;total=0
for i in range(iterations):
 state=(state*17+(i&127))%1009;total=(total+state)%65521
runtime=f'''int co_await(int x,int i){{return (x*17+(i&127))%1009;}}
int co_yield(int x){{return x;}}
int co_return(int x){{return x;}}
int main(int argc,char**argv){{int state=argc>1?argv[1][0]-'0':1,total=0;
for(int i=0;i<{iterations};++i){{state=co_await(state,i);total=(total+co_yield(co_return(state)))%65521;}}
return total=={total}&&state=={state}?0:1;}}
'''
# Previously correct ordinary identifiers exercise the class-name index owner.
for n in [600,1200,2400]:
 name='names'+str(n)
 source=''.join(f'struct R{i}{{int value()const{{return co_await+co_yield+co_return;}} int spare,co_await; enum{{a,co_yield=3}}; int(co_return);}};\n' for i in range(n))+runtime
 src=out/(name+'.cpp');src.write_text(source)
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=seed,expected=[state,total])
 images={};executables={}
 for label in 'AB':
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
  text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text);executables[label]=exe
 assert images['A']==images['B'];r['images'][name]=images
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:measure(name,mode,[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [executables[label],'7'],block,label,block)
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];summary={}
  for label in 'AB':
   v=[x for x in rows if x['label']==label and x['block']]
   summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in v),range_s=[min(x['wall_s'] for x in v),max(x['wall_s'] for x in v)],peak_rss_kib=max(x['peak_rss_kib'] for x in v))
  ratios=[statistics.mean(x['wall_s'] for x in rows if x['label']=='B' and x['block']==i)/statistics.mean(x['wall_s'] for x in rows if x['label']=='A' and x['block']==i) for i in range(1,7)]
  aa=[x['wall_s'] for x in rows if not x['block']]
  summary.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
  r['summary'].setdefault(name,{})[mode]=summary;save()
 print(name,json.dumps(r['summary'][name]),flush=True)
for n in [600,1200,2400]:
 name='contextual'+str(n)
 source=''.join(f'template<class T> void inert{i}(T t){{auto a=co_await t.get(); auto b=(co_yield t=t,t); co_yield {{a,b}}; co_return b;}}\n' for i in range(n))+runtime
 src=out/(name+'.cpp');src.write_text(source)
 reject=run([bins['A'],*r['flags'],src,'-o',out/'reject.o'],False);assert reject.returncode
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=seed,expected=[state,total],entry_rejection=dict(status=reject.returncode,stderr=reject.stderr))
 obj=out/(name+'.o');exe=out/name
 run([bins['B'],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
 text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
 r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text)
 # Standard C++11 identifier behavior in the executable is independently checked.
 hostsrc=out/'runtime.cpp';hostsrc.write_text(runtime);host=out/'host'
 run(['g++','-std=c++11','-O0',hostsrc,'-o',host]);run([host,'7'])
 for mode in ['compile','runtime']:
  for trial in range(8):measure(name,mode,[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7'],trial)
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
  r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items());save()

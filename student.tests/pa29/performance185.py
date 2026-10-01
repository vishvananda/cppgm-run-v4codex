#!/usr/bin/env python3
"""Final-only bit-integer demand scaling; entry rejection is not a timing baseline."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 if ok:assert not p.returncode,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for k in range(8):
 started=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'true'])
 r['launchers'].append(dict(wall_s=time.perf_counter()-started,peak_rss_kib=int((out/'rss').read_text())))
iterations=600000;state=7;total=0
for i in range(iterations):
 state=(state*1009+(i&127))&((1<<93)-1)
 total+=state%1009
runtime=f'''using U=unsigned _BitInt(93);
U step(U x,long i){{return x*U(1009)+U(i&127);}}
int main(int argc,char**argv){{U state=argc>1?argv[1][0]-'0':1;long total=0;
for(long i=0;i<{iterations};++i){{state=step(state,i);total+=long(state%U(1009));}}
return total=={total}?0:1;}}
'''
for n in [600,1200,2400]:
 name='widths'+str(n);source='template<int N> constexpr unsigned _BitInt(N%126+3) probe(){return static_cast<unsigned _BitInt(N%126+3)>(-1)+static_cast<unsigned _BitInt(N%126+3)>(2);}\n'
 source+=''.join(f'static_assert(probe<{j}>()==1 && probe<{j}>()==1,"width");\n' for j in range(n))+runtime
 src=out/(name+'.cpp');src.write_text(source)
 p=run([bins['A'],*r['flags'],src,'-o',out/'reject.o'],False);assert p.returncode
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=7,expected=total,entry_rejection=dict(status=p.returncode,stderr=p.stderr))
 obj=out/(name+'.o');exe=out/name
 run([bins['B'],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
 host=out/(name+'-host');run(['clang++','-std=c++11','-O0',src,'-o',host]);run([host,'7'])
 text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
 r['images'][name]=dict(text_bytes=text,object_sha256=sha(obj),executable_sha256=sha(exe))
 for mode in ['compile','runtime']:
  for trial in range(8):
   args=[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7']
   started=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-started
   r['runs'].append(dict(workload=name,mode=mode,trial=trial,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
   save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
  r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

#!/usr/bin/env python3
"""PA29 guide/default scaling; final-only for sources rejected by entry."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert not p.returncode,(args,p.returncode,p.stderr)
 return p
r=dict(compiler=dict(path=str(cc),sha256=sha(cc)),flags=['-O0','-c','--stats'],runs=[],inputs={},images={},summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for i in range(8):
 start=time.perf_counter();run(['/bin/true']);r['launchers'].append(time.perf_counter()-start)
expected=0;state=7
for i in range(12000000):state=(state*17+(i&127))%1009;expected+=state
for family in ['guides','defaults']:
 for n in [600,1200,2400]:
  name=f'{family}{n}';src=out/(name+'.cpp')
  if family=='guides':
   source='template<int I> struct tag {}; template<class T> struct box {};\n'
   source+=''.join(f'template<class T> explicit(sizeof(T)>8) box(tag<{i}>,T value) noexcept(noexcept(value+value))->box<T>;\n' for i in range(n))
   source+='long probe(long seed){return seed;}\n';check=7
  else:
   source='template<int I> long item(long x,int width=sizeof(x)){return (x+I+width)%97;}\nlong probe(long x){long s=0;\n'
   source+=''.join(f's+=item<{i}>(x);\n' for i in range(n))+'return s;}\n';check=sum((7+i+8)%97 for i in range(n))
  source+='long step(long x,long i){return (x*17+(i&127))%1009;}\n'
  source+=f'''int main(int argc,char**argv){{long seed=argc>1?argv[1][0]-'0':1;
if(probe(seed)!={check})return 2;long state=seed,total=0;
for(long i=0;i<12000000;++i){{state=step(state,i);total+=state;}}
return total=={expected}?0:1;}}\n'''
  src.write_text(source);r['inputs'][name]=dict(sha256=sha(src),source=source,N=n,family=family,seed=7,iterations=12000000,expected=expected,probe=check)
  obj=out/(name+'.o');exe=out/name;host=out/(name+'-host')
  run([cc,*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
  run(['clang++','-std=c++20','-O0',src,'-o',host]);run([host,'7'])
  text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
  r['images'][name]=dict(text_bytes=text,object_sha256=sha(obj),executable_sha256=sha(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*r['flags'],src,'-o',out/'timed.o'] if mode=='compile' else [exe,'7']
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,sample=sample,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')]))
    save()
   rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
   r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
  print(name,r['summary'][name],flush=True)
assert sha(cc)==r['compiler']['sha256']
assert len({r['images']['guides'+str(n)]['text_bytes'] for n in [600,1200,2400]})==1

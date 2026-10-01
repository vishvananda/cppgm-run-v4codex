#!/usr/bin/env python3
"""PA29 required layout/body validation cost and scaling, checked native output."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve();entry=pathlib.Path(sys.argv[3]).resolve()
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 if not allow:assert not p.returncode,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(v),sha256=sha(v),bytes=v.stat().st_size) for k,v in [('entry',entry),('final',cc)]},flags=['-std=c++14','-O0','-c','--stats'],affinity=affinity,inputs={},images={},entry_checks={},runs=[],summary={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);r['launcher'].append(time.perf_counter()-start)
iterations=20000000
for kind in ['layout','wrappers']:
 for n in [600,1200,2400]:
  key=kind+str(n);src=out/(key+'.cpp')
  if kind=='layout':
   prefix='''template<class T,unsigned N> using Vec __attribute__((ext_vector_type(N))) = T;
template<int N> struct Shape {typedef float V __attribute__((vector_size(16)));char tag;V values;};
template<int N> long work(long x){
static_assert(sizeof(typename Shape<N>::V)==16,"lane width");
static_assert(sizeof(Shape<N>)==32,"member layout");
static_assert(sizeof(Vec<bool,9>)==2,"packed mask");
return x+sizeof(Shape<N>)+sizeof(Vec<bool,9>)+(N&7);}
'''
   demanded=sum(35+(i&7) for i in range(n));period=[i+42 for i in range(128)]
  else:
   prefix='''typedef int V __attribute__((vector_size(16)));
inline V unused_literal(){return V{1,2,3,4};}
inline void unavailable(){__builtin_unknown_instruction_167();}
inline void unused_call(){unavailable();}
inline long leaf(long x){return x+7;}
inline long branch(long x){return leaf(x);}
template<int N> long work(long x){return branch(x)+(N&7);}
'''+''.join('inline int dormant%d(int x){return x+%d;}\n'%(i,i) for i in range(n))
   demanded=sum(8+(i&7) for i in range(n));period=[i+15 for i in range(128)]
  checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
  src.write_text(prefix+'long demanded(long x){long sum=0;\n'+''.join('sum+=work<%d>(x);\n'%i for i in range(n))+'return sum;}\nint main(int argc,char**){if(demanded(argc)!='+str(demanded)+')return 1;\nlong sum=0;for(int i=0;i<argc*20000000;++i)sum+=work<7>((i&127)+argc);\nreturn sum=='+str(checksum)+'LL?0:2;}\n')
  obj=out/(key+'.o');exe=out/key
  old=run([entry,*r['flags'],src,'-o',out/'entry.o'],True)
  r['entry_checks'][key]=dict(status=old.returncode,stderr=old.stderr.decode())
  run([cc,*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  r['inputs'][key]=dict(path=str(src),sha256=sha(src));r['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    r['runs'].append(dict(workload=key,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[s for s in r['runs'] if s['workload']==key and s['mode']==mode]
   r['summary'].setdefault(key,{})[mode]=dict(median_s=statistics.median(s['wall_s'] for s in rows),range_s=[min(s['wall_s'] for s in rows),max(s['wall_s'] for s in rows)],peak_rss_kib=max(s['peak_rss_kib'] for s in rows));save()
  print(key,json.dumps(r['summary'][key]),flush=True)
assert sha(cc)==r['binaries']['final']['sha256'] and sha(entry)==r['binaries']['entry']['sha256'];save()

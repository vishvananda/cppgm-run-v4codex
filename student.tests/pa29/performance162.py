#!/usr/bin/env python3
"""Corrected atomic ownership costs: frozen B, checked runtime and demand scaling.

The entry compiler fails the reduced correctness controls; its affected runtime
is deliberately not treated as an equivalent implementation. Common correct
A/A+ABBA comparisons use performance147_common.py alongside this experiment.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve()
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 assert not p.returncode,(args,p.returncode,p.stderr.decode());return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binary=dict(path=str(cc),sha256=sha(cc)),flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
iterations=2000000
for kind in ['snapshot','unaligned']:
 for n in [600,1200,2400]:
  key=kind+str(n);src=out/(key+'.cpp')
  if kind=='snapshot':
   prefix='''struct Reader {_Atomic(int)* p;int read(const int& x){*p=7;return x;}};
struct Pointer {Reader* p;Reader& operator*() const{return *p;}};
template<int N> struct Cell {int value;};
template<int N> long work(long x){
static_assert(__has_trivial_copy(Cell<N>),"shared class property");
static_assert(__reference_constructs_from_temporary(const int&,_Atomic(int)&),"snapshot");
_Atomic(int) a=int(x);_Atomic(bool) flag=false;flag+=N&1;
Reader r={&a};const int& snapshot=a;
int value=__builtin_invoke(&Reader::read,Pointer{&r},snapshot);
return value+(flag?1:0)+a;}
'''
   demanded=n*8+n//2;period=[i+9 for i in range(128)]
  else:
   prefix='''typedef __int128 Packed __attribute__((aligned(1)));
struct Holder {char guard;Packed value;};
template<int N> long work(long x){alignas(16) Holder h;Packed* p=&h.value;
__atomic_store_n(p,x,5);return (long)__atomic_add_fetch(p,N&7,5);}
'''
   demanded=sum(1+(i&7) for i in range(n));period=[i+8 for i in range(128)]
  checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
  src.write_text(prefix+'long demanded(long x){long sum=0;\n'+''.join('sum+=work<%d>(x);\n'%i for i in range(n))+'return sum;}\nint main(int argc,char**){if(demanded(argc)!='+str(demanded)+')return 1;\nlong sum=0;for(int i=0;i<argc*2000000;++i)sum+=work<7>((i&127)+argc);\nreturn sum=='+str(checksum)+'LL?0:2;}\n')
  obj=out/(key+'.o');exe=out/key
  run([cc,*result['flags'],src,'-o',obj]);run(['g++',obj,'-latomic','-o',exe]);run([exe])
  result['inputs'][key]=dict(path=str(src),sha256=sha(src))
  result['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    result['runs'].append(dict(workload=key,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[r for r in result['runs'] if r['workload']==key and r['mode']==mode]
   result['summary'].setdefault(key,{})[mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
  print(key,json.dumps(result['summary'][key]),flush=True)
assert sha(cc)==result['binary']['sha256'];save()

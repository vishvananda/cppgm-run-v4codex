#!/usr/bin/env python3
"""Pack selection/template identity and linear sequence-generation capability costs."""
import hashlib,json,os,pathlib,platform,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry,cc=[pathlib.Path(p).resolve() for p in sys.argv[2:4]]
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 if not allow:assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for k,p in [('A',entry),('B',cc)]},flags=flags,affinity=affinity,platform=platform.platform(),script_sha256=sha(pathlib.Path(__file__)),inputs={},images={},runs=[],summary={},entry_checks={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);r['launcher'].append(time.perf_counter()-start)
iterations=20000000
scale=int(sys.argv[4]) if len(sys.argv)>4 else 1
for kind in (['sequence','integer-pack'] if scale!=1 else ['selection','sequence','integer-pack']):
 for n in [600*scale,1200*scale,2400*scale]:
  name=kind+str(n);src=out/(name+'.cpp')
  if kind=='selection':
   prefix='''template<int N> struct Cell {long value;};
template<template<unsigned long,class...> class Select,int N>
long work(long x){typedef Select<0,Cell<N>,long> Type;Type cell={x};return cell.value+(N&7);}
long demanded(long x){long sum=0;
'''+''.join('sum+=work<__type_pack_element,%d>(x);\n'%i for i in range(n))+'return sum;}\n'
   period=[i+8 for i in range(128)];checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
   body='if(demanded(argc)!='+str(sum(1+(i&7) for i in range(n)))+')return 1;long sum=0;for(int i=0;i<argc*20000000;++i)sum+=work<__type_pack_element,7>((i&127)+argc);return sum=='+str(checksum)+'LL?0:2;'
  else:
   prefix='template<class T,T... I> struct seq {};\n'
   if kind=='sequence':prefix+='typedef __make_integer_seq<seq,unsigned,'+str(n)+'> Big;\n'
   else:prefix+='template<template<class,int...>class S,int N>using Make=S<int,__integer_pack(N)...>;\ntypedef Make<seq,'+str(n)+'> Big;\n'
   prefix+='template<class T,T...I>long work(seq<T,I...>,unsigned x){static const T values[]={I...};return values[x%sizeof...(I)];}\n'
   checksum=n*(n-1)//2*(iterations//n)+(iterations%n)*((iterations%n)-1)//2
   body='long sum=0;for(int i=0;i<argc*20000000;++i)sum+=work(Big{},i);return sum=='+str(checksum)+'LL?0:2;'
  src.write_text(prefix+'int main(int argc,char**){'+body+'}\n')
  r['inputs'][name]=dict(path=str(src),sha256=sha(src))
  old=run([entry,*flags,src,'-o',out/'entry.o'],True);assert old.returncode!=0
  r['entry_checks'][name]=dict(status=old.returncode,stderr=old.stderr.decode())
  obj=out/(name+'.o');exe=out/name
  run([cc,*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*flags,src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[s for s in r['runs'] if s['workload']==name and s['mode']==mode]
   r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(s['wall_s'] for s in rows),range_s=[min(s['wall_s'] for s in rows),max(s['wall_s'] for s in rows)],peak_rss_kib=max(s['peak_rss_kib'] for s in rows));save()
  print(name,json.dumps(r['summary'][name]),flush=True)
assert sha(cc)==r['binaries']['B']['sha256'] and sha(entry)==r['binaries']['A']['sha256'];save()

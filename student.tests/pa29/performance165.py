#!/usr/bin/env python3
"""Required evaluation/storage cost and scaling; no entry/invalid speedup claim."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True);cc=pathlib.Path(sys.argv[2]).resolve()
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90);assert p.returncode==0,(args,p.returncode,p.stderr.decode());return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binary=dict(path=str(cc),sha256=sha(cc)),flags=['-std=c++14','-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
iterations=20000000
for kind in ['evaluation','storage']:
 for n in [600,1200,2400]:
  key=kind+str(n);src=out/(key+'.cpp')
  prefix='constexpr bool active(){return __builtin_is_constant_evaluated();}\n'
  if kind=='evaluation':
   prefix+='template<int N> constexpr long work(long x){return active()?x+(N&7):x-(N&7);}\n'
   body=''.join('constexpr long k%d=work<%d>(%d);sum+=k%d+work<%d>(x);\n'%(i,i,i,i,i) for i in range(n))
   demanded=n*(n+1)//2;period=[i+1-7 for i in range(128)]
  else:
   prefix+='''constexpr long choose(long x){return active()?x+3:x-2;}
template<int N>struct Box{long n;constexpr Box():n(active()?N:0){}};
template<int N>constexpr long work(long x){constexpr Box<N> box;const long& r=choose(N);
bool a[]={active()};return x+box.n+r+(a[0]?1000000:0);}
'''
   body=''.join('constexpr long k%d=work<%d>(1);sum+=k%d+work<%d>(x);\n'%(i,i,i,i) for i in range(n))
   demanded=1000000*n+2*n*n+6*n;period=[i+18 for i in range(128)]
  checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
  src.write_text(prefix+'long demanded(long x){long sum=0;\n'+body+'return sum;}\nint main(int argc,char**){if(demanded(argc)!='+str(demanded)+')return 1;\nlong sum=0;for(int i=0;i<argc*20000000;++i)sum+=work<7>((i&127)+argc);\nreturn sum=='+str(checksum)+'LL?0:2;}\n')
  obj=out/(key+'.o');exe=out/key;run([cc,*result['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  result['inputs'][key]=dict(path=str(src),sha256=sha(src));result['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    result['runs'].append(dict(workload=key,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[r for r in result['runs'] if r['workload']==key and r['mode']==mode]
   result['summary'].setdefault(key,{})[mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
  print(key,json.dumps(result['summary'][key]),flush=True)
assert sha(cc)==result['binary']['sha256'];save()

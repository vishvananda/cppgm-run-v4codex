#!/usr/bin/env python3
"""PA29 new capability cost/scaling. Failing baseline is never a speed baseline."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
a,b=(pathlib.Path(p).resolve() for p in sys.argv[2:4]);affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
 assert (p.returncode==0)==ok,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in [('A',a),('B',b)]},policy='Standalone costs for new semantics, no A/B speedup claim.',flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},unsupported_A={},launcher=[])
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
def measure(label,args,**fields):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 row=dict(workload=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')],**fields)
 result['runs'].append(row);save();return row
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
for n in [600,1200,2400]:
 src=out/('traits'+str(n)+'.cpp')
 src.write_text('''template<int N> struct Cell {long value;};
template<int N> long item(long x) {
 using T = __remove_reference_t(Cell<N>&);
 static_assert(__is_object(T) && __is_trivially_destructible(T), "shape and destruction");
 static_assert(__is_convertible(int,long) && __array_rank(T[2][3])==2, "conversion and rank");
 return x+N;
}
long demanded(long x){long sum=0;
'''+''.join('sum+=item<%d>(x);\n'%i for i in range(n))+'''return sum;}
int main(int argc,char**){
 if(demanded(argc)!='''+str(n*(n+1)//2)+''')return 1;
 long sum=0;for(int i=0;i<argc*60000000;++i)sum+=item<7>(i&127);
 return sum==4230000000L?0:2;
}
''')
 key='traits'+str(n);result['inputs'][key]=dict(path=str(src),sha256=sha(src));obj=out/(key+'.o');exe=out/key
 p=run([a,*result['flags'],src,'-o',out/'bad.o'],False);result['unsupported_A'][key]=dict(status=p.returncode,stderr=p.stderr.decode())
 run([b,*result['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 result['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
 for mode in ['compile','runtime']:
  for i in range(8):measure(key,[b,*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe],mode=mode,sample=i)
pp=out/'probes.cpp'
pp.write_text('#define DECL(N) struct C##N {int value;};\n'+''.join('''#if __has_builtin(__remove_reference_t) && __has_feature(cxx_constexpr)
DECL(%d)
#else
#error missing support
#endif
'''%i for i in range(10000)))
result['inputs']['preprocess']=dict(path=str(pp),sha256=sha(pp))
p=run([a,'-E',pp,'-o',out/'bad.pp'],False);result['unsupported_A']['preprocess']=dict(status=p.returncode,stderr=p.stderr.decode())
for i in range(8):
 measure('preprocess',[b,'-E','--stats',pp,'-o',out/'probes.pp'],mode='preprocess',sample=i)
 data=(out/'probes.pp').read_text();assert data.count('simple struct KW_STRUCT')==10000 and 'identifier C9999\n' in data
result['preprocessed_sha256']=sha(out/'probes.pp')
for workload in result['inputs']:
 result['summary'][workload]={}
 for mode in {r['mode'] for r in result['runs'] if r['workload']==workload}:
  rows=[r for r in result['runs'] if r['workload']==workload and r['mode']==mode]
  result['summary'][workload][mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows))
assert sha(a)==result['binaries']['A']['sha256'] and sha(b)==result['binaries']['B']['sha256'];save()
print(json.dumps(result['summary']),flush=True)

#!/usr/bin/env python3
"""Frozen same-source projected-storage A/A and six ABBA block comparisons."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ.get('PERF_CPU','0')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
 assert p.returncode==0,(list(map(str,args)),p.returncode,p.stderr.decode())
 return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.decode().splitlines() if l.split() and l.split()[0].startswith('.text'))
# Identical source in each pair: entry already implements projected storage.
# Two defaults in one nested receiver exercise reuse of the same path.
head='''struct{int a;struct{int b;int c=this->b+1;int d=this->c+1;};};'''
runtime='template<int N> struct Value{long prefix;'+head+'''Value(int x):prefix(N),a(x),b(a+1){} long total()const{return prefix+a+b+c+d;}};
template<int N> __attribute__((noinline)) long item(int x){Value<N> v(x);return v.total();}
long demand(int x){long sum=0;'''+''.join('sum+=item<%d>(x);'%n for n in range(600))+'return sum;}\n'
expected=sum(sum(n+4*(i&63)+6 for n in range(600)) for i in range(12000))%1009
runtime+='int main(int argc,char**){long sum=0;for(int i=0;i<argc*12000;++i)sum=(sum+demand(i&63))%%1009;return sum==%d?0:1;}\n'%expected
constant='struct Value{long prefix;'+head+'''constexpr Value(int x):prefix(5),a(x),b(a+1){} constexpr long total()const{return prefix+a+b+c+d;}};\n'''
constant+=''.join('constexpr Value v%d(%d);\n'%(n,n) for n in range(8192))
constant+='static const long data[8192]={'+','.join('v%d.total()'%n for n in range(8192))+'};\n'
expected_constant=sum(4*(i%8192)+11 for i in range(5000000))%1009
constant+='int main(int argc,char**){long sum=0;for(int i=0;i<argc*5000000;++i)sum=(sum+data[i&8191])%%1009;return sum==%d?0:1;}\n'%expected_constant
sources={}
for name,body in [('construction',runtime),('constant',constant)]:
 p=out/(name+'.cpp');p.write_text(body);sources[name]=p
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in binaries.items()},inputs={k:sha(p) for k,p in sources.items()},
 flags=['-O0','-c','--stats'],affinity=affinity,expected=dict(construction=expected,constant=expected_constant),
 host_linker=run(['g++','--version']).stdout.decode(),runs=[],images={},summary={})
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for name,source in sources.items():
 images={}
 for label,binary in binaries.items():
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([binary,*result['flags'],source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_bytes=exe.stat().st_size,object_text_bytes=size(obj),executable_text_bytes=size(exe))
 result['images'][name]=images
 for mode in ('compile','runtime'):
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:
    args=[binaries[label],*result['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label)]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    result['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.decode().splitlines() if l.startswith('{')]))
    save()
  rows=[r for r in result['runs'] if r['workload']==name and r['mode']==mode]
  ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
  aa=[r['wall_s'] for r in rows if not r['block']]
  summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in 'AB':
   samples=[r for r in rows if r['block'] and r['label']==label]
   summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
  result['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())

#!/usr/bin/env python3
"""Constructor costs: frozen before/after on named storage, then equivalent
named/projected programs on the final compiler. Each pair uses A/A and six ABBA
blocks. Projected input is new behavior, not a valid entry-compiler benchmark.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry,final=[pathlib.Path(p).resolve() for p in sys.argv[2:4]]
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
 assert p.returncode==0,(list(map(str,args)),p.returncode,p.stderr.decode())
 return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.decode().splitlines() if l.split() and l.split()[0].startswith('.text'))
count=600;iterations=12000
expected=sum(sum(2*(i&63)+n+5 for n in range(count)) for i in range(iterations))%1009
named='''template<int N> struct Value{long prefix;
struct Storage{int a;int b=this->a+N;Storage(int x):a(x){}} state;
Value(int x):prefix(5),state(x){} long total()const{return prefix+state.a+state.b;}};
'''
projected='''template<int N> struct Value{long prefix;
struct{int a;int b=this->a+N;};
Value(int x):prefix(5),a(x){} long total()const{return prefix+a+b;}};
'''
tail='''template<int N> __attribute__((noinline)) long item(int x){Value<N> v(x);return v.total();}
long demand(int x){long sum=0;
'''+''.join('sum+=item<%d>(x);\n'%n for n in range(count))+'''return sum;}
int main(int argc,char**){long sum=0;for(int i=0;i<argc*12000;++i)sum=(sum+demand(i&63))%1009;
return sum=='''+str(expected)+'?0:1;}\n'
mode=sys.argv[4] if len(sys.argv)>4 else 'runtime-construction'
if mode=='constant-construction':
 count=8192;iterations=5000000
 expected=sum(2*(i%count)+6 for i in range(iterations))%1009
 named='struct Value{long prefix;struct Storage{int a;int b;constexpr Storage(int x):a(x),b(x+1){}} state;\nconstexpr Value(int x):prefix(5),state(x){} constexpr long total()const{return prefix+state.a+state.b;}};\n'
 projected='struct Value{long prefix;struct{int a;struct{int b;};};\nconstexpr Value(int x):prefix(5),a(x),b(x+1){} constexpr long total()const{return prefix+a+b;}};\n'
 tail=''.join('constexpr Value v%d(%d);\n'%(n,n) for n in range(count))
 tail+='static const long data[8192]={'+','.join('v%d.total()'%n for n in range(count))+'};\n'
 tail+='int main(int argc,char**){long sum=0;for(int i=0;i<argc*5000000;++i)sum=(sum+data[i&8191])%%1009;return sum==%d?0:1;}\n'%expected
elif mode!='runtime-construction':raise ValueError(mode)
sources={}
for label,head in [('named',named),('projected',projected)]:
 p=out/(label+'.cpp');p.write_text(head+tail);sources[label]=p
variants={'A':(entry,sources['named']),'B':(final,sources['named']),'P':(final,sources['projected'])}
flags=['-O0','-c','--stats']
result=dict(workload_mode=mode,affinity=affinity,variants={k:dict(binary=str(b),binary_sha256=sha(b),input_sha256=sha(s),input_bytes=s.stat().st_size) for k,(b,s) in variants.items()},flags=flags,expected=expected,images={},runs=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for label,(binary,source) in variants.items():
 obj=out/(label+'.o');exe=out/label
 run([binary,*flags,source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 result['images'][label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_bytes=exe.stat().st_size,object_text_bytes=size(obj),executable_text_bytes=size(exe))
save()
for pair in ('AB','BP'):
 a,b=pair
 for mode in ('compile','runtime'):
  for block,order in enumerate([a*4]+[a+b+b+a]*6):
   for label in order:
    binary,source=variants[label]
    args=[binary,*flags,source,'-o',out/'measured.o'] if mode=='compile' else [out/label]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    result['runs'].append(dict(pair=pair,mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.decode().splitlines() if l.startswith('{')]))
    save()
  rows=[r for r in result['runs'] if r['pair']==pair and r['mode']==mode]
  ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==i and r['label']==b)/statistics.mean(r['wall_s'] for r in rows if r['block']==i and r['label']==a) for i in range(1,7)]
  aa=[r['wall_s'] for r in rows if not r['block']]
  summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in pair:
   samples=[r for r in rows if r['block'] and r['label']==label]
   summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
  result['summary'].setdefault(pair,{})[mode]=summary;save();print(pair,mode,json.dumps(summary),flush=True)
assert all(sha(variants[k][0])==v['binary_sha256'] and sha(variants[k][1])==v['input_sha256'] for k,v in result['variants'].items())

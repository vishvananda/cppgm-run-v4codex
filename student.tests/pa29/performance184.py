#!/usr/bin/env python3
"""Frozen explicit-cast owner A/A + ABBA and new constexpr-owner scaling."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for i in range(8):
 start=time.perf_counter();run(['/bin/true']);r['launchers'].append(time.perf_counter()-start)
iterations=6000000;expected=0;state=7
for i in range(iterations):state=(state*17+(i&127))%1009;expected+=state
for name,n in [('runtime2400',2400),('constant600',600),('constant1200',1200),('constant2400',2400)]:
 common=name.startswith('runtime');src=out/(name+'.cpp')
 if common:
  source='template<int N> long item(const long *p){return *(long*)p+N;}\nlong probe(long x){long s=0;\n'
  source+=''.join(f's+=item<{i}>(&x);\n' for i in range(n))+'return s;}\n'
  probe=7*n+n*(n-1)//2
 else:
  source='''struct L {int pad; constexpr L():pad(3){}};
struct B {int value;constexpr B():value(11){}};
struct D:L,B {constexpr D():L(),B(){}};
constexpr D object;
constexpr const int B::*member=&B::value;
template<int N> constexpr int read(const D& d){return ((B&)d).value+d.*(int D::*)member+N;}
'''
  source+=''.join(f'static_assert(read<{i}>(object)=={22+i} && read<{i}>(object)=={22+i},"identity");\n' for i in range(n))
  source+='long probe(long x){return x;}\n';probe=7
 source+='long step(long x,long i){const long *p=&x;return (*(long*)p*17+(i&127))%1009;}\n'
 source+=f'''int main(int argc,char**argv){{long seed=argc>1?argv[1][0]-'0':1;
if(probe(seed)!={probe})return 2;long state=seed,total=0;
for(long i=0;i<{iterations};++i){{state=step(state,i);total+=state;}}
return total=={expected}?0:1;}}\n'''
 src.write_text(source);r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,seed=7,iterations=iterations,expected=expected,probe=probe)
 labels='AB' if common else 'B';exes={};r['images'][name]={}
 if not common:
  p=run([bins['A'],*r['flags'],src,'-o',out/'entry-reject.o'],False)
  assert p.returncode
  r['inputs'][name]['entry_rejection']=dict(status=p.returncode,stderr=p.stderr)
 for k in labels:
  obj=out/(name+k+'.o');exe=out/(name+k)
  run([bins[k],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);exes[k]=exe
  text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
  r['images'][name][k]=dict(text_bytes=text,object_sha256=sha(obj),executable_sha256=sha(exe))
 host=out/(name+'-host');run(['clang++','-std=c++11','-O0',src,'-o',host]);run([host,'7'])
 for mode in ['compile','runtime']:
  orders=['AAAA']+['ABBA']*6 if common else ['B']*8
  for block,order in enumerate(orders):
   for k in order:
    args=[bins[k],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exes[k],'7']
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);wall=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,block=block,label=k,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
    save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];summary={}
  for k in labels:
   values=[x for x in rows if x['label']==k and (x['block'] or not common)]
   summary[k]=dict(median_s=statistics.median(x['wall_s'] for x in values),range_s=[min(x['wall_s'] for x in values),max(x['wall_s'] for x in values)],peak_rss_kib=max(x['peak_rss_kib'] for x in values))
  if common:
   aa=[x['wall_s'] for x in rows if not x['block']]
   ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
   summary.update(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  r['summary'].setdefault(name,{})[mode]=summary;save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

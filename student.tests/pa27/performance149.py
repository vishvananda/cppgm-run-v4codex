#!/usr/bin/env python3
"""Affected signature A/A + six ABBA blocks, and newly supported hosted cost.
Usage: performance149.py OUT ENTRY_COMPILER FINAL_COMPILER
The hosted input has no correct entry implementation: report final cost only.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
here=pathlib.Path(__file__).resolve().parent
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ.get('PERF_CPU','0')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,data=None,okay=True):
 p=subprocess.run(list(map(str,args)),input=data,capture_output=True,timeout=120)
 assert (p.returncode==0)==okay,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):
 return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.decode().splitlines() if l.split() and l.split()[0].startswith('.text'))
signature='''template<bool,class T> struct Enable {};
template<class T> struct Enable<true,T> {typedef T type;};
template<int N> struct Tag {};
template<class T> struct Gate {static const bool value=true;};
template<int N> typename Enable<Gate<Tag<N>>::value,long>::type item(long);
template<int M> typename Enable<Gate<Tag<M>>::value,long>::type item(long x){return (x+M)%97;}
long demanded(long x){long sum=0;
'''+''.join('sum+=item<%d>(x);\n'%i for i in range(1000))+'return sum;}\n'
expected=sum(sum((i+n)%97 for n in range(1000)) for i in range(64))*156
expected+=sum(sum((i+n)%97 for n in range(1000)) for i in range(16))
signature+='int main(int argc,char**){long sum=0;for(int i=0;i<argc*10000;++i)sum+=demanded(i&63);return sum==%d?0:1;}\n'%expected
sources={'signature':out/'signature.cpp','hosted':out/'hosted.cpp'}
sources['signature'].write_text(signature)
sources['hosted'].write_bytes((here/'hosted-stream-runtime.cpp').read_bytes())
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in binaries.items()},
 inputs={k:sha(p) for k,p in sources.items()},flags=['-O0','-c','--stats'],affinity=affinity,
 expected=dict(signature=expected,hosted=200000),stdin='extern-template-vtable\n',
 host_linker=run(['g++','--version']).stdout.decode(),runs=[],images={},summary={})
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for name,source in sources.items():
 labels='AB' if name=='signature' else 'B'
 data=result['stdin'].encode() if name=='hosted' else None
 if name=='hosted':
  p=run([binaries['A'],*result['flags'],source,'-o',out/'unsupported.o'],okay=False)
  result['unsupported_entry']=dict(status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode())
  run(['g++','-std=c++11','-O0',source,'-o',out/'host-control']);run([out/'host-control'],data)
 images={}
 for label in labels:
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([binaries[label],*result['flags'],source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe],data)
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,
   executable_bytes=exe.stat().st_size,object_text_bytes=text_size(obj),executable_text_bytes=text_size(exe))
 result['images'][name]=images
 for mode in ('compile','runtime'):
  orders=['AAAA']+['ABBA']*6 if name=='signature' else ['BBBB']*3
  for block,order in enumerate(orders):
   for label in order:
    args=[binaries[label],*result['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label)]
    start=time.perf_counter()
    p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args],data if mode=='runtime' else None)
    result['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,
     peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,
     phase_counters=[json.loads(l) for l in p.stderr.decode().splitlines() if l.startswith('{')]))
    save()
  rows=[r for r in result['runs'] if r['workload']==name and r['mode']==mode]
  summary={}
  if name=='signature':
   ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/
    statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
   aa=[r['wall_s'] for r in rows if not r['block']]
   summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,
    paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in labels:
   samples=[r for r in rows if r['label']==label and (name=='hosted' or r['block'])]
   summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),
    range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],
    peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
  result['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())

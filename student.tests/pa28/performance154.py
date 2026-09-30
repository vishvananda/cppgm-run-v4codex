#!/usr/bin/env python3
"""Fixed valid dynamic-override workload: A/A + six ABBA compile/runtime blocks."""
import hashlib, json, os, pathlib, statistics, subprocess, sys, time
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
 assert p.returncode==0,(args,p.returncode,p.stderr.decode(errors='replace'))
 return p
def text_size(p):
 return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
source=out/'override.cpp'
source.write_text('''struct Error { int value; Error(int n):value(n){} };
struct Specific:Error { Specific(int n):Error(n){} };
struct Base {virtual int value(int) throw(Error)=0;};
template<int N> struct Derived:Base {
 int value(int x) throw(Specific) {if (x&1) throw Specific(x+N); return x+N;}
 template<class T> void unused(){T::missing();}
};
int invoke(Base* p,int n){try{return p->value(n);}catch(const Error& e){return e.value;}}
long demanded(int x){long sum=0;
'''+''.join('Derived<%d> d%d;sum+=invoke(&d%d,x);\n'%(n,n,n) for n in range(400))+'''
return sum;}
int main(int argc,char**){if(demanded(argc)!=80200)return 1;
Derived<7> d; long sum=0;
for(int i=0;i<argc*240000;++i)sum+=invoke(&d,i%97);
return sum=='''+str(sum(i%97+7 for i in range(240000)))+'''?0:2;}
''')
result=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in binaries.items()},
 source_sha256=sha(source),flags=['-O0','-c','--stats'],affinity=affinity,
 host_linker=run(['g++','--version']).stdout.decode(),images={},runs=[],summary={})
executables={}
for label,binary in binaries.items():
 obj,exe=out/(label+'.o'),out/label
 run([binary,*result['flags'],source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe]);executables[label]=exe
 result['images'][label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe),object_text_bytes=text_size(obj))
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ('compile','runtime'):
 for block,order in enumerate(['AAAA']+['ABBA']*6):
  for label in order:
   args=[binaries[label],*result['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [executables[label]]
   started=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-started
   result['runs'].append(dict(mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,
    phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]))
   save()
 rows=[r for r in result['runs'] if r['mode']==mode]
 ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
 aa=[r['wall_s'] for r in rows if not r['block']]
 summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
 for label in 'AB':
  samples=[r for r in rows if r['block'] and r['label']==label]
  summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
 result['summary'][mode]=summary;save()
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
print(json.dumps(result['summary']),flush=True)

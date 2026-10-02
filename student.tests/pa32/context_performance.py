#!/usr/bin/env python3
"""Same-level frozen A/B call/EH/floating workloads; A/A and six ABBA blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ.get('PERF_CPU','2')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
 return p
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
level=os.environ.get('PA32_LEVEL','1')
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},affinity=affinity,flags=['-O'+level,'-c'],inputs={},runs=[],images={},summary={},
 budgets=dict(context_site_work=4096,context_caller_work=32768,context_unit_work='32*(I+O+P+S+F+1)',clone_caller_growth=1536,single_use_growth=2048,private_object_bytes=64,merge_growth=0,floating_growth=0))
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in os.environ.get('CONTEXT_WORKLOADS','checked floating parent landing').split():
 pre='';suffix='.lowir';count=2400;iterations=16000000
 if name=='checked':
  pre='''declare function @abort() -> void [object=abort, return=noreturn]
function @checked(%p : ptr, %n : i64) -> i64 [binding=internal] {
 block ^entry: eh_try ^landing %outside = cmp uge i64 %n, 4 branch %outside, ^throw, ^load
 block ^throw: call void @abort() return i64 -1
 block ^load: %slot = index i64 %p, %n %v = load i64 %slot eh_end return i64 %v
 block ^landing: eh_catch_all, 1 return i64 -2
}
'''
  body='''function @kernel0(%p : ptr) -> i64 [no_inline=yes] { block ^entry:
 %v = call i64 @checked(%p, 2) return i64 %v }
'''
  decl='extern "C" long kernel0(long*);';initialize='long a[4]={0,0,0,0};';step='a[2]=i&255;sum+=kernel0(a);';expected=iterations//256*32640
 elif name=='floating':
  body='''function @kernel0(%x : i64) -> i64 [no_inline=yes] {
 block ^entry: %inf = const f64 inf %zero = convert sitofp f64 i32 0
 %c = cmp gt f64 %inf, %zero branch %c, ^yes, ^no
 block ^yes: return i64 %x
 block ^no: %v = unary neg i64 %x return i64 %v
}
'''
  decl='extern "C" long kernel0(long);';initialize='';step='sum+=kernel0(i&255);';expected=iterations//256*32640
 elif name=='parent':
  suffix='.cpp';pre='''namespace std { template<class T> class initializer_list { const T* p; unsigned long n; public: const T* begin() const {return p;} }; }
int second(std::initializer_list<int> xs){return xs.begin()[1];}
'''
  body='extern "C" __attribute__((noinline)) int kernel0(int x){return second({13,x,15});}\n'
  decl='extern "C" int kernel0(int);';initialize='';step='sum+=kernel0(i&255);';expected=iterations//256*32640
 else:
  suffix='.cpp';count=600;iterations=240000
  pre='''int count;struct Guard {~Guard(){++count;}};
__attribute__((noinline)) void fail(int x){throw x;}
'''
  body='''extern "C" __attribute__((noinline)) int kernel0(int x){try {Guard a;fail(x);}catch(int v){return v;}return -1;}
'''
  decl='extern "C" int kernel0(int);extern int count;';initialize='';step='sum+=kernel0(i&255);';expected=(iterations//256*32640+(iterations%256)*(iterations%256-1)//2)
 source=out/(name+suffix);source.write_text(pre+''.join(body.replace('kernel0(',f'kernel{k}(') for k in range(count)))
 main=out/(name+'-main.cpp');main.write_text(f'''{decl}
int main(int argc,char**argv){{if(argc!=2)return 3;long n=(argv[1][0]-48)*{iterations//8}L;
{initialize}long sum=0;for(long i=0;i<n;++i){{{step}}}
return sum!={expected}L{('||count!=n' if name=='landing' else '')};}}
''')
 r['inputs'][name]=dict(source=str(source),source_sha256=sha(source),main=str(main),main_sha256=sha(main),arguments=['8'],expected=expected)
 run([bins['A'],'-O0','-c',main,'-o',out/'main.o']);images={}
 for label in 'AB':
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([bins[label],*r['flags'],source,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'8'])
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe),telemetry=[json.loads(s) for s in run([bins[label],*r['flags'],'--stats',source,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
  (out/(name+label+'.asm')).write_text(run(['objdump','-d','--disassemble=kernel0',obj]).stdout)
 r['images'][name]=images;save()
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:
    command=[bins[label],*r['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),'8']
    start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*command])
    row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode)
    if mode=='compile':
     row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
    r['runs'].append(row);save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];aa=[x['wall_s'] for x in rows if not x['block']]
  ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
  summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in 'AB':
   samples=[x for x in rows if x['block'] and x['label']==label]
   summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
  r['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

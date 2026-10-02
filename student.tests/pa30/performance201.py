#!/usr/bin/env python3
"""Equivalent O0 function-boundary/CFG owner scaling, with frozen A/A and ABBA."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(x).resolve() for x in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 assert p.returncode==0,(args,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],launchers=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,args in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
 for i in range(8):r['launchers'].append(dict(mode=mode,trial=i,**measured(args)))
body='''template<class T> int step(T x,int i){const int scale=17;
struct Local{int run(int x,int i){return (x*scale+(i&127))%1009;}};
auto work=[&]{return Local().run(x,i);};
while(2>1){if(i>=0)return work();}}
int invoke(int x,int i){return step(x,i);}
[[noreturn]] void stop(){throw 3;}
int stopped(){stop();}
int guarded(int x){try{if(x)return 4;stop();}catch(int){return 5;}}
'''
x=7;total=0;iterations=3000000
for i in range(iterations):x=(x*17+(i&127))%1009;total=(total+x)%65521
for n in [64,256,1024]:
 name='function_flow'+str(n)
 source=''.join('namespace block%d {\n%s\n}\n'%(i,body) for i in range(n))
 source+='''int main(int argc,char**argv){if(argc!=2)return 3;int x=argv[1][0]-48,total=0;
for(int i=0;i<%d;++i){x=block0::invoke(x,i);total=(total+x)%%65521;}
return x==%d && total==%d && block0::guarded(0)==5 ? 0:1;}\n'''%(iterations,x,total)
 src=out/(name+'.cpp');src.write_text(source)
 r['inputs'][name]=dict(N=n,source=source,sha256=sha(src),iterations=iterations,seed=7,expected=[x,total])
 executables={};images={}
 for k,b in bins.items():
  obj=out/(name+k+'.o');exe=out/(name+k)
  run([b,*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);executables[k]=exe
  images[k]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),executable_text_bytes=size(exe))
 r['images'][name]=images;save()
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for k in order:
    args=[bins[k],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [executables[k],'7']
    row=dict(workload=name,mode=mode,block=block,label=k,**measured(args))
    if mode=='compile':
     row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[k]['object_sha256']
    r['runs'].append(row);save()
  rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode];aa=[v['wall_s'] for v in rows if v['block']==0]
  ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
  s=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for k in bins:
   samples=[v for v in rows if v['block'] and v['label']==k]
   s[k]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
  r['summary'].setdefault(name,{})[mode]=s;save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

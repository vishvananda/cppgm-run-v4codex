#!/usr/bin/env python3
"""Frozen equivalent vector A/A + ABBA, then final-only SIMD scaling.
Preserves every trial; compiler and checked executable timings are separate.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(x).resolve() for x in sys.argv[2:4]]));affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(cmd,ok=True):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=45)
 if ok:assert not p.returncode,(cmd,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],launchers=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measured(cmd):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*cmd]);return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,cmd in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
 for trial in range(8):r['launchers'].append(dict(mode=mode,trial=trial,**measured(cmd)))
iterations=3000000
for family in ['vector','simd']:
 x=7;total=0
 for i in range(iterations):
  x=(x*17+(i&127))%1009
  if family=='simd':x=(x+max(-128,min(127,x-500))+128)%1009
  total=(total+x)%65521
 if family=='vector':body='''typedef int V __attribute__((vector_size(8)));
template<class T> int step(T x,int i) {
 V a=__builtin_ia32_vec_init_v2si(x*17,i&127);
 return (__builtin_ia32_vec_ext_v2si(a,0)+__builtin_ia32_vec_ext_v2si(a,1))%1009;
}
int invoke(int x,int i){return step(x,i);}
'''
 else:body='''typedef short S __attribute__((vector_size(8)));
typedef char B __attribute__((vector_size(8)));
typedef float F __attribute__((vector_size(16)));
template<class T> int step(T x,int i) {
 int v=(x*17+(i&127))%1009;
 S a={(short)(v-500),-129,128,32767};
 B b=__builtin_ia32_packsswb(a,S{-32768,-1,0,1});
 F f=__builtin_ia32_addss(F{(float)v,2,3,4},F{(float)b[0],6,7,8});
 return ((int)f[0]+128)%1009;
}
int invoke(int x,int i){return step(x,i);}
'''
 for n in [64,256,1024]:
  name=family+str(n);src=out/(name+'.cpp')
  source=''.join('namespace group%d{\n%s}\n'%(j,body) for j in range(n))
  source+='int main(int argc,char**argv){if(argc!=2)return 3;int x=argv[1][0]-48,total=0;for(int i=0;i<%d;++i){x=group0::invoke(x,i);total=(total+x)%%65521;}return x==%d&&total==%d?0:1;}\n'%(iterations,x,total)
  src.write_text(source);r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,expected=[x,total]);images={};exes={}
  labels='AB' if family=='vector' else 'B'
  if family=='simd':
   p=run([bins['A'],*r['flags'],src,'-o',out/'entry.o'],False);assert p.returncode
   r['inputs'][name]['entry_rejection']=dict(status=p.returncode,stderr=p.stderr)
  for label in labels:
   obj=out/(name+label+'.o');exe=out/(name+label)
   run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);exes[label]=exe
   images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),executable_text_bytes=size(exe))
  r['images'][name]=images;save()
  for mode in ['compile','runtime']:
   orders=['AAAA']+['ABBA']*6 if family=='vector' else ['BBBB']*2
   for block,order in enumerate(orders):
    for label in order:
     cmd=[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exes[label],'7']
     row=dict(workload=name,mode=mode,block=block,label=label,**measured(cmd))
     if mode=='compile':row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
     r['runs'].append(row);save()
   rows=[s for s in r['runs'] if s['workload']==name and s['mode']==mode];summary={}
   if family=='vector':
    aa=[s['wall_s'] for s in rows if s['block']==0];ratios=[statistics.mean(s['wall_s'] for s in rows if s['block']==j and s['label']=='B')/statistics.mean(s['wall_s'] for s in rows if s['block']==j and s['label']=='A') for j in range(1,7)]
    summary.update(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
   for label in labels:
    sample=[s for s in rows if s['label']==label and (s['block'] or family=='simd')]
    summary[label]=dict(median_s=statistics.median(s['wall_s'] for s in sample),range_s=[min(s['wall_s'] for s in sample),max(s['wall_s'] for s in sample)],peak_rss_kib=max(s['peak_rss_kib'] for s in sample))
   r['summary'].setdefault(name,{})[mode]=summary;save()
  print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

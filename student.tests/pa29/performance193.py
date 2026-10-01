#!/usr/bin/env python3
"""Selected ABI owner scaling. Repeated-tag inputs are correct equivalent A/B.
Definition-only tags are corrected-only; invalid ABI output is not a baseline.
Usage: performance193.py OUT ENTRY FINAL (PERF_CPU optionally pins affinity).
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-std=c++11','-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],launchers=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def textsize(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,args in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
 for i in range(8):r['launchers'].append(dict(mode=mode,index=i,**measured(args)))
seed=7;iterations=5000000;x=seed;total=0
for i in range(iterations):x=(x+3)%97;total+=x
for n in [256,1024,4096]:
 for family in ['repeated','suppressed']:
  name=family+str(n);tag='__attribute__((abi_tag("keep")))'
  source='template<int N> struct Box{struct Inner{'+tag+' static long value(long);};};\n'
  source+='template<int N> '+(tag if family=='repeated' else '')+' long Box<N>::Inner::value(long x){return (x+N)%97;}\n'
  source+='long demanded(long x){long sum=0;\n'+''.join('sum+=Box<%d>::Inner::value(x);\n'%i for i in range(n))+'return sum;}\n'
  expected=sum((seed+i)%97 for i in range(n))
  source+='int main(int argc,char**argv){if(argc!=2)return 3;long x=argv[1][0]-48;\n'
  source+='if(demanded(x)!=%d)return 2;long sum=0;for(int i=0;i<%d;++i){x=Box<3>::Inner::value(x);sum+=x;}\n'%(expected,iterations)
  source+='return sum==%d && x==%d?0:1;}\n'%(total,x)
  src=out/(name+'.cpp');src.write_text(source)
  r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=seed,expected=[expected,total,x],policy=family)
  labels='AB' if family=='repeated' else 'B';images={}
  for label in labels:
   obj=out/(name+label+'.o');exe=out/(name+label)
   run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,str(seed)])
   names=[s.split()[-1] for s in run(['nm',obj]).stdout.splitlines() if '5value' in s]
   assert len(names)==n and all(('B4keep' in s)==(family=='repeated') for s in names)
   images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=textsize(obj),executable_text_bytes=textsize(exe),object_bytes=obj.stat().st_size,executable_bytes=exe.stat().st_size,checked_symbols=n)
  if family=='repeated':assert images['A']['object_sha256']==images['B']['object_sha256']
  r['images'][name]=images
  for mode in ['compile','runtime']:
   orders=['AAAA']+['ABBA']*6 if family=='repeated' else ['BBBB']*2
   for block,order in enumerate(orders):
    for label in order:
     args=[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),str(seed)]
     r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,**measured(args)));save()
   rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode];summary={}
   for label in labels:
    values=[v for v in rows if v['label']==label]
    summary[label]=dict(median_s=statistics.median(v['wall_s'] for v in values),range_s=[min(v['wall_s'] for v in values),max(v['wall_s'] for v in values)],peak_rss_kib=max(v['peak_rss_kib'] for v in values))
   if family=='repeated':
    ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
    summary.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(v['wall_s'] for v in rows if not v['block']),max(v['wall_s'] for v in rows if not v['block'])])
   r['summary'].setdefault(name,{})[mode]=summary;save()
  print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

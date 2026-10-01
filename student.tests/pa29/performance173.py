#!/usr/bin/env python3
"""Ordinary closure A/A+ABBA and explicit-template closure capability scaling."""
import hashlib,json,os,pathlib,platform,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(v).resolve() for k,v in zip('AB',sys.argv[2:4])}
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
 if not allow:assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for k,p in cc.items()},flags=flags,affinity=affinity,platform=platform.platform(),script_sha256=sha(pathlib.Path(__file__)),inputs={},images={},runs=[],summary={},entry_checks={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for i in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);r['launcher'].append(time.perf_counter()-start)
for kind,n in [('ordinary',1200)]+[(k,n) for k in ['closures','calls'] for n in [600,1200,2400]]:
 name=kind+str(n);src=out/(name+'.cpp');iterations=20000000
 if kind in ['ordinary','closures']:
  head='' if kind=='ordinary' else '<class T>'
  param='long' if kind=='ordinary' else 'T'
  prefix='template<int N> long work(long x){auto f=[x]'+head+'('+param+' y){return (x+y+N)%97;};return f(1)+f(1L)+f(1);}\n'
  prefix+='long demanded(long x){long s=0;'+''.join('s+=work<%d>(x);'%i for i in range(n))+'return s;}\n'
  expected=sum(3*((2+i)%97) for i in range(n));runtime='work<7>((i&127)+argc)'
  period=[3*((i+9)%97) for i in range(128)]
 else:
  prefix='long demanded(long x){auto f=[x]<int N>(){return (x+N)%97;};long s=0;'+''.join('s+=f.operator()<%d>();'%i for i in range(n))+'return s;}\n'
  prefix+='long work(long x){auto f=[x]<int N>(){return (x+N)%97;};return f.operator()<7>();}\n'
  expected=sum((1+i)%97 for i in range(n));runtime='work((i&127)+argc)';period=[(i+8)%97 for i in range(128)]
 checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
 src.write_text(prefix+'int main(int argc,char**){if(demanded(argc)!='+str(expected)+')return 1;long sum=0;for(int i=0;i<argc*20000000;++i)sum+='+runtime+';return sum=='+str(checksum)+'LL?0:2;}\n')
 r['inputs'][name]=dict(path=str(src),sha256=sha(src));labels='AB' if kind=='ordinary' else 'B';executables={}
 if labels=='B':
  old=run([cc['A'],*flags,src,'-o',out/'entry.o'],True);assert old.returncode!=0
  r['entry_checks'][name]=dict(status=old.returncode,stderr=old.stderr.decode())
 for label in labels:
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([cc[label],*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe]);executables[label]=exe
  r['images'].setdefault(name,{})[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,text_bytes=text_size(exe))
 for mode in ['compile','runtime']:
  orders=['AAAA']+['ABBA']*6 if labels=='AB' else ['BBBB']*2
  for block,order in enumerate(orders):
   for label in order:
    args=[cc[label],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [executables[label]]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];s={}
  for label in labels:
   a=[x for x in rows if x['label']==label and (labels=='B' or x['block'])]
   s[label]=dict(median_s=statistics.median(x['wall_s'] for x in a),range_s=[min(x['wall_s'] for x in a),max(x['wall_s'] for x in a)],peak_rss_kib=max(x['peak_rss_kib'] for x in a))
  if labels=='AB':
   ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
   aa=[x['wall_s'] for x in rows if not x['block']];s.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
  r['summary'].setdefault(name,{})[mode]=s;save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in cc.items());save()

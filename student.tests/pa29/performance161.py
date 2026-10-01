#!/usr/bin/env python3
"""Frozen atomic capability costs and equivalent GNU add A/A + ABBA controls."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
a,b=(pathlib.Path(p).resolve() for p in sys.argv[2:4]);affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 assert (p.returncode==0)==ok,(str(args[0]),p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binaries={k:dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for k,p in [('A',a),('B',b)]},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},unsupported_A={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
def measure(workload,args,**fields):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 result['runs'].append(dict(workload=workload,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')],**fields));save()
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
iterations=6000000
for kind,n in [('c11',600),('c11',1200),('c11',2400),('gnu',1200)]:
 key=kind+str(n);src=out/(key+'.cpp')
 if kind=='c11':
  prefix='template<int N> int work(int x){_Atomic(int) value=x;return __c11_atomic_fetch_xor(&value,N&7,0)+__c11_atomic_load(&value,0);}\n'
  demanded=sum(1+(1^(i&7)) for i in range(n));period=[i+1+((i+1)^7) for i in range(128)]
 else:
  prefix='template<int N> long work(long x){long value=x;return __atomic_add_fetch(&value,N+1,0);}\n'
  demanded=sum(1+i+1 for i in range(n));period=[i+1+8 for i in range(128)]
 checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
 src.write_text(prefix+'long demanded(long x){long sum=0;\n'+''.join('sum+=work<%d>(x);\n'%i for i in range(n))+'return sum;}\nint main(int argc,char**){if(demanded(argc)!='+str(demanded)+')return 1;\nlong sum=0;for(int i=0;i<argc*6000000;++i)sum+=work<7>((i&127)+argc);\nreturn sum=='+str(checksum)+'LL?0:2;}\n')
 result['inputs'][key]=dict(path=str(src),sha256=sha(src));images={};executables={}
 if kind=='c11':
  p=run([a,*result['flags'],src,'-o',out/'bad.o'],False);result['unsupported_A'][key]=dict(status=p.returncode,stderr=p.stderr.decode())
 for label,cc in ([('B',b)] if kind=='c11' else [('A',a),('B',b)]):
  obj=out/(key+label+'.o');exe=out/(key+label)
  run([cc,*result['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe]);executables[label]=exe
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
 result['images'][key]=images
 for mode in ['compile','runtime']:
  for block,order in enumerate(['BBBBBBBB'] if kind=='c11' else ['AAAA']+['ABBA']*6):
   for sample,label in enumerate(order):
    measure(key,[(a if label=='A' else b),*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [executables[label]],mode=mode,block=block,sample=sample,label=label)
 result['summary'][key]={}
 for mode in ['compile','runtime']:
  rows=[r for r in result['runs'] if r['workload']==key and r['mode']==mode];summary={}
  for label in images:
   rr=[r for r in rows if r['label']==label and (kind=='c11' or r['block'])]
   summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in rr),range_s=[min(r['wall_s'] for r in rr),max(r['wall_s'] for r in rr)],peak_rss_kib=max(r['peak_rss_kib'] for r in rr))
  if kind=='gnu':
   ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==block and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==block and r['label']=='A') for block in range(1,7)]
   aa=[r['wall_s'] for r in rows if not r['block']]
   summary.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
  result['summary'][key][mode]=summary
 save();print(key,json.dumps(result['summary'][key]),flush=True)
assert sha(a)==result['binaries']['A']['sha256'] and sha(b)==result['binaries']['B']['sha256'];save()

#!/usr/bin/env python3
"""Fixed positive-array A/A+ABBA, plus new zero-extent scaling (no reject speedup)."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert not p.returncode,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],runs=[],images={},inputs={},summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for width in [1,0]:
 for n in [400,800,1600]:
  name=f'array{width}-{n}'; src=out/(name+'.cpp')
  check=sum((7+i)%97 for i in range(n))
  source=f'''template<int I> struct Box{{long value; long array[{width}];}};
template<int I> long read(long x){{Box<I> a={{x,{{}}}};Box<I> b=a;return (b.value+I)%97;}}
long probe(long x){{long s=0;'''+''.join(f's+=read<{i}>(x);' for i in range(n))+f'''return s;}}
struct State{{long value;long tail[{width}];}};
long step(State& s,long i){{s.value=(s.value*17+(i&127))%1009;return s.value;}}
int main(int argc,char**argv){{long seed=argc>1?argv[1][0]-'0':1;if(probe(seed)!={check})return 2;
State s={{seed,{{}}}};long total=0;for(long i=0;i<10000000;++i)total+=step(s,i);
return total==EXPECTED?0:1;}}
'''
  if width==1 and n==400:
   value=7;expected=0
   for i in range(10000000):value=(value*17+(i&127))%1009;expected+=value
  src.write_text(source.replace('EXPECTED',str(expected)))
  r['inputs'][name]=dict(sha256=sha(src),N=n,width=width,seed=7,iterations=10000000,expected=expected,probe=check)
  labels='AB' if width else 'B';exes={};r['images'][name]={}
  for label in labels:
   obj=out/(name+label+'.o');exe=out/(name+label)
   run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);exes[label]=exe
   text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
   r['images'][name][label]=dict(text_bytes=text,object_sha256=sha(obj),executable_sha256=sha(exe))
  for mode in ['compile','runtime']:
   orders=['AAAA']+['ABBA']*6 if width else ['BBBB','BBBB']
   for block,order in enumerate(orders):
    for label in order:
     args=[bins[label],*r['flags'],src,'-o',out/'timed.o'] if mode=='compile' else [exes[label],'7']
     start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-start
     r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')]))
     save()
   rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];summary={}
   for label in labels:
    values=[x for x in rows if x['label']==label and (x['block'] or not width)]
    summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in values),range_s=[min(x['wall_s'] for x in values),max(x['wall_s'] for x in values)],peak_rss_kib=max(x['peak_rss_kib'] for x in values))
   if width:
    ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
    aa=[x['wall_s'] for x in rows if not x['block']]
    summary.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
   r['summary'].setdefault(name,{})[mode]=summary;save()
  print(name,r['summary'][name],flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

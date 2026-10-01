#!/usr/bin/env python3
"""Frozen callable-owner A/A+ABBA and final-only extension scaling at O0."""
import pathlib,subprocess,json,hashlib,statistics,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(p).resolve() for k,p in zip('AB',sys.argv[2:4])}
flags=['-std=c++11','-O0','-c','--stats'];r=dict(flags=flags,binaries={},inputs={},runs=[],summary={},launcher_s=[])
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for k,p in cc.items():r['binaries'][k]=dict(path=str(p),sha256=sha(p))
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),text=True,capture_output=True,timeout=90)
 if ok:assert not p.returncode,(args,p.returncode,p.stderr)
 return p
def text_size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
for _ in range(8):
 t=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss','/bin/true']);r['launcher_s'].append(time.perf_counter()-t)
for family,n in [('straight',600),('straight',1200),('straight',2400),('branch',1200),('floating',1200),('static',600),('static',1200),('static',2400)]:
 name=family+str(n);src=out/(name+'.cpp');reps=24000000//n
 if family in ['straight','static']:
  body='return (x+N)%97;';fn=lambda i,x:(x+i)%97
 elif family=='branch':
  body='if((x+N)%3==0)return (x+N)%97;return (x+N)%37;';fn=lambda i,x:(x+i)%(97 if (x+i)%3==0 else 37)
 else:
  body='double y=double((x+N)%97)*.5+1.;return long(y);';fn=lambda i,x:((x+i)%97)//2+1
 head='template<int N> '+('__attribute__((always_inline)) inline long work(long x){'+body+'}' if family!='static' else 'struct F {static long operator()(long x){'+body+'}};')
 calls=''.join('sum+='+('work<%d>(x);'%i if family!='static' else 'F<%d>()(x);'%i) for i in range(n))
 src.write_text('extern "C" long strtol(const char*,char**,int);extern "C" int printf(const char*,...);\n'+head+'\nlong demanded(long x){long sum=0;'+calls+'return sum;}\nint main(int argc,char** argv){long seed=argc>1?strtol(argv[1],0,10):3;long sum=0;for(int i=0;i<%d;++i)sum+=demanded(seed+i%%13);printf("%%ld\\n",sum);}\n'%reps)
 expected=str(sum(sum(fn(i,17+j) for i in range(n))*(reps//13+(j<reps%13)) for j in range(13)))+'\n'
 entry=run([cc['A'],*flags,src,'-o',out/'entry.o'],False)
 if family=='static':assert entry.returncode
 else:assert not entry.returncode,entry.stderr
 labels='B' if family=='static' else 'AB';images={};executables={}
 for label in labels:
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([cc[label],*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);assert run([exe,'17']).stdout==expected
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text_size(exe));executables[label]=exe
 host=out/(name+'-host');run(['clang++','-std=c++11','-Wno-c++23-extensions','-O0',src,'-o',host]);assert run([host,'17']).stdout==expected
 r['inputs'][name]=dict(path=str(src),sha256=sha(src),n=n,family=family,expected=expected,args=['17'],entry_status=entry.returncode,entry_stderr=entry.stderr,images=images)
 for mode in ['compile','runtime']:
  orders=['BBBB','BBBB'] if family=='static' else ['AAAA']+['ABBA']*6
  for block,order in enumerate(orders):
   for label in order:
    args=[cc[label],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [executables[label],'17']
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-start
    if mode=='runtime':assert p.stdout==expected
    r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]));save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];summary={}
  for label in labels:
   values=[x for x in rows if x['label']==label and (family=='static' or x['block'])]
   summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in values),range_s=[min(x['wall_s'] for x in values),max(x['wall_s'] for x in values)],peak_rss_kib=max(x['peak_rss_kib'] for x in values))
  if family!='static':
   aa=[x['wall_s'] for x in rows if not x['block']]
   ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
   summary.update(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  r['summary'].setdefault(name,{})[mode]=summary;save()
 print(name,r['summary'][name],flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in cc.items())

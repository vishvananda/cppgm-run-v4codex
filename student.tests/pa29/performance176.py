#!/usr/bin/env python3
"""Required declaration/demand scaling; compiler and checked runtime separate."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(v).resolve() for k,v in zip('AB',sys.argv[2:4])}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 if not allow:assert not p.returncode,(args,p.returncode,p.stderr)
 return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in cc.items()},flags=['-std=c++11','-O0','-c','--stats'],inputs={},runs=[],summary={})
def save():(out/'declaration-performance.json').write_text(json.dumps(r,indent=2)+'\n')
for n in [600,1200,2400]:
 source=out/('declarations%d.cpp'%n)
 source.write_text('extern "C" long strtol(const char*,char**,int);\nextern "C" int printf(const char*,...);\n#define EX __attribute__((exclude_from_explicit_instantiation))\nint creations; long materialize(int n) { ++creations; return n; }\ntemplate<int N> struct Cell { EX inline static long value = materialize(N); EX long sample(long x) const; };\ntemplate<int N> long Cell<N>::sample(long x) const { return (x+value)%97; }\n'+''.join('extern template struct Cell<%d>;\n'%i for i in range(n))+'long work(long x) { long sum=0;\n'+''.join('sum+=Cell<%d>().sample(x);\n'%i for i in range(n))+'return sum;}\nint main(int argc,char** argv) { if(creations!=%d)return 2; long seed=argc>1?strtol(argv[1],0,10):3; long sum=0;for(int i=0;i<%d;++i)sum+=work(seed+i%%13);printf("%%ld\\n",sum);}\n'%(n,24000000//n))
 obj=out/('declarations%d.o'%n);exe=out/('declarations%d'%n)
 entry=run([cc['A'],*r['flags'],source,'-o',out/'entry.o'],True)
 entry_link=run(['g++',out/'entry.o','-o',out/'entry'],True) if not entry.returncode else None
 run([cc['B'],*r['flags'],source,'-o',obj]);run(['g++',obj,'-o',exe])
 reps=24000000//n
 expected=str(sum(sum((17+i+j)%97 for j in range(n))*(reps//13+(i<reps%13)) for i in range(13)))+'\n'
 assert run([exe,'17']).stdout==expected
 r['inputs'][str(n)]=dict(path=str(source),sha256=sha(source),entry_status=entry.returncode,entry_stderr=entry.stderr,entry_link_status=entry_link.returncode if entry_link else None,entry_link_stderr=entry_link.stderr if entry_link else None,expected=expected,object_sha256=sha(obj),executable_sha256=sha(exe),runtime_args=['17'],text_bytes=size(exe))
 for mode in ['compile','runtime']:
  for sample in range(8):
   args=[cc['B'],*r['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [exe,'17']
   begin=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-begin
   if mode=='runtime':assert p.stdout==expected
   r['runs'].append(dict(n=n,mode=mode,sample=sample,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
   save()
  rows=[a for a in r['runs'] if a['n']==n and a['mode']==mode]
  r['summary'].setdefault(str(n),{})[mode]=dict(median_s=statistics.median(a['wall_s'] for a in rows),range_s=[min(a['wall_s'] for a in rows),max(a['wall_s'] for a in rows)],peak_rss_kib=max(a['peak_rss_kib'] for a in rows))
 save();print(n,r['summary'][str(n)],flush=True)
r['launcher_s']=[]
for _ in range(8):
 begin=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss','/bin/true']);r['launcher_s'].append(time.perf_counter()-begin)
save()
assert all(sha(cc[k])==v['sha256'] for k,v in r['binaries'].items())

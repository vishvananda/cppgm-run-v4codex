#!/usr/bin/env python3
"""Corrected decomposition scaling; entry rejection is not a speed baseline."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(v).resolve() for k,v in zip('AB',sys.argv[2:4])}
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 if not allow:assert not p.returncode,(args,p.returncode,p.stderr)
 return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in cc.items()},flags=flags,inputs={},runs=[],summary={},launcher_s=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for _ in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss','/bin/true']);r['launcher_s'].append(time.perf_counter()-start)
for family in ['class','array','lifetime']:
 for n in [600,1200,2400]:
  name=family+str(n);src=out/(name+'.cpp');reps=24000000//n
  if family=='class':
   head='struct Pair{long a;long& b;};'
   body='Pair p{(x+N)%97,x};auto [a,b]=p;b+=N%5;return a+b;'
   fn=lambda i,x:(x+i)%97+x+i%5
  elif family=='array':
   head=''
   body='long a[2]={x+N,x-N};auto [u,v]=a;u+=v;return u%97;'
   fn=lambda i,x:2*x%97
  else:
   head='long destroyed;struct Pair{long a,b;Pair(long x):a(x),b(x+1){}~Pair(){++destroyed;}};'
   body='Pair p((x+N)%97);auto [a,b]=p;return a+b;'
   fn=lambda i,x:2*((x+i)%97)+1
  expected=str(sum(sum(fn(i,17+j) for i in range(n))*(reps//13+(j<reps%13)) for j in range(13)))+'\n'
  src.write_text('extern "C" long strtol(const char*,char**,int);extern "C" int printf(const char*,...);\n'+head+'template<int N> long work(long x){'+body+'}\nlong demanded(long x){long sum=0;'+''.join('sum+=work<%d>(x);'%i for i in range(n))+'return sum;}\nint main(int argc,char** argv){long seed=argc>1?strtol(argv[1],0,10):3;long sum=0;for(int i=0;i<%d;++i)sum+=demanded(seed+i%%13);'%reps+('if(destroyed!=48000000)return 2;' if family=='lifetime' else '')+'printf("%ld\\n",sum);}\n')
  entry=run([cc['A'],*flags,src,'-o',out/'entry.o'],True);assert entry.returncode
  obj=out/(name+'.o');exe=out/name;host=out/(name+'-host')
  run([cc['B'],*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);run(['g++','-std=c++17','-O0',src,'-o',host])
  assert run([host,'17']).stdout==expected and run([exe,'17']).stdout==expected
  r['inputs'][name]=dict(path=str(src),sha256=sha(src),n=n,family=family,expected=expected,args=['17'],entry_status=entry.returncode,entry_stderr=entry.stderr,object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc['B'],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [exe,'17']
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-start
    if mode=='runtime':assert p.stdout==expected
    r['runs'].append(dict(workload=name,mode=mode,sample=sample,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]));save()
   rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
   r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
  print(name,r['summary'][name],flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in cc.items())

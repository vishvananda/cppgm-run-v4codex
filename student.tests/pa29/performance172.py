#!/usr/bin/env python3
"""New fold capability costs, frozen inputs/flags/binaries, checked runtime."""
import hashlib,json,os,pathlib,platform,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
entry,cc=[pathlib.Path(p).resolve() for p in sys.argv[2:4]]
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
 if not allow:assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for k,p in [('A',entry),('B',cc)]},flags=flags,affinity=affinity,platform=platform.platform(),script_sha256=sha(pathlib.Path(__file__)),inputs={},images={},runs=[],summary={},entry_checks={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for i in range(8):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);r['launcher'].append(time.perf_counter()-start)
for kind in ['runtime','query']:
 for n in ([600,1200,2400] if kind=='runtime' else [6000,12000,24000]):
  name=kind+str(n);src=out/(name+'.cpp');iterations=20000000
  if kind=='runtime':
   prefix='''template<int N> long work(long x){return (x+N)%97;}
template<int...N> long demanded(long x){return (work<N>(x)+...+0);}
'''
   expected=sum((1+i)%97 for i in range(n))
   prefix+='long check(long x){return demanded<'+','.join(map(str,range(n)))+'>(x);}\n'
   body='if(check(argc)!='+str(expected)+')return 1;long sum=0;for(int i=0;i<argc*20000000;++i)sum+=demanded<1,3,7>((i&127)+argc);'
   period=[sum((i+1+j)%97 for j in [1,3,7]) for i in range(128)]
  else:
   prefix='template<int N>struct Count{static constexpr int value=N;};template<int...N>using Sum=Count<(N+...)>;\n'
   prefix+='typedef Sum<'+','.join(['1']*n)+'> Big;static_assert(Big::value=='+str(n)+',"fold sum");\n'
   prefix+='int item(int x){return (Big::value+x)%97;}\n'
   body='long sum=0;for(int i=0;i<argc*20000000;++i)sum+=item((i&127)+argc);'
   period=[(n+i+1)%97 for i in range(128)]
  checksum=sum(period)*(iterations//128)+sum(period[:iterations%128])
  src.write_text(prefix+'int main(int argc,char**){'+body+'return sum=='+str(checksum)+'LL?0:2;}\n')
  r['inputs'][name]=dict(path=str(src),sha256=sha(src))
  old=run([entry,*flags,src,'-o',out/'entry.o'],True);assert old.returncode!=0
  r['entry_checks'][name]=dict(status=old.returncode,stderr=old.stderr.decode())
  obj=out/(name+'.o');exe=out/name
  run([cc,*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*flags,src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[s for s in r['runs'] if s['workload']==name and s['mode']==mode]
   r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(s['wall_s'] for s in rows),range_s=[min(s['wall_s'] for s in rows),max(s['wall_s'] for s in rows)],peak_rss_kib=max(s['peak_rss_kib'] for s in rows));save()
  print(name,json.dumps(r['summary'][name]),flush=True)
assert sha(cc)==r['binaries']['B']['sha256'] and sha(entry)==r['binaries']['A']['sha256'];save()

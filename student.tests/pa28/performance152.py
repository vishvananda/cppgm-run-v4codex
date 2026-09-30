#!/usr/bin/env python3
"""New host-filter semantics: standalone costs; baseline behavior is incorrect."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args): return subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
source=out/'filters.cpp'; iterations=120000
checksum=sum(7 if (i%97)&1 else 2*(i%97) for i in range(iterations))
conversions=400+sum(bool((i%97)&1) for i in range(iterations))
source.write_text('''namespace std {typedef void (*unexpected_handler)(); unexpected_handler set_unexpected(unexpected_handler) throw();}
extern "C" void abort();
int live,dead,converted;
struct Guard { Guard(){++live;} ~Guard(){--live;++dead;} };
void unexpected(){if(live)abort();++converted;throw 3.5;}
template<int N> void step(int x) throw(double){Guard g;if(x&1)throw x+N;throw double(x+N);}
int demanded(int x){int sum=0;
'''+''.join('try{step<%d>(x);}catch(double d){sum+=int(d*2);}\n'%n for n in range(400))+'''
return sum;}
int main(int argc,char**){std::set_unexpected(unexpected);long sum=0;
try {if(demanded(argc)!=2800)return 2;
for(int i=0;i<argc*120000;++i){try{step<0>(i%97);}catch(double d){sum+=int(d*2);}}}
catch(...){return 9;}
return live==0 && dead==120400 && converted=='''+str(conversions)+''' && sum=='''+str(checksum)+'''?0:3;}
''')
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in binaries.items()},source_sha256=sha(source),flags=['-O0','-c','--stats'],affinity=affinity,runs=[],summary={})
result['baseline']={}
for label in 'AB':
 obj=out/(label+'.o');exe=out/label
 p=run([binaries[label],'-O0','-c','--stats',source,'-o',obj]);assert p.returncode==0,p.stderr
 p=run(['g++',obj,'-o',exe]);assert p.returncode==0,p.stderr
 p=run([exe]);assert (p.returncode==0)==(label=='B'),(label,p.returncode,p.stderr)
 if label=='A': result['baseline']=dict(status=p.returncode,stderr=p.stderr.decode(),reason='does not enforce allowed-exception filter')
obj,exe=out/'B.o',out/'B'
def text_size(path):
 p=run(['size','-A',path]);assert p.returncode==0
 return sum(int(s.split()[1]) for s in p.stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result['image']=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,object_text_bytes=text_size(obj),executable_bytes=exe.stat().st_size,executable_text_bytes=text_size(exe))
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ('compile','runtime'):
 for i in range(12):
  args=[binaries['B'],'-O0','-c','--stats',source,'-o',out/'measure.o'] if mode=='compile' else [exe]
  start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
  assert p.returncode==0,(mode,p.returncode,p.stderr.decode())
  result['runs'].append(dict(mode=mode,sample=i,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]))
  save()
 rows=[r for r in result['runs'] if r['mode']==mode]
 result['summary'][mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
print(json.dumps(result['summary']),flush=True)

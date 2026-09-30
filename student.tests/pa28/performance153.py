#!/usr/bin/env python3
"""New virtual-primary semantics: standalone costs, never compare incorrect A speed."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):return subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
def size(p):
 r=run(['size','-A',p]);assert not r.returncode
 return sum(int(s.split()[1]) for s in r.stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
source=out/'primary.cpp';iterations=3000000
checksum=sum(2*(i%97+8) for i in range(iterations))%1009
source.write_text('''struct Root {virtual ~Root(){} virtual long value(){return 3;}};
struct Owner : virtual Root {virtual Root* self(){return this;}};
struct Extra {virtual ~Extra(){} virtual long extra(){return -1000;}};
long secondary(Extra* p){return p->extra();}
template<int N> struct Cell : Owner, Extra {
 long payload; Cell(long n):payload(n){if(secondary(this)!=n+N)payload=-10000;}
 Cell* self(){return payload?this:0;}
 long value(){return payload+N;} long extra(){return payload+N;}
};
template<int N> struct Complete : Cell<N> {Complete(long n):Cell<N>(n){}};
template<int N> long demand(long x){Complete<N> c(x);Owner* o=&c;Root* r=&c;
 return o->self()==r && sizeof(c)==24 ? r->value():-10000;}
long demanded(long x){long sum=0;
'''+''.join('sum+=demand<%d>(x);\n'%n for n in range(400))+'''
return sum;}
long read(Owner* p){return p->self()->value();}
int main(int argc,char**){if(demanded(argc)!=80200)return 2;
Complete<7> c(argc);long sum=0;
for(int i=0;i<argc*3000000;++i){c.payload=i%97+1;sum=(sum+read(&c)+secondary(&c))%1009;}
return sum=='''+str(checksum)+'''?0:3;}
''')
result=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in binaries.items()},source_sha256=sha(source),flags=['-O0','-c','--stats'],affinity=affinity,runs=[],summary={})
for label in 'AB':
 obj,exe=out/(label+'.o'),out/label
 r=run([binaries[label],'-O0','-c','--stats',source,'-o',obj]);assert not r.returncode,r.stderr
 r=run(['g++',obj,'-o',exe]);assert not r.returncode,r.stderr
 r=run([exe]);assert (r.returncode==0)==(label=='B'),(label,r.returncode,r.stderr)
 if label=='A':result['baseline']=dict(status=r.returncode,reason='does not share nearly-empty virtual primary storage')
 else:result['image']=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,object_text_bytes=size(obj),executable_bytes=exe.stat().st_size,executable_text_bytes=size(exe))
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ['compile','runtime']:
 for i in range(12):
  args=[binaries['B'],'-O0','-c','--stats',source,'-o',out/'measure.o'] if mode=='compile' else [out/'B']
  start=time.perf_counter();r=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
  assert not r.returncode,(mode,r.returncode,r.stderr)
  result['runs'].append(dict(mode=mode,sample=i,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=r.returncode,phase_counters=[json.loads(s) for s in r.stderr.decode().splitlines() if s.startswith('{')]))
  save()
 rows=[r for r in result['runs'] if r['mode']==mode]
 result['summary'][mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
print(json.dumps(result['summary']),flush=True)

#!/usr/bin/env python3
"""Measure corrected current-instantiation queries; A rejects these workloads.
Run after the equivalent fixed A/A + ABBA benchmarks, without contention.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def textsize(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,args in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
 for i in range(8):r['launchers'].append(dict(mode=mode,index=i,**measured(args)))
seed=7;iterations=3000000;x=seed;total=0
for i in range(iterations):x=(x*17+(i&127))%1009;total=(total+x)%65521
body='''template<class T> struct base { int value; int read() const { return value; } };
template<class T> struct box : base<T> {
  typedef base<T> alias;
  box(int v=7) { this->value=v; }
  void swap(box& other) { int v=this->value; this->value=other.alias::value; other.alias::value=v; }
  int step(int i) { box((this->value*17+(i&127))%1009).swap(*this); return this->value; }
  void reset() { box().swap(*this); }
  int read(const box& other) const { return other.alias::read(); }
};
int probe(int x) { box<int> b(x); b.reset(); b.step(x); return b.read(b); }
'''
for n in [128,512,2048]:
 name='current'+str(n)
 source=''.join('namespace block%d {\n%s\n}\n'%(i,body) for i in range(n))
 source+='''int main(int argc,char**argv) { if(argc!=2)return 3;
block0::box<int> b(argv[1][0]-48); int total=0;
for(int i=0;i<%d;++i) { b.step(i); total=(total+b.read(b))%%65521; }
return b.value==%d && total==%d?0:1; }\n'''%(iterations,x,total)
 src=out/(name+'.cpp');src.write_text(source)
 reject=run([bins['A'],*r['flags'],src,'-o',out/'entry.o'],False);assert reject.returncode
 r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=seed,expected=[x,total],entry_rejection=dict(status=reject.returncode,stderr=reject.stderr))
 obj=out/(name+'.o');exe=out/name
 run([bins['B'],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,str(seed)])
 r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=textsize(obj),executable_text_bytes=textsize(exe))
 for mode in ['compile','runtime']:
  for trial in range(8):
   args=[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,str(seed)]
   r['runs'].append(dict(workload=name,mode=mode,trial=trial,**measured(args)));save()
  rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode]
  r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(v['wall_s'] for v in rows),range_s=[min(v['wall_s'] for v in rows),max(v['wall_s'] for v in rows)],peak_rss_kib=max(v['peak_rss_kib'] for v in rows));save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items());save()

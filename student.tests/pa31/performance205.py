#!/usr/bin/env python3
"""PA31 equivalent allocation A/A+ABBA and newly valid list/inheritance costs.
No timing comparison treats a rejecting/crashing entry compiler as equivalent.
"""
import hashlib,json,os,pathlib,resource,statistics,subprocess,sys,time
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,check=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 if check:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],launchers=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,args in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
 for trial in range(8):r['launchers'].append(dict(mode=mode,trial=trial,**measured(args)))
iterations=3000000;x=7;total=0
for i in range(iterations):x=(x*17+(i&127))%1009;total=(total+x)%65521
bodies={
'stream':'''#include <sstream>
#include <string>
''',
'allocation':'''template<class T> int step(T x,int i){int* p=new int[2];p[0]=x;p[1]=i&127;int n=(p[0]*17+p[1])%1009;delete[] p;return n;}
int invoke(int x,int i){return step(x,i);}
''',
'list':'''struct Box{int sum;Box(std::initializer_list<int> xs) noexcept:sum(0){for(int x:xs)sum+=x;}};
int receive(const Box& b) noexcept{return b.sum%1009;}
template<class T>int step(T x,int i){return receive({x*17,i&127});}
int invoke(int x,int i){return step(x,i);}
''',
'inherited':'''struct Base{int n;Base() noexcept:n(1){}};
struct Derived:Base{using Base::Base;Derived(Derived&&)=default;};
template<class T>int step(T x,int i){Derived d;return (x*17+(i&127)+d.n-1)%1009;}
int invoke(int x,int i){return step(x,i);}
'''}
for family,body in bodies.items():
 if family not in os.environ.get('PERF_FAMILIES','stream,allocation,list,inherited').split(','):continue
 for n in ([1] if family=='stream' else [64,256,1024]):
  name=family+str(n);src=out/(name+'.cpp')
  source=('#include <initializer_list>\n' if family=='list' else '')+''.join('namespace group%d{\n%s}\n'%(j,body) for j in range(n))
  source+='int main(int argc,char**argv){if(argc!=2)return 3;int x=argv[1][0]-48,total=0;for(int i=0;i<%d;++i){x=group0::invoke(x,i);total=(total+x)%%65521;}return x==%d&&total==%d?0:1;}\n'%(iterations,x,total)
  if family=='stream':
   source=body+'int main(int argc,char**argv){if(argc!=2)return 3;int total=0;for(int i=0;i<argc*100000;++i){std::ostringstream out;out<<argv[1];std::string s=out.str();if(s.size()!=1 || s[0]!=argv[1][0])return 4;total+=s[0];}return total==11000000?0:1;}\n'
  src.write_text(source);r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=200000 if family=='stream' else iterations,seed=7,expected=11000000 if family=='stream' else [x,total]);save()
  labels='AB' if family in ['allocation','stream'] else 'B'
  if family not in ['allocation','stream']:
   p=run([bins['A'],*r['flags'],src,'-o',out/'entry.o'],False)
   r['inputs'][name]['entry_failure']=dict(status=p.returncode,stderr=p.stderr);save();assert p.returncode
  images={};exes={}
  for label in labels:
   obj=out/(name+label+'.o');exe=out/(name+label)
   run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);exes[label]=exe
   images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),executable_text_bytes=size(exe))
  r['images'][name]=images;save()
  for mode in ['compile','runtime']:
   orders=['AAAA']+['ABBA']*6 if family in ['allocation','stream'] else ['BBBB']*2
   for block,order in enumerate(orders):
    for label in order:
     args=[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exes[label],'7']
     row=dict(workload=name,mode=mode,block=block,label=label,**measured(args))
     if mode=='compile':row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
     r['runs'].append(row);save()
   rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode];summary={}
   if family in ['allocation','stream']:
    aa=[v['wall_s'] for v in rows if not v['block']];ratios=[statistics.mean(v['wall_s'] for v in rows if v['label']=='B' and v['block']==b)/statistics.mean(v['wall_s'] for v in rows if v['label']=='A' and v['block']==b) for b in range(1,7)]
    summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
   for label in labels:
    samples=[v for v in rows if v['label']==label and (v['block'] or family not in ['allocation','stream'])]
    summary[label]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
   r['summary'].setdefault(name,{})[mode]=summary;save()
  print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

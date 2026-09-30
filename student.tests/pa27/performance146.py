#!/usr/bin/env python3
"""Focused naming benchmark: frozen A/A plus six ABBA blocks, no host boundary.
Usage: performance146.py OUT BEFORE AFTER
The required ABI spellings differ; both self-contained workloads execute the
same checked computation. This measures correctness costs, not an optimization.
"""
import hashlib,json,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',map(lambda p:pathlib.Path(p).resolve(),sys.argv[2:4])))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
 assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.decode().splitlines() if l.split() and l.split()[0].startswith('.text'))
count=1200;iterations=6000
values=[sum((x+n)%97 for n in range(count)) for x in range(64)]
expected=sum(values[i&63] for i in range(iterations))%1009
source=out/'naming.cpp'
source.write_text('''namespace std {
template<class C> struct char_traits {};
template<class C,class Tr> struct basic_ostream {};
template<class T> struct allocator {};
}
template<int N> struct Tag {};
template<int N> __attribute__((noinline)) int item(std::allocator<Tag<N> >*,std::basic_ostream<char,std::char_traits<char> >*,int x){return (x+N)%97;}
int demand(int x){int s=0;
'''+''.join('s+=item<%d>(nullptr,nullptr,x);\n'%n for n in range(count))+'''return s;}
int main(int argc,char**){int s=0;for(int i=0;i<argc*6000;++i)s=(s+demand(i&63))%1009;
return s=='''+str(expected)+'?0:1;}\n')
flags=['-O0','-c','--stats'];result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=flags,input_sha256=sha(source),input_bytes=source.stat().st_size,expected=expected,runs=[],images={},summary={})
for label,binary in bins.items():
 obj=out/(label+'.o');exe=out/label
 run([binary,*flags,source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 result['images'][label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_bytes=exe.stat().st_size,object_text_bytes=size(obj),executable_text_bytes=size(exe))
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ('compile','runtime'):
 for block,order in enumerate(['AAAA']+['ABBA']*6):
  for label in order:
   args=[bins[label],*flags,source,'-o',out/'measured.o'] if mode=='compile' else [out/label]
   start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);elapsed=time.perf_counter()-start
   result['runs'].append(dict(mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.decode().splitlines() if l.startswith('{')]))
   save()
 rows=[r for r in result['runs'] if r['mode']==mode]
 ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
 aa=[r['wall_s'] for r in rows if not r['block']]
 summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
 for label in 'AB':
  samples=[r for r in rows if r['label']==label and r['block']]
  summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
 result['summary'][mode]=summary;save();print(mode,json.dumps(summary),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in result['binaries'].items())

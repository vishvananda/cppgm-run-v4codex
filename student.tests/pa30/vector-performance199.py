#!/usr/bin/env python3
"""Checked runtime for newly supported vector operations; entry rejects input."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,success=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 if success:assert p.returncode==0,(args,p.stderr)
 return p
iterations=3000000;x=7;total=0
for i in range(iterations):x=(x*17+(i&127))%1009;total=(total+x)%65521
source='''typedef int V __attribute__((vector_size(8)));
volatile V observed;
int step(int x,int i) {
 V a=__builtin_ia32_vec_init_v2si(x*17,i&127);
 unsigned long long bits=(unsigned long long)a;
 observed=(V)bits;
 return (__builtin_ia32_vec_ext_v2si(observed,0)+__builtin_ia32_vec_ext_v2si(observed,1))%%1009;
}
int main(int argc,char** argv) {if(argc!=2)return 3;
int x=argv[1][0]-48,total=0;
for(int i=0;i<%d;++i){x=step(x,i);total=(total+x)%%65521;}
return x==%d && total==%d?0:1;}
'''%(iterations,x,total)
src=out/'vectors.cpp';src.write_text(source);obj=out/'vectors.o';exe=out/'vectors'
flags=['-O0','-c','--stats']
reject=run([bins['A'],*flags,src,'-o',out/'entry.o'],False);assert reject.returncode!=0
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=flags,affinity=affinity,
 input=dict(source=source,sha256=sha(src),iterations=iterations,seed=7,expected=[x,total],entry_rejection=dict(status=reject.returncode,stderr=reject.stderr)),runs=[],summary={})
run([bins['B'],*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
def textsize(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
r['image']=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=textsize(obj),executable_text_bytes=textsize(exe))
for mode in ['compile','runtime']:
 for trial in range(8):
  args=[bins['B'],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7']
  started=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
  row=dict(mode=mode,trial=trial,status=p.returncode,wall_s=time.perf_counter()-started,peak_rss_kib=int((out/'rss').read_text()),phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
  if mode=='compile':
   row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==r['image']['object_sha256']
  r['runs'].append(row);(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
 rows=[v for v in r['runs'] if v['mode']==mode]
 r['summary'][mode]=dict(median_s=statistics.median(v['wall_s'] for v in rows),range_s=[min(v['wall_s'] for v in rows),max(v['wall_s'] for v in rows)],peak_rss_kib=max(v['peak_rss_kib'] for v in rows))
 (out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())
print(json.dumps(r['summary']))

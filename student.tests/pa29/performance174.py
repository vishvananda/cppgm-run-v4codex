#!/usr/bin/env python3
"""Frozen audit A/A+ABBA comparisons and required-semantics scaling."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(v).resolve() for k,v in zip('AB',sys.argv[2:4])}
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 if not allow:assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
sources={};capability=set()
# Frozen inherited owner inputs, with their handoff hashes verified.
for stage,filename,names in [
    (171,'affected-performance',['selection1200','sequence2400','integer-pack2400']),
    (172,'affected-performance',['runtime1200','query24000']),
    (173,'closure-performance',['ordinary1200','closures1200','calls1200'])]:
    prior=json.loads((root/('student.tests/pa29/evidence%d/%s.json'%(stage,filename))).read_text())
    for name in names:
        record=prior['inputs'][name];p=pathlib.Path(record['path'])
        assert sha(p)==record['sha256'];sources[name]=p
for kind in ['discard','capture']:
    for n in [600,1200,2400]:
        name=kind+str(n);p=out/(name+'.cpp');sources[name]=p;capability.add(name)
        if kind=='discard':
            prefix='long copied,dead;struct V{int n;V(int x):n(x){} V(const volatile V& x){copied+=x.n;}~V(){++dead;}};\n'
            prefix+='template<class T,T...I>struct Seq{}; template<unsigned...I>void work(Seq<unsigned,I...>,volatile V& v){((static_cast<void>(I),v),...);}\n'
            p.write_text(prefix+'int main(int argc,char**){volatile V v(argc+1);for(int i=0;i<argc*'+str(20000000//n)+';++i)work(__make_integer_seq<Seq,unsigned,'+str(n)+'>{},v);return copied=='+str(2*n*(20000000//n))+'LL && dead=='+str(n*(20000000//n))+'LL?0:1;}\n')
        else:
            prefix='int copies;struct Shape{Shape(){}Shape(const Shape&){++copies;}};\n'
            prefix+='template<int N>long work(long x){Shape shape;auto f=[=]<class T>(T y){decltype(shape) local;return (y+N)%97;};return f(x); }\n'
            prefix+='long demanded(long x){long s=0;'+''.join('s+=work<%d>(x);'%i for i in range(n))+'return s;}\n'
            iterations=20000000;period=[(i+8)%97 for i in range(128)]
            checksum=sum(period)*(iterations//128)+sum(period[:iterations%128]);expected=sum((1+i)%97 for i in range(n))
            p.write_text(prefix+'int main(int argc,char**){if(demanded(argc)!='+str(expected)+' || copies)return 1;long sum=0;for(int i=0;i<argc*20000000;++i)sum+=work<7>((i&127)+argc);return sum=='+str(checksum)+'LL && !copies?0:2;}\n')

r=dict(binaries={k:dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size) for k,p in cc.items()},flags=flags,affinity=affinity,inputs={k:dict(path=str(p),sha256=sha(p)) for k,p in sources.items()},runs=[],images={},image_equality={},summary={},entry_checks={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);r['launcher'].append(time.perf_counter()-start)
for name,source in sources.items():
 labels='B' if name in capability else 'AB';images={};executables={}
 if name in capability:
  p=run([cc['A'],*flags,source,'-o',out/'entry.o'],True)
  record=dict(compile_status=p.returncode,stderr=p.stderr.decode())
  if not p.returncode:
   run(['g++',out/'entry.o','-o',out/'entry-check'])
   record['runtime_status']=run([out/'entry-check'],True).returncode
   assert record['runtime_status']!=0
  r['entry_checks'][name]=record
 for label in labels:
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([cc[label],*flags,source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe]);executables[label]=exe
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text_size(exe),object_bytes=obj.stat().st_size)
 r['images'][name]=images
 if labels=='AB':r['image_equality'][name]=images['A']==images['B']
 for mode in ['compile','runtime']:
  orders=['BBBB']*2 if name in capability else ['AAAA']+['ABBA']*6
  for block,order in enumerate(orders):
   for label in order:
    args=[cc[label],*flags,source,'-o',out/'measure.o'] if mode=='compile' else [executables[label]]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];s={}
  for label in labels:
   a=[x for x in rows if x['label']==label and (name in capability or x['block'])]
   s[label]=dict(median_s=statistics.median(x['wall_s'] for x in a),range_s=[min(x['wall_s'] for x in a),max(x['wall_s'] for x in a)],peak_rss_kib=max(x['peak_rss_kib'] for x in a))
  if name not in capability:
   ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
   aa=[x['wall_s'] for x in rows if not x['block']];s.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
  r['summary'].setdefault(name,{})[mode]=s;save()
 print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(cc[k])==v['sha256'] for k,v in r['binaries'].items());save()

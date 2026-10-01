#!/usr/bin/env python3
"""PA29 corrected-owner scaling; invalid entry output is never a timing baseline.
Usage: performance192.py OUT ENTRY FINAL. Set PERF_CPU for fixed affinity.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
def text_size(p):
 return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
result=dict(affinity=affinity,binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in binaries.items()},
 flags=['-O0','-c','--stats'],runtime_args=['3'],inputs={},entry_failures={},images={},launchers=[],runs=[],summary={},
 host_compiler=run(['g++','--version']).stdout,host_linker=run(['g++','--version']).stdout)
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,
  phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
for mode,args in [('compile',[binaries['B'],'--help']),('runtime',['/bin/true'])]:
 for i in range(8):result['launchers'].append(dict(mode=mode,index=i,**measured(args)))
save()
sources={}
for kind,typ in [('half','_Float16'),('quad','__float128')]:
 for n in [128,512,2048]:
  iterations=1000000
  prefix='using T=%s;\n'%typ
  prefix+='template<int N> T step(T x){return x+T(N&7)/T(8); }\n'
  prefix+='long demand(T x){long sum=0;\n'+''.join('sum+=long(step<%d>(x)*T(8));\n'%i for i in range(n))+'return sum;}\n'
  body='int main(int argc,char**argv){if(argc!=2)return 3;int seed=argv[1][0]-48;T x=seed;\n'
  body+='if(demand(x)!=%d)return 2;long sum=0;\n'%(n*24+sum(i&7 for i in range(n)))
  body+='for(int i=0;i<%d;++i){x=step<3>(x);x-=T(3)/T(8);sum+=long(x*T(8));}\n'%iterations
  body+='return sum==%d?0:1;}\n'%(24*iterations)
  sources[kind+str(n)]=dict(source=prefix+body,n=n,expected=24*iterations,family=kind,iterations=iterations)
for name,data in sources.items():
 source=out/(name+'.cpp');source.write_text(data['source'])
 result['inputs'][name]=dict(**data,sha256=sha(source))
 entry=run([binaries['A'],'-O0','-c','--stats',source,'-o',out/'entry.o'],False)
 result['entry_failures'][name]=dict(status=entry.returncode,stderr=entry.stderr)
 assert entry.returncode!=0,(name,'entry now supports input; use ABBA instead')
 obj=out/(name+'.o');exe=out/name
 run([binaries['B'],'-O0','-c','--stats',source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'3'])
 run(['g++','-std=gnu++11','-O0','-mno-avx',source,'-o',out/'host']);run([out/'host','3'])
 result['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,
  executable_bytes=exe.stat().st_size,object_text_bytes=text_size(obj),executable_text_bytes=text_size(exe),
  native_status=0,host_status=0)
 for mode in ['compile','runtime']:
  args=[binaries['B'],'-O0','-c','--stats',source,'-o',out/'measure.o'] if mode=='compile' else [exe,'3']
  for index in range(8):
   result['runs'].append(dict(workload=name,mode=mode,index=index,**measured(args)));save()
  rows=[r for r in result['runs'] if r['workload']==name and r['mode']==mode]
  result['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),
   range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows))
  save()
 print(name,json.dumps(result['summary'][name]),flush=True)
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())

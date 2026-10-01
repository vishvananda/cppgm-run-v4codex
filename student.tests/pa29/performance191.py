#!/usr/bin/env python3
"""PA29 corrected-owner scaling; invalid entry output is never a timing baseline.
Usage: performance191.py OUT ENTRY FINAL. Set PERF_CPU for fixed affinity.
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
 host_compiler=run(['clang++','--version']).stdout,host_linker=run(['g++','--version']).stdout)
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
def measured(args):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
 return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,
  phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
for mode,args in [('compile',[binaries['B'],'--help']),('runtime',['/bin/true'])]:
 for i in range(8):result['launchers'].append(dict(mode=mode,index=i,**measured(args)))
save()
sources={}
for n in [128,512,2048]:
 expected=[3,2,4,8]
 for i in range(n):
  for j in range(4):expected[j]^=(i*(j+1))&127
 iterations=2000000
 odd=[a^b for a,b in zip([3,2,4,8],[3,6,9,12])]
 checksum=(15 | 0)+(odd[0]|odd[1]|odd[2]|odd[3])
 prefix='using I=int __attribute__((ext_vector_type(4)));\n'
 prefix+='template<int N> I step(I x){I c{N&127,(N*2)&127,(N*3)&127,(N*4)&127};return x^c;}\n'
 prefix+='I demand(I x){\n'+''.join('x=step<%d>(x);\n'%i for i in range(n))+'return x;}\n'
 body='int main(int argc,char**argv){if(argc!=2)return 3;int seed=argv[1][0]-48;I x{seed,2,4,8};\n'
 body+='I got=demand(x),expected{%s};if(__builtin_reduce_or(got^expected))return 2;\n'%','.join(map(str,expected))
 body+='long sum=0;for(int i=0;i<%d;++i){x=step<3>(x);sum+=__builtin_reduce_or(x);}\n'%iterations
 body+='return sum==%d?0:1;}\n'%(checksum*(iterations//2))
 sources['demand'+str(n)]=dict(source=prefix+body,n=n,expected=checksum*(iterations//2),family='demand')
for n in [16,64,256]:
 iterations=4000000//n;expected=0
 for i in range(1,iterations+1):expected+=((3+i)|(2+2*i)|(4+3*i)|(8+4*i))&1023
 prefix='using I=int __attribute__((ext_vector_type(%d)));\n'%n
 prefix+=''.join('I add%d(I x){I d{1,2,3,4};return x+d;}\n'%i for i in range(128))
 body='int main(int argc,char**argv){if(argc!=2)return 3;int seed=argv[1][0]-48;I x{seed,2,4,8};long sum=0;\n'
 body+='for(int i=0;i<%d;++i){x=add0(x);sum+=__builtin_reduce_or(x)&1023;}\n'%iterations
 body+='return sum==%d?0:1;}\n'%expected
 sources['width'+str(n)]=dict(source=prefix+body,n=n,expected=expected,family='width',iterations=iterations)
for name,data in sources.items():
 source=out/(name+'.cpp');source.write_text(data['source'])
 result['inputs'][name]=dict(**data,sha256=sha(source))
 entry=run([binaries['A'],'-O0','-c','--stats',source,'-o',out/'entry.o'],False)
 result['entry_failures'][name]=dict(status=entry.returncode,stderr=entry.stderr)
 assert entry.returncode!=0,(name,'entry now supports input; use ABBA instead')
 obj=out/(name+'.o');exe=out/name
 run([binaries['B'],'-O0','-c','--stats',source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'3'])
 run(['clang++','-std=c++11','-O0','-mno-avx',source,'-o',out/'host']);run([out/'host','3'])
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

#!/usr/bin/env python3
"""New scalar capability costs; unsupported A is not a performance baseline."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
a,b=(pathlib.Path(p).resolve() for p in sys.argv[2:4]);affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 assert (p.returncode==0)==ok,(args,p.returncode,p.stderr.decode())
 return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in [('A',a),('B',b)]},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},unsupported_A={},launcher=[])
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
def measure(label,args,**fields):
 start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
 row=dict(workload=label,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')],**fields)
 result['runs'].append(row);save()
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
def compute(x):return x.bit_count()+64-x.bit_length()
iterations=6000000
checksum=sum(compute((i&65535)+1+7) for i in range(65536))*(iterations//65536)+sum(compute((i&65535)+1+7) for i in range(iterations%65536))
for n in [600,1200,2400]:
 src=out/('scalar'+str(n)+'.cpp')
 src.write_text('''template<int N> unsigned work(unsigned long long x) {return __builtin_popcountll(x+N)+__builtin_clzg(x+N,64);}
unsigned long long demanded(unsigned long long x){unsigned long long sum=0;
'''+''.join('sum+=work<%d>(x);\n'%i for i in range(n))+'''return sum;}
int main(int argc,char**){if(demanded(argc)!='''+str(sum(compute(i+1) for i in range(n)))+''')return 1;
unsigned long long sum=0;for(int i=0;i<argc*6000000;++i)sum+=work<7>((i&65535)+argc);
return sum=='''+str(checksum)+'''ULL?0:2;}
''')
 key='scalar'+str(n);result['inputs'][key]=dict(path=str(src),sha256=sha(src));obj=out/(key+'.o');exe=out/key
 p=run([a,*result['flags'],src,'-o',out/'bad.o'],False);result['unsupported_A'][key]=dict(status=p.returncode,stderr=p.stderr.decode())
 run([b,*result['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 result['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
 for mode in ['compile','runtime']:
  for i in range(8):measure(key,[b,*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe],mode=mode,sample=i)
for workload in result['inputs']:
 result['summary'][workload]={}
 for mode in ['compile','runtime']:
  rows=[r for r in result['runs'] if r['workload']==workload and r['mode']==mode]
  result['summary'][workload][mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows))
assert sha(a)==result['binaries']['A']['sha256'] and sha(b)==result['binaries']['B']['sha256'];save()
print(json.dumps(result['summary']),flush=True)

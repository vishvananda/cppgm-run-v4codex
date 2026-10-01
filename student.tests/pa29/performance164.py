#!/usr/bin/env python3
"""PA29 demanded function strings/query bounds: required semantic cost and scaling."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True);cc=pathlib.Path(sys.argv[2]).resolve()
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90);assert p.returncode==0,(args,p.returncode,p.stderr.decode());return p
def text_size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result=dict(binary=dict(path=str(cc),sha256=sha(cc)),flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launcher=[])
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for i in range(6):
 start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'/usr/bin/true']);result['launcher'].append(time.perf_counter()-start)
iterations=30000000
for kind in ['strings','queries']:
 for n in [600,1200,2400]:
  key=kind+str(n);src=out/(key+'.cpp')
  if kind=='strings':
   prefix='template<int N> int work(int x){const char* s=__PRETTY_FUNCTION__;return s[x&15];}\n'
   demanded=ord('n')*n
   text='int work(int) [N = 7]';period=[ord(text[i]) for i in range(16)]
  else:
   prefix='template<int N,int M> constexpr int suffix(const char(&)[N],const char(&)[M]){return M-1;}\ntemplate<int N> int work(int x){return (x&15)+sizeof(char[1+suffix(__PRETTY_FUNCTION__,"int work(int) [N = ")])-1;}\n'
   size=len('int work(int) [N = ');demanded=(1+size)*n;period=[i+size for i in range(16)]
  checksum=sum(period)*(iterations//16)+sum(period[:iterations%16])
  src.write_text(prefix+'long demanded(int x){long sum=0;\n'+''.join('sum+=work<%d>(x);\n'%i for i in range(n))+'return sum;}\nint main(int argc,char**){if(demanded(argc)!='+str(demanded)+')return 1;\nlong sum=0;for(int i=0;i<argc*30000000;++i)sum+=work<7>((i+argc-1)&15);\nreturn sum=='+str(checksum)+'LL?0:2;}\n')
  obj=out/(key+'.o');exe=out/key;run([cc,*result['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
  result['inputs'][key]=dict(path=str(src),sha256=sha(src));result['images'][key]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe))
  for mode in ['compile','runtime']:
   for sample in range(8):
    args=[cc,*result['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe]
    start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
    result['runs'].append(dict(workload=key,mode=mode,sample=sample,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]));save()
   rows=[r for r in result['runs'] if r['workload']==key and r['mode']==mode]
   result['summary'].setdefault(key,{})[mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
  print(key,json.dumps(result['summary'][key]),flush=True)
assert sha(cc)==result['binary']['sha256'];save()

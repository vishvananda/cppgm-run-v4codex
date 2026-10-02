#!/usr/bin/env python3
"""Frozen compiler/range/control A/B, A/A calibration, six ABBA blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
root=pathlib.Path(__file__).resolve().parents[2]
affinity=['taskset','-c',os.environ.get('PERF_CPU','2')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
 return p
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,runs=[],inputs={},images={},summary={},
 bounds=dict(loop_candidate_work='16*(I+O+E+1)',partial_slot_work='16*(I+O+B)',new_loop_ir_growth=0,runtime_helpers_per_unit=1,runtime_helper_instructions=4),
 diagnostic_targets=dict(compile_ratio=1.5,rss_ratio=1.5,small_runtime_ratio=1.25))
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in os.environ.get('RANGE_WORKLOADS','reference-large reference-small reference-empty zero-large pointer truth partial').split():
 isref=name.startswith('reference');iszero=name.startswith('zero');isrange=isref or iszero
 if isrange:
  fixture='500-fill-loop-byte-from-reference' if isref else '500-fill-loop-word-zero'
  if isref:
   body='''function @kernel0(%first : ptr, %n : i64, %value : ptr) -> void [binding=strong, linkage=c, no_inline=yes] {
block ^entry: %last = index i8 %first, %n
jump ^head
block ^head: %p = phi ptr [^entry: %first, ^body: %next]
%c = cmp ne ptr %p, %last
branch %c, ^body, ^done
block ^body: %v = load u8 %value
store u8 %v, %p
%next = index i8 %p, 1
jump ^head
block ^done: return void
}\n'''
  else:body=(root/f'pa32/tests/o1/{fixture}.t').read_text().replace('@zero_words','@kernel0')
  length=1024 if name.endswith('large') else 8 if name.endswith('small') else 0
  n=2000000 if name.endswith('large') else 40000000
  expected=(n//256*32640+(n%256)*(n%256-1)//2) if isref else 0
  main='''extern "C" void kernel0(unsigned char*,long%s);
int main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*%dL;
unsigned char buf[1024];long sum=0;for(long i=0;i<n;++i){
buf[3]=i&255;kernel0(buf,%d%s);sum+=buf[%d];}
return sum==%d?0:1;}
'''%(',unsigned char*' if isref else '',n//10,length if isref else length//4,',buf+3' if isref else '',3 if length==0 else length-1,expected)
 elif name=='pointer':
  body='''function @kernel0(%first : ptr, %last : ptr) -> ptr [no_inline=yes] {
block ^entry: jump ^head
block ^head: %p = phi ptr [^entry: %first, ^body: %next]
%c = cmp ne ptr %p, %last
branch %c, ^body, ^done
block ^body: %next = index i8 %p, -1
jump ^head
block ^done: return ptr %p
}
'''
  expected=40000000
  main='''extern "C" unsigned char* kernel0(unsigned char*,unsigned char*);
int main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*4000000L;
unsigned char buf[64];long sum=0;
for(long i=0;i<n;++i)sum+=kernel0(buf+(i&63),buf)==buf;
return sum==40000000?0:1;}
'''
 else:
  fixture=(root/'pa32/tests/o1/500-phi-integrity-survivors.t').read_text()
  target='repair_fold_target_phi' if name=='truth' else 'cross_slot_needed_phi'
  body='function @kernel0('+fixture.split('function @'+target+'(')[1].split('\n}')[0]+'\n}\n'
  
  n=60000000;expected=n//2*(33 if name=='truth' else 14)
  main='''extern "C" long kernel0(long);
extern "C" __attribute__((noinline)) void observe(long){}
int main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*6000000L;
long sum=0;for(long i=0;i<n;++i)sum+=kernel0((i&1)*256);
return sum==%d?0:1;}
'''%expected
 src=out/(name+'.lowir');src.write_text(('declare function @observe(%value : i64) -> void\n' if name=='partial' else '')+''.join(body.replace('@kernel0(',f'@kernel{k}(') for k in range(5000)))
 mainfile=out/(name+'.cpp');mainfile.write_text(main)
 flags=['-O1','-c'];args=[':']
 r['inputs'][name]=dict(lowir_sha256=sha(src),main_sha256=sha(mainfile),flags=flags,runtime_arguments=args,expected=expected)
 run([bins['A'],'-c','-O0',mainfile,'-o',out/'main.o']);images={}
 for label in 'AB':
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([bins[label],*flags,src,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,*args])
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe),
   telemetry=[json.loads(s) for s in run([bins[label],*flags,'--stats',src,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
  (out/(name+label+'.asm')).write_text(run(['objdump','-d','--disassemble=kernel0',obj]).stdout)
 r['images'][name]=images;save()
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:
    command=[bins[label],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),*args]
    start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*command])
    row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode)
    if mode=='compile':
     row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
    r['runs'].append(row);save()
  rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];aa=[x['wall_s'] for x in rows if not x['block']]
  ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
  summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in 'AB':
   samples=[x for x in rows if x['block'] and x['label']==label]
   summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
  r['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

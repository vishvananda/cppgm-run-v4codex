#!/usr/bin/env python3
"""Frozen memory A/B: checked dynamic kernels, A/A and six ABBA blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert r.returncode==0,(args,r.returncode,r.stderr)
 return r
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
def kernel(name,mode):
 head=f'function @{name}(%p : ptr [alias=noalias], %q : ptr [alias=noalias]) -> i64 [binding=strong, linkage=c, no_inline=yes] {{\n'
 if mode=='loads':
  body=['block ^entry:','%s0 = const i64 0']
  for n in range(16):body += [f'%v{n} = load i64 %p',f'%s{n+1} = binary add i64 %s{n}, %v{n}']
  body += ['return i64 %s16']
 elif mode=='copies':
  body=['block ^entry:']
  for n in range(3):body += [f'%p{n} = index i8 [projection=field] %p, {8*n}',f'%q{n} = index i8 [projection=field] %q, {8*n}',f'%v{n} = load i64 %p{n}',f'store i64 %v{n}, %q{n}']
  body += ['%last = load i64 %q2','return i64 %last']
 elif mode in ['conditional','private','private-loads']:
  body='''block ^entry:
 %p8 = index i8 %p, 8
 %x = load i64 %p
 %y = load i64 %p8
 %less = cmp lt i64 %x, %y
 branch %less, ^a, ^b
block ^a: jump ^end
block ^b: jump ^end
block ^end:
 %ptr = phi ptr [^a: %p, ^b: %p8]
 %value = load i64 %ptr
 return i64 %value'''.splitlines()
 else:
  body='''block ^entry:
 %flag = load u8 %p
 %long = binary and u8 %flag, 1
 %test = cmp ne u8 %long, 0
 branch %test, ^a, ^b
block ^a:
 %p8 = index i8 [projection=field] %p, 8
 %size = load i64 %p8
 jump ^end
block ^b:
 %half = binary ushr u8 %flag, 1
 %small = convert zext i64 u8 %half
 jump ^end
block ^end:
 %first = phi i64 [^a: %size, ^b: %small]
 %flag2 = load u8 %p
 %long2 = binary and u8 %flag2, 1
 %test2 = cmp ne u8 %long2, 0
 branch %test2, ^a2, ^b2
block ^a2:
 %p82 = index i8 [projection=field] %p, 8
 %size2 = load i64 %p82
 jump ^end2
block ^b2:
 %half2 = binary ushr u8 %flag2, 1
 %small2 = convert zext i64 u8 %half2
 jump ^end2
block ^end2:
 %second = phi i64 [^a2: %size2, ^b2: %small2]
 %twice = binary add i64 %first, %first
 %result = binary add i64 %twice, %second
 return i64 %result'''.splitlines()
 if mode=='private-loads':
  head += 'slot $x : i64\nslot $y : i64\n'
  at=body.index(' %less = cmp lt i64 %x, %y')
  body[at:at]=[' store i64 %x, $x',' store i64 %y, $y',' %ax = addr $x',' %ay = addr $y']
  body=[line.replace('[^a: %p, ^b: %p8]','[^a: %ax, ^b: %ay]') for line in body]
 if mode=='private':
  head = f'function @{name}(%x : i64, %y : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {{\nslot $x : i64\nslot $y : i64\n'
  body=[line for line in body if not any(t in line for t in ['%p8 =','%x = load','%y = load'])]
  at=body.index(' %less = cmp lt i64 %x, %y')
  body[at:at]=[' store i64 %x, $x',' store i64 %y, $y',' %ax = addr $x',' %ay = addr $y']
  body=[line.replace('[^a: %p, ^b: %p8]','[^a: %ax, ^b: %ay]') for line in body]
 return head+'\n'.join(body)+'\n}\n'
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,runs=[],inputs={},images={},summary={},
 diagnostic_targets=dict(compiler_ratio=1.5,rss_ratio=1.5,text_ratio=1.05),
 enforced_bounds=dict(memory_cells=32,work='128*(I+O+E+1), plus linear census and bounded dominance',diamond_window=16,comparison_steps=64,copy_pieces=16,copy_bytes=128,ir_growth=0))
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in ['loads','conditional','private','private-loads','copies','diamonds']:
 source=out/(name+'.lowir');source.write_text(''.join(kernel('kernel'+str(n),name) for n in range(1800)))
 vals=[]
 for i in range(64):
  x,y,z=i&31,(i+17)&63,(i+11)&63
  vals.append({'loads':16*x,'conditional':min(x,y),'private':min(x,y),'private-loads':min(x,y),'copies':z,'diamonds':3*(y if x&1 else x//2)}[name])
 expected=sum(vals)*(12000000//64)
 main=out/(name+'.cpp');main.write_text('''extern "C" long kernel0(long*,long*);
int main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*1000000L;
long p[3],q[3];long sum=0;
for(long i=0;i<n;++i){p[0]=i&31;p[1]=(i+17)&63;p[2]=(i+11)&63;sum+=kernel0(p,q);}
return sum==%d?0:1;}
'''%expected)
 if name=='private':main.write_text(main.read_text().replace('kernel0(long*,long*)','kernel0(long,long)').replace('kernel0(p,q)','kernel0(p[0],p[1])'))
 r['inputs'][name]=dict(lowir_sha256=sha(source),main_sha256=sha(main),expected=expected,runtime_arguments=['<'],flags=['-O1','-c'])
 run([bins['A'],'-c','-O0',main,'-o',out/'main.o']);images={}
 for label in 'AB':
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([bins[label],*r['inputs'][name]['flags'],source,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'<'])
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe),
   telemetry=[json.loads(s) for s in run([bins[label],*r['inputs'][name]['flags'],'--stats',source,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
  (out/(name+label+'.asm')).write_text(run(['objdump','-d','--disassemble=kernel0',obj]).stdout)
 r['images'][name]=images
 for mode in ['compile','runtime']:
  for block,order in enumerate(['AAAA']+['ABBA']*6):
   for label in order:
    args=[bins[label],*r['inputs'][name]['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),'<']
    start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
    row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode)
    if mode=='compile':
     row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
    r['runs'].append(row);save()
  rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode]
  aa=[v['wall_s'] for v in rows if not v['block']]
  ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
  summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
  for label in 'AB':
   samples=[v for v in rows if v['block'] and v['label']==label]
   summary[label]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
  r['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

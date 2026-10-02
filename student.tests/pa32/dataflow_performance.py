#!/usr/bin/env python3
"""Frozen same-level A/B, A/A calibration and six ABBA wall/RSS blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(args,p.returncode,p.stderr)
    return p
def size(p): return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
def slot_kernel(name):
    return f'''function @{name}(%n : i64, %seed : i64) -> i64 [binding=strong, linkage=c] {{
 slot $i : i64
 slot $acc : i64
 block ^entry:
  store i64 0, $i
  store i64 %seed, $acc
  jump ^head
 block ^head:
  %i = load i64 $i
  %more = cmp lt i64 %i, %n
  branch %more, ^body, ^exit
 block ^body:
  %acc = load i64 $acc
  %wide = binary mul i64 %acc, 17
  %extra = binary and i64 %i, 127
  %sum = binary add i64 %wide, %extra
  %value = binary mod i64 %sum, 1009
  store i64 %value, $acc
  %next = binary add i64 %i, 1
  store i64 %next, $i
  jump ^head
 block ^exit:
  %result = load i64 $acc
  return i64 %result
}}
'''
def dominance_kernel(name):
    return f'''function @{name}(%n : i64, %seed : i64) -> i64 [binding=strong, linkage=c] {{
 block ^entry:
  jump ^head
 block ^head:
  %i = phi i64 [^entry: 0, ^tail: %next]
  %acc = phi i64 [^entry: %seed, ^tail: %value]
  %more = cmp lt i64 %i, %n
  branch %more, ^body, ^exit
 block ^body:
  %wide = binary mul i64 %acc, 17
  %odd = binary and i64 %i, 1
  branch %odd, ^left, ^right
 block ^left:
  %left = binary mul i64 %acc, 17
  jump ^tail
 block ^right:
  %right = binary mul i64 %acc, 17
  jump ^tail
 block ^tail:
  %chosen = phi i64 [^left: %left, ^right: %right]
  %twice = binary add i64 %wide, %chosen
  %extra = binary and i64 %i, 127
  %sum = binary add i64 %twice, %extra
  %value = binary mod i64 %sum, 1009
  %next = binary add i64 %i, 1
  jump ^head
 block ^exit:
  return i64 %acc
}}
'''
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,
 flags=['-O1','-c'],runs=[],inputs={},images={},summary={},budgets=dict(compiler_ratio=2.0,rss_ratio=1.75,text_ratio=1.0))
def save(): (out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name,kernel,factor in [('slots',slot_kernel,17),('dominance',dominance_kernel,34)]:
    source=out/(name+'.lowir');source.write_text(''.join(kernel('kernel'+str(n)) for n in range(1200)))
    expected=7
    for i in range(6000000): expected=(expected*factor+(i&127))%1009
    main=out/(name+'.cpp');main.write_text('extern "C" long kernel0(long,long);\nint main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*1000000L;return kernel0(n,7)==%d?0:1;}\n'%expected)
    r['inputs'][name]=dict(lowir_sha256=sha(source),main_sha256=sha(main),expected=expected,runtime_arguments=['6'])
    run([bins['A'],'-c','-O0',main,'-o',out/'main.o'])
    images={}
    for label in 'AB':
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([bins[label],*r['flags'],source,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'6'])
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=size(exe),object_text_bytes=size(obj),
            telemetry=[json.loads(s) for s in run([bins[label],*r['flags'],'--stats',source,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
    r['images'][name]=images
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                args=[bins[label],*r['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),'6']
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

#!/usr/bin/env python3
"""Frozen object-copy A/B; A/A then six ABBA wall/RSS and runtime blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert r.returncode==0,(args,r.returncode,r.stderr)
    return r
def size(p): return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
def kernel(name,export):
    s=f'''function @{name}(%n : i64, %seed : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {{
 slot $a : obj<16x8>
 slot $b : obj<16x8>
 slot $c : obj<16x8>
 block ^entry:
  jump ^head
 block ^head:
  %i = phi i64 [^entry: 0, ^body: %next]
  %acc = phi i64 [^entry: %seed, ^body: %result]
  %more = cmp lt i64 %i, %n
  branch %more, ^body, ^exit
 block ^body:
  %a = addr $a
  %a8 = index i8 [projection=field] %a, 8
  %extra = binary and i64 %i, 127
  store i64 %acc, %a
  store i64 %extra, %a8
  %b = addr $b
  copyobj 16x8 %a, %b
  %c = addr $c
  copyobj 16x8 %b, %c
'''
    if export: s+='  %sum = call i64 @observe(%c)\n'
    else: s+='''  %low = load i64 %c
  %c8 = index i8 [projection=field] %c, 8
  %high = load i64 %c8
  %wide = binary mul i64 %low, 17
  %sum = binary add i64 %wide, %high
'''
    return s+'''  %result = binary mod i64 %sum, 1009
  %next = binary add i64 %i, 1
  jump ^head
 block ^exit:
  return i64 %acc
}
'''
observer='''function @observe(%p : ptr) -> i64 [no_inline=yes] {
 block ^entry:
 %a = load i64 %p
 %p8 = index i8 [projection=field] %p, 8
 %b = load i64 %p8
 %wide = binary mul i64 %a, 17
 %sum = binary add i64 %wide, %b
 return i64 %sum
}
'''
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,
 flags=['-O1','-c'],runs=[],inputs={},images={},summary={},
 diagnostic_targets=dict(compiler_ratio=1.5,rss_ratio=1.5,text_ratio=1.0),
 enforced_bounds=dict(object_bytes=64,fields_per_object=16,split_invocations=2,added_instructions_per_function='<=8*(input instructions+1) per invocation'))
def save(): (out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
expected=7
for i in range(12000000): expected=(expected*17+(i&127))%1009
for name,export in [('copies',False),('export',True)]:
    source=out/(name+'.lowir');source.write_text((observer if export else '')+''.join(kernel('kernel'+str(n),export) for n in range(1200)))
    main=out/(name+'.cpp');main.write_text('extern "C" long kernel0(long,long);\nint main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*2000000L;return kernel0(n,7)==%d?0:1;}\n'%expected)
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

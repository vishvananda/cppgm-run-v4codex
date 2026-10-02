#!/usr/bin/env python3
"""Frozen O0/O1 scalar workload: A/A calibration, six ABBA wall/RSS blocks.
Compiler and executable measurements are separate; arguments and checked
results keep the loop live. g++ is used only to link compiler-produced objects.
"""
import hashlib, json, os, pathlib, statistics, subprocess, sys, time
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
binary = pathlib.Path(sys.argv[2]).resolve()
affinity = ['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r = subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert r.returncode == 0,(args,r.returncode,r.stderr)
    return r
def size(p): return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
def kernel(name):
    lines = [f'function @{name}(%n : i64, %seed : i64) -> i64 [binding=strong, linkage=c] {{',
        ' block ^entry:', ' jump ^head', ' block ^head:',
        ' %i = phi i64 [^entry: 0, ^body: %next]', ' %acc = phi i64 [^entry: %seed, ^body: %value]',
        ' %more = cmp lt i64 %i, %n',' branch %more, ^body, ^exit',' block ^body:']
    for n in range(12): lines += [f' %m{n} = binary mul i64 %acc, 17']
    lines += [' %sum0 = copy i64 %m0']
    for n in range(1,12): lines += [f' %sum{n} = binary add i64 %sum{n-1}, %m{n}']
    lines += [' %extra = binary and i64 %i, 127',' %wide = binary add i64 %sum11, %extra',
        ' %value = binary mod i64 %wide, 1009',' %next = binary add i64 %i, 1',' jump ^head',
        ' block ^exit:',' return i64 %acc','}']
    return '\n'.join(lines)+'\n'
source=out/'scalar.lowir'; source.write_text(''.join(kernel('kernel'+str(n)) for n in range(600)))
expected=7
for i in range(3000000): expected=(expected*17*12+(i&127))%1009
main=out/'main.cpp'; main.write_text('extern "C" long kernel0(long,long);\nint main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*1000000L;return kernel0(n,7)==%d?0:1;}\n'%expected)
result=dict(binary=dict(path=str(binary),sha256=sha(binary)),flags={'A':['-O0','-c'],'B':['-O1','-c']},
    affinity=affinity,inputs={p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in [source,main]},
    runtime_arguments=['3'],expected=expected,runs=[],summary={},images={})
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
run([binary,'-c','-O0',main,'-o',out/'main.o'])
for label in 'AB':
    obj=out/(label+'.o');exe=out/label
    run([binary,*result['flags'][label],source,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'3'])
    result['images'][label]=dict(object_sha256=sha(obj),exe_sha256=sha(exe),text_bytes=size(exe),object_text_bytes=size(obj))
    result['images'][label]['telemetry']=[json.loads(s) for s in run([binary,*result['flags'][label],'--stats',source,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')]
for mode in ['compile','runtime']:
    for block,order in enumerate(['AAAA']+['ABBA']*6):
        for label in order:
            args=[binary,*result['flags'][label],source,'-o',out/'measure.o'] if mode=='compile' else [out/label,'3']
            start=time.perf_counter();r=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
            row=dict(mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=r.returncode)
            if mode=='compile':
                row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==result['images'][label]['object_sha256']
            result['runs'].append(row);save()
    rows=[r for r in result['runs'] if r['mode']==mode]
    aa=[r['wall_s'] for r in rows if r['block']==0]
    ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
    summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
    for label in 'AB':
        samples=[r for r in rows if r['block'] and r['label']==label]
        summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
    result['summary'][mode]=summary;save();print(mode,json.dumps(summary),flush=True)
assert sha(binary)==result['binary']['sha256']

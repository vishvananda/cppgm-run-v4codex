#!/usr/bin/env python3
"""Frozen equivalent owner A/A and ABBA, preserving inherited exact workloads.
Usage: performance194.py OUT ENTRY FINAL [PERF_CPU in environment].
The signaling-NaN repair changes literal bits, not instruction counts. These
already-correct source inputs measure its shared encoding owner's other users.
"""
import hashlib, json, os, pathlib, statistics, subprocess, sys, time
here = pathlib.Path(__file__).resolve().parent
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
bins = dict(zip('AB', [pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity = ['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p = subprocess.run(list(map(str,args)), capture_output=True, text=True, timeout=90)
    assert p.returncode == 0, (args,p.returncode,p.stderr)
    return p
r = dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},
    flags=['-O0','-c','--stats'], affinity=affinity, inputs={}, images={}, runs=[], launchers=[], summary={})
def save(): (out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def measure(args):
    start=time.perf_counter(); p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
    return dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),
        status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
def size(p):
    return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for mode,args in [('compile',[bins['B'],'--help']),('runtime',['/bin/true'])]:
    for i in range(8): r['launchers'].append(dict(mode=mode,index=i,**measure(args)))
for stage,name,arguments in [(191,'demand512',['3']),(192,'half512',['3']),
                              (192,'quad512',['3']),(193,'repeated1024',['7'])]:
    inherited=json.loads((here/('evidence'+str(stage))/'owner-performance.json').read_text())
    source=out/(name+'.cpp'); source.write_text(inherited['inputs'][name]['source'])
    assert sha(source)==inherited['inputs'][name]['sha256']
    r['inputs'][name]=dict(inherited['inputs'][name],evidence=stage,runtime_args=arguments)
    images={}
    for label in 'AB':
        obj=out/(name+label+'.o'); exe=out/(name+label)
        run([bins[label],*r['flags'],source,'-o',obj]); run(['g++',obj,'-o',exe]); run([exe,*arguments])
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),
            object_text_bytes=size(obj),executable_text_bytes=size(exe))
    assert images['A']==images['B'], images
    r['images'][name]=images
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                args=[bins[label],*r['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),*arguments]
                r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,**measure(args))); save()
        rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode]
        summary={}
        for label in 'AB':
            values=[v for v in rows if v['block'] and v['label']==label]
            summary[label]=dict(median_s=statistics.median(v['wall_s'] for v in values),
                range_s=[min(v['wall_s'] for v in values),max(v['wall_s'] for v in values)],
                peak_rss_kib=max(v['peak_rss_kib'] for v in values))
        ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/
                statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
        aa=[v['wall_s'] for v in rows if not v['block']]
        summary.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),
            paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
        r['summary'].setdefault(name,{})[mode]=summary; save()
    print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

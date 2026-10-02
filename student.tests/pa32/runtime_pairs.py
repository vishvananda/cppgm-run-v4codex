#!/usr/bin/env python3
"""Longer runtime-only diagnosis on frozen, already checked executables."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]);bins=dict(zip('AB',map(pathlib.Path,sys.argv[2:4])))
cpu=os.environ.get('PERF_CPU','2');args=sys.argv[4:]
r=dict(binaries={k:dict(path=str(p),sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for k,p in bins.items()},cpu=cpu,args=args,repetitions=8,runs=[])
for block,order in enumerate(['AAAA']+['ABBA']*6):
    for label in order:
        start=time.perf_counter()
        for iteration in range(8): subprocess.run(['taskset','-c',cpu,str(bins[label]),*args],check=True)
        r['runs'].append(dict(block=block,label=label,wall_s=time.perf_counter()-start))
ratios=[statistics.mean(v['wall_s'] for v in r['runs'] if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in r['runs'] if v['block']==b and v['label']=='A') for b in range(1,7)]
aa=[v['wall_s'] for v in r['runs'] if not v['block']]
r['summary']=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
out.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r['summary']),flush=True)

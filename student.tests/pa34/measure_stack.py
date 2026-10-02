#!/usr/bin/env python3
"""Frozen A/A and ABBA diagnosis of the compiler's long call-chain workload.

The 64 MiB stack is diagnostic, allowing both baseline binaries to finish.
Canonical validation must still run with the original default stack limit.
"""
import hashlib
import json
import pathlib
import resource
import statistics
import subprocess
import sys
import time

out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
bins = dict(zip('AB', (pathlib.Path(p).resolve() for p in sys.argv[2:4])))
source = pathlib.Path('dev/src/semantic/output.cpp').resolve()
flags = ['-O3','-c','--stats','-I'+str(pathlib.Path('dev/src').resolve())]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def stack(): resource.setrlimit(resource.RLIMIT_STACK, (64*1024*1024, resource.getrlimit(resource.RLIMIT_STACK)[1]))
result = dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},
              source=dict(path=str(source),sha256=sha(source)), flags=flags,
              cpu=2, stack_bytes=64*1024*1024, runs=[], images={}, summary={},
              runtime='compiler component object; no generated executable entry')
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
work = None
for block,order in enumerate(['AAAA']+['ABBA']*6):
    for label in order:
        obj=out/'measure.o'
        cmd=['/usr/bin/time','-f','%M','-o',out/'rss','taskset','-c','2',bins[label],*flags,source,'-o',obj]
        start=time.perf_counter();p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,preexec_fn=stack,timeout=120)
        row=dict(block=block,label=label,status=p.returncode,wall_s=time.perf_counter()-start,
                 peak_rss_kib=int((out/'rss').read_text()),stderr=p.stderr)
        result['runs'].append(row); save(); assert p.returncode==0,row
        row['object_sha256']=sha(obj)
        counts=[{k:v for k,v in json.loads(s).items() if not k.endswith('_ms') and not k.endswith('_rss_kib')}
                for s in p.stderr.splitlines() if s.startswith('{')]
        if work is None: work=counts
        assert work==counts,('work',label)
        if label not in result['images']:
            text=subprocess.check_output(['size','-A',obj],text=True)
            result['images'][label]=dict(sha256=sha(obj),text_bytes=sum(int(s.split()[1]) for s in text.splitlines() if s.split() and s.split()[0].startswith('.text')))
        assert row['object_sha256']==result['images']['A']['sha256']
ratios=[statistics.mean(r['wall_s'] for r in result['runs'] if r['block']==b and r['label']=='B')/
        statistics.mean(r['wall_s'] for r in result['runs'] if r['block']==b and r['label']=='A') for b in range(1,7)]
result['summary']=dict(paired_ratios=ratios,median_ratio=statistics.median(ratios),
                      aa_range_s=[min(r['wall_s'] for r in result['runs'] if not r['block']),max(r['wall_s'] for r in result['runs'] if not r['block'])])
for label in 'AB':
    rows=[r for r in result['runs'] if r['block'] and r['label']==label]
    result['summary'][label]=dict(median_s=statistics.median(r['wall_s'] for r in rows),
        range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows))
save(); print(json.dumps(result['summary']),flush=True)
assert all(sha(p)==result['binaries'][k]['sha256'] for k,p in bins.items())

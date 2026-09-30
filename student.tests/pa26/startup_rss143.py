#!/usr/bin/env python3
"""Measure compiler-only RSS: Python's child-tree maximum includes fork overhead."""
import json,pathlib,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();p=out/'startup.json';result=json.loads(p.read_text())
assert 'direct_rss_runs' not in result
result['rss_policy']='Batch child-tree RSS includes Python fork memory; direct /usr/bin/time samples measure compiler RSS.'
result['direct_rss_runs']=[]
for block,order in enumerate(['AAAA']+['ABBA']*6):
    for label in order:
        args=[result['binaries'][label]['path'],'-O0','-c','-o',str(out/'measure.o'),str(out/'short.cpp')]
        start=time.perf_counter();subprocess.run(['/usr/bin/time','-f','%M','-o',str(out/'rss.txt'),*args],check=True,capture_output=True)
        result['direct_rss_runs'].append({'label':label,'block':block,'wall_s':time.perf_counter()-start,'peak_rss_kib':int((out/'rss.txt').read_text())})
for k in 'AB': result['summary'][k]['peak_compiler_rss_kib']=max(r['peak_rss_kib'] for r in result['direct_rss_runs'] if r['label']==k)
p.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result['summary'],indent=2))

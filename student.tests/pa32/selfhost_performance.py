#!/usr/bin/env python3
"""Compile a fixed compiler-owned C++ source at O0 with frozen A/B binaries."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
source=pathlib.Path('dev/src/lowir/folding.cpp').resolve()
flags=['-O0','-c','-I'+str(pathlib.Path('dev/src').resolve())]
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.returncode,r.stderr)
 return r
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=flags,affinity=affinity,
 inputs={str(source):sha(source)},runs=[],images={},summary={},runtime='not applicable: compiler component object, no executable entry')
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for label in 'AB':
 obj=out/(label+'.o');run([bins[label],*flags,source,'-o',obj]);r['images'][label]=dict(sha256=sha(obj),text_bytes=size(obj))
for block,order in enumerate(['AAAA']+['ABBA']*6):
 for label in order:
  start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,bins[label],*flags,source,'-o',out/'measure.o'])
  row=dict(block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,object_sha256=sha(out/'measure.o'))
  assert row['object_sha256']==r['images'][label]['sha256'];r['runs'].append(row);save()
aa=[v['wall_s'] for v in r['runs'] if not v['block']]
ratios=[statistics.mean(v['wall_s'] for v in r['runs'] if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in r['runs'] if v['block']==b and v['label']=='A') for b in range(1,7)]
r['summary']=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
for label in 'AB':
 samples=[v for v in r['runs'] if v['block'] and v['label']==label]
 r['summary'][label]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
save();print(json.dumps(r['summary']),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

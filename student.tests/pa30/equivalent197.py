#!/usr/bin/env python3
"""Large namespace control accepted by both binaries; same input for each.
Use one shared declaration instead of distinct aliases, to diagnose large-owner
latency without timing the entry compiler's rejection as a supposed speedup.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',map(lambda p:pathlib.Path(p).resolve(),sys.argv[2:4])))
owner=json.loads(pathlib.Path(sys.argv[4]).read_text())
source=owner['inputs']['lookup2048']['source'].replace('namespace origin { struct object','namespace origin { typedef const int number; struct object')
source=source.replace('typedef origin::object object;','using origin::object;').replace('using object = origin::object;','using origin::object;')
source=source.replace('typedef const int number; namespace sub = origin::sub;','using origin::number;').replace('using number = const int; namespace sub = origin::sub;','using origin::number;')
src=out/'shared-declarations.cpp';src.write_text(source)
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
 assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},source=source,input_sha256=sha(src),affinity=affinity,flags=['-O0','-c','--stats'],runs=[],images={},summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def textsize(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
exes={}
for k,b in bins.items():
 obj=out/'program.o';exe=out/k
 run([b,*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7']);exes[k]=exe
 r['images'][k]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=textsize(obj),executable_text_bytes=textsize(exe))
assert r['images']['A']==r['images']['B'];save()
for mode in ['compile','runtime']:
 for block,order in enumerate(['AAAA']+['ABBA']*6):
  for k in order:
   args=[bins[k],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exes[k],'7']
   start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
   r['runs'].append(dict(mode=mode,block=block,label=k,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')]))
   save()
 rows=[v for v in r['runs'] if v['mode']==mode];aa=[v['wall_s'] for v in rows if v['block']==0]
 ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
 r['summary'][mode]=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
 for k in bins:
  samples=[v for v in rows if v['block'] and v['label']==k]
  r['summary'][mode][k]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
 save();print(mode,r['summary'][mode],flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

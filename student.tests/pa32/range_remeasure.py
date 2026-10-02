#!/usr/bin/env python3
"""Bind frozen runtime observations to identical final objects; remeasure compiler."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
previous=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[3:5])))
old=json.loads((previous/'performance.json').read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stderr)
 return p
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},runtime_evidence=dict(path=str(previous/'performance.json'),sha256=sha(previous/'performance.json')),affinity=old['affinity'],inputs=old['inputs'],images={},runs=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name,inputs in old['inputs'].items():
 src=previous/(name+'.lowir');main=previous/(name+'.cpp');assert sha(src)==inputs['lowir_sha256'] and sha(main)==inputs['main_sha256']
 run([bins['A'],'-c','-O0',main,'-o',out/'main.o']);images={}
 for label in 'AB':
  obj=out/(name+label+'.o');exe=out/(name+label)
  run([bins[label],*inputs['flags'],src,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,*inputs['runtime_arguments']])
  assert sha(obj)==old['images'][name][label]['object_sha256'],(name,label,'object changed')
  assert sha(exe)==old['images'][name][label]['executable_sha256'],(name,label,'executable changed')
  images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=old['images'][name][label]['object_text_bytes'],text_bytes=old['images'][name][label]['text_bytes'])
 r['images'][name]=images;save()
 for block,order in enumerate(['AAAA']+['ABBA']*6):
  for label in order:
   start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*r['affinity'],bins[label],*inputs['flags'],src,'-o',out/'measure.o'])
   row=dict(workload=name,mode='compile',block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode,object_sha256=sha(out/'measure.o'))
   assert row['object_sha256']==images[label]['object_sha256'];r['runs'].append(row);save()
 rows=[x for x in r['runs'] if x['workload']==name];aa=[x['wall_s'] for x in rows if not x['block']]
 ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
 summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
 for label in 'AB':
  samples=[x for x in rows if x['block'] and x['label']==label]
  summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
 r['summary'][name]=dict(compile=summary,runtime=old['summary'][name]['runtime']);save();print(name,summary['paired_ratio_median'],flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())

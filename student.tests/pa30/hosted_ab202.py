#!/usr/bin/env python3
"""Equivalent heavy headers across the audit repairs, with calibrated ABBA."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(x).resolve() for x in sys.argv[2:4]]));affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,flags=['-O0','-c','--stats'],inputs={},images={},runs=[],summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in ['600-hosted-fstream-stream-compile','700-libstdcxx-regex-compiler-member-alias-call']:
 src=root/f'pa30/tests/compile/{name}.t';r['inputs'][name]=dict(path=str(src.relative_to(root)),sha256=sha(src));images={}
 for block,order in enumerate(['AAAA']+['ABBA']*6):
  for label in order:
   obj=out/(name+label+'.o');args=['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,bins[label],*r['flags'],src,'-o',obj]
   start=time.perf_counter();p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
   row=dict(workload=name,label=label,block=block,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,stderr=p.stderr,object_sha256=sha(obj) if obj.exists() else None)
   r['runs'].append(row);save();assert p.returncode==0,row
   if label in images:assert images[label]['object_sha256']==sha(obj)
   else:
    sizes=subprocess.check_output(['size','-A',obj],text=True)
    images[label]=dict(object_sha256=sha(obj),object_text_bytes=sum(int(s.split()[1]) for s in sizes.splitlines() if s.split() and s.split()[0].startswith('.text')))
   r['images'][name]=images;save()
 rows=[v for v in r['runs'] if v['workload']==name];ratios=[statistics.mean(v['wall_s'] for v in rows if v['label']=='B' and v['block']==i)/statistics.mean(v['wall_s'] for v in rows if v['label']=='A' and v['block']==i) for i in range(1,7)];aa=[v['wall_s'] for v in rows if not v['block']]
 result=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
 for label in 'AB':
  samples=[v for v in rows if v['label']==label and v['block']];result[label]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
 r['summary'][name]=result;save();print(name,result['paired_ratio_median'],flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items())

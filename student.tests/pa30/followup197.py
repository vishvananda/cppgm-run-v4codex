#!/usr/bin/env python3
"""Revisit two noisy largest samples; preserve the entire original series."""
import hashlib,json,os,pathlib,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
owner_path=pathlib.Path(sys.argv[2]).resolve();owner=json.loads(owner_path.read_text());base=owner_path.parent
compiler=pathlib.Path(sys.argv[3]).resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(compiler)==owner['binaries']['B']['sha256']
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
r=dict(compiler_sha256=sha(compiler),flags=owner['flags'],affinity=affinity,inputs={},runs=[])
for name in ['lookup2048','ordering2048']:
 src=base/(name+'.cpp');exe=base/name
 assert sha(src)==owner['inputs'][name]['sha256']
 assert sha(exe)==owner['images'][name]['executable_sha256']
 r['inputs'][name]=dict(source_sha256=sha(src),executable_sha256=sha(exe))
 for mode in ['compile','runtime']:
  for trial in range(4):
   obj=out/(name+'.o');args=[compiler,*r['flags'],src,'-o',obj] if mode=='compile' else [exe,'7']
   start=time.perf_counter();p=subprocess.run(list(map(str,['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])),capture_output=True,text=True,timeout=45)
   row=dict(workload=name,mode=mode,trial=trial,status=p.returncode,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
   assert p.returncode==0,p.stderr
   if mode=='compile':
    row['object_sha256']=sha(obj);assert row['object_sha256']==owner['images'][name]['object_sha256']
   r['runs'].append(row);(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
 print(name,[(v['mode'],v['wall_s']) for v in r['runs'] if v['workload']==name],flush=True)

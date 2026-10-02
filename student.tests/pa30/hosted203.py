#!/usr/bin/env python3
"""Final-only newly accepted heavy-header cost; retain entry rejection."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
a,b=map(lambda p:pathlib.Path(p).resolve(),sys.argv[2:4]);affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in [('A',a),('B',b)]},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},runs=[],images={},summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in ['600-random-to-address-qualified-call','700-hosted-random-mersenne-rshift-compile']:
 src=root/f'pa30/tests/compile/{name}.t';r['inputs'][name]=dict(path=str(src.relative_to(root)),sha256=sha(src))
 p=subprocess.run([str(a),'-c',str(src),'-o',str(out/'entry.o')],capture_output=True,text=True,timeout=45);assert p.returncode
 r['inputs'][name]['entry_rejection']=dict(status=p.returncode,stderr=p.stderr)
 for trial in range(8):
  obj=out/(name+'.o');start=time.perf_counter();p=subprocess.run(list(map(str,['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,b,*r['flags'],src,'-o',obj])),capture_output=True,text=True,timeout=45)
  row=dict(workload=name,trial=trial,status=p.returncode,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),stderr=p.stderr,object_sha256=sha(obj) if obj.exists() else None);r['runs'].append(row);save();assert not p.returncode
  text=sum(int(s.split()[1]) for s in subprocess.check_output(['size','-A',obj],text=True).splitlines() if s.split() and s.split()[0].startswith('.text'))
  image=dict(object_sha256=sha(obj),object_text_bytes=text)
  if name in r['images']:assert image==r['images'][name]
  r['images'][name]=image
 rows=[s for s in r['runs'] if s['workload']==name];r['summary'][name]=dict(median_s=statistics.median(s['wall_s'] for s in rows),range_s=[min(s['wall_s'] for s in rows),max(s['wall_s'] for s in rows)],peak_rss_kib=max(s['peak_rss_kib'] for s in rows));save();print(name,r['summary'][name],flush=True)
assert sha(a)==r['binaries']['A']['sha256'] and sha(b)==r['binaries']['B']['sha256']

#!/usr/bin/env python3
"""Final-only compile cost on the nine previously failing hosted fixtures."""
import hashlib,json,os,pathlib,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve()
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-O0','-c','--stats'];r=dict(compiler=str(compiler),sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),flags=flags,affinity=affinity,runs=[],inputs={})
for path in json.loads((root/'student.tests/pa30/evidence195/stage-delta.json').read_text())['fixed']:
 source=root/path;obj=out/(source.stem+'.o')
 r['inputs'][path]=hashlib.sha256(source.read_bytes()).hexdigest()
 for trial in range(4):
  start=time.perf_counter()
  args=['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,compiler,*flags,source,'-o',obj]
  p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
  row=dict(path=path,trial=trial,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
  assert p.returncode==0,(path,p.stderr)
  size=subprocess.check_output(['size','-A',obj],text=True)
  row['object_text_bytes']=sum(int(s.split()[1]) for s in size.splitlines() if s.split() and s.split()[0].startswith('.text'))
  row['object_sha256']=hashlib.sha256(obj.read_bytes()).hexdigest();r['runs'].append(row)
  (out/'hosted.json').write_text(json.dumps(r,indent=2)+'\n')
 print(path,flush=True)
assert hashlib.sha256(compiler.read_bytes()).hexdigest()==r['sha256']

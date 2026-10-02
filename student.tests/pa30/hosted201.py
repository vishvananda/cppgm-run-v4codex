#!/usr/bin/env python3
"""Final costs of repaired rejections and retained heavy-header emission."""
import hashlib,json,os,pathlib,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=pathlib.Path(sys.argv[2]).resolve();affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
flags=['-O0','-c','--stats'];r=dict(compiler=str(cc),sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),flags=flags,affinity=affinity,inputs={},runs=[])
paths=['200-local-callable-cross-function-reference-negative','400-reachable-missing-return-bad','700-hosted-replaceable-operator-new-dynamic-exception-spec','600-noreturn-control-convergence','700-libstdcxx-regex-compiler-member-alias-call','600-hosted-recursive-std-function-string-substr','700-hosted-map-subscript-piecewise-construct-compile']
for name in paths:
 path='pa30/tests/compile/'+name+'.t';src=root/path;obj=out/(name+'.o');reject=(src.with_suffix('.ref.exit_status').read_text().strip()=='EXIT_FAILURE')
 r['inputs'][path]=dict(sha256=hashlib.sha256(src.read_bytes()).hexdigest(),expected_rejection=reject)
 for trial in range(4):
  obj.unlink(missing_ok=True);start=time.perf_counter();args=['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,cc,*flags,src,'-o',obj]
  p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=45)
  row=dict(path=path,trial=trial,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text().splitlines()[-1]),status=p.returncode,stderr=p.stderr,phase_counters=[json.loads(s) for s in p.stderr.splitlines() if s.startswith('{')])
  assert (p.returncode!=0)==reject,(path,p.stderr)
  if reject:assert not obj.exists()
  else:
   size=subprocess.check_output(['size','-A',obj],text=True)
   row.update(object_text_bytes=sum(int(s.split()[1]) for s in size.splitlines() if s.split() and s.split()[0].startswith('.text')),object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest())
  r['runs'].append(row);(out/'hosted.json').write_text(json.dumps(r,indent=2)+'\n')
 print(path,flush=True)
assert hashlib.sha256(cc.read_bytes()).hexdigest()==r['sha256']

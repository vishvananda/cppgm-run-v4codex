#!/usr/bin/env python3
"""Owning PA13 lifecycle, signature and cross-TU controls under both compilers."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
A,B,WORK=map(lambda p:Path(p).resolve(),sys.argv[1:4]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
result=dict(harness_sha256=sha(__file__),binaries=[dict(path=str(p),sha256=sha(p)) for p in (A,B)],checks=[])
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
for label,binary in [('release',A),('sanitized',B)]:
 for name in ('check_native','check_virtual_semantics','check_ir','check_linkage','audit_check','check_literal_storage'):
  harness=ROOT/f'student.tests/pa13/{name}.py';cmd=[sys.executable,str(harness),str(binary)]
  p=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True,timeout=600)
  log=WORK/(label+'-'+name+'.log');log.write_text(p.stdout+p.stderr)
  result['checks'].append(dict(command=cmd,harness_path=str(harness),harness_sha256=sha(harness),log=str(log),log_sha256=sha(log),exit_code=p.returncode))
  (ROOT/'student.tests/pa14/virtual-demand-abi-validation.json').write_text(json.dumps(result,indent=2)+'\n')
  assert p.returncode==0,(name,p.returncode,p.stdout,p.stderr)
  print(label,name,'PASS',flush=True)

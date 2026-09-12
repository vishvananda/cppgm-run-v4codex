#!/usr/bin/env python3
"""Check the same course/personal inputs with frozen release and ASan/UBSan builds.
An incomplete-stage rejection must remain a rejection; sanitizer errors are
never accepted as diagnostics. Successful typed LowIR must be byte-identical.
"""
from pathlib import Path
import os, subprocess, sys, tempfile
root=Path(__file__).resolve().parents[2]
release,sanitized=map(lambda p:Path(p).resolve(),sys.argv[1:3])
inputs=sorted((root/'pa14/tests').glob('*/*.t'))+sorted((root/'student.tests/pa14').glob('*.cpp'))
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
with tempfile.TemporaryDirectory(prefix='pa14-sanitizers-') as tmp:
 for i,source in enumerate(inputs):
  outputs=[];statuses=[]
  for j,binary in enumerate((release,sanitized)):
   ir=Path(tmp)/f'{i}-{j}.lowir'
   command=[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source]
   r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60,env=env)
   assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr and 'UndefinedBehaviorSanitizer' not in r.stderr,(source,r.stderr)
   assert r.returncode>=0,(source,r.returncode,r.stderr)
   statuses.append(r.returncode);outputs.append(ir.read_bytes() if r.returncode==0 else None)
  assert statuses[0]==statuses[1],(source,statuses)
  assert outputs[0]==outputs[1],(source,'different successful LowIR')
  if (i+1)%50==0: print(i+1,'checked',flush=True)
 print(len(inputs),'source status/output/sanitizer checks PASS')

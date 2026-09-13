#!/usr/bin/env python3
"""Run object and prototype ownership reducers explicitly."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
with tempfile.TemporaryDirectory(prefix='pa14-object-reducers-') as directory:
 work=Path(directory)
 for name in ('field-category','default-identity','parameter-shape'):
  ir=work/(name+'.lowir');exe=work/name;source=root/'student.tests/pa14'/(name+'.t')
  for command in ([binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source],
                  [root/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
   result=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60)
   assert result.returncode==0,(name,command,result.returncode,result.stderr)
   assert not any(marker in result.stderr for marker in ('AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:')),result.stderr
  print(name,'PASS')

#!/usr/bin/env python3
"""Compile and execute independent, defined PA13 behavior through PA8's backend."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
binary=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'dev/cppgm++'
with tempfile.TemporaryDirectory(prefix='pa13-native-') as tmp:
 for source in sorted((root/'student.tests/pa13').glob('*.cpp')):
  ir=Path(tmp)/(source.stem+'.lowir'); exe=Path(tmp)/source.stem
  for cmd in ([binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source],
              [root/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]):
   run=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60)
   assert run.returncode==0,(source.name,list(map(str,cmd)),run.returncode,run.stderr)
  print(source.name, 'passed')

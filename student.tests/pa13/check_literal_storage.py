#!/usr/bin/env python3
"""Typed LowIR literal-address reducer: unreachable null load and bad float."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='pa13-storage-') as tmp:
 for name,value,valid in [('null','0',True),('floating','0.5',False)]:
  src=Path(tmp)/(name+'.lowir');out=Path(tmp)/'out'
  src.write_text('function @main() -> i32 {\nblock ^entry:\n return i32 0\nblock ^unreachable:\n %p = load ptr '+value+'\n return i32 1\n}\n')
  r=subprocess.run([str(root/'dev/lowir'),'-o',str(out),str(src)],capture_output=True,text=True)
  assert (r.returncode==0)==valid,(name,r.stderr)
print('literal storage accepts integer pointer and rejects floating pointer')

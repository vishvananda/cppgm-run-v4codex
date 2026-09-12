#!/usr/bin/env python3
"""Run all course sources with an alternate compiler and check sanitizer output."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
compiler=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='pa13-course-') as tmp:
 cases=sorted((root/'pa13/tests').glob('*/*.t'))
 for source in cases:
  expected=source.with_suffix('.ref.exit_status').read_text().strip()=='EXIT_SUCCESS'
  cmd=[compiler,'--emit-lowir','-O0','--validate-lowir','-o',Path(tmp)/(source.stem+'.lowir'),source]
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60)
  assert (r.returncode==0)==expected,(source,r.returncode,r.stderr)
  assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr,(source,r.stderr)
 print(len(cases),'course status/LowIR/sanitizer controls passed')

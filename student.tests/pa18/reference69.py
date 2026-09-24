#!/usr/bin/env python3
"""Record a reduced incorrect noexcept oracle: WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
work,out=map(Path,sys.argv[1:]);work.mkdir(parents=True,exist_ok=True)
assert not out.exists()
inputs=[ROOT/'student.tests/pa18/noexcept69_reducer.cpp',ROOT/'pa18/tests/spec/300-scalar-pseudo-destructor-noexcept.t']
data=dict(bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',proof='N3485 5.2.4/1, 5.3.7/3, 15.4/12, 7/4',observations=[])
for source in inputs:
 for label,cc in [('reference','dev/cppgm++-ref'),('student','dev/cppgm++')]:
  ir=work/(source.stem+'-'+label+'.lowir');cmd=[cc,'--emit-lowir','-O0','-o',str(ir),str(source)]
  r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=30)
  data['observations'].append(dict(source=str(source.relative_to(ROOT)),source_text=source.read_text(),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),compiler=cc,command=cmd,exit=r.returncode,diagnostic=r.stderr,lowir=ir.read_text() if ir.exists() else None))
out.write_text(json.dumps(data,indent=2)+'\n')
assert [r['exit'] for r in data['observations']]==[0,1,0,1]
print('Pinned reference accepts both ill-formed static assertions; student rejects both.')

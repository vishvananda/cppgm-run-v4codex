#!/usr/bin/env python3
"""Observe pinned reference failure and correct student execution; WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=map(Path,sys.argv[1:]);WORK.mkdir(parents=True,exist_ok=True)
source=ROOT/'student.tests/pa18/constant_reference_reducer.cpp'
data=dict(bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
 proof='N3485 3.6.2 [basic.start.init]/2; 5.19 [expr.const]; 7.1.5 [dcl.constexpr]',
 source=source.read_text(),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),observations=[])
for label,cc in [('reference','dev/cppgm++-ref'),('student','dev/cppgm++')]:
 ir=WORK/(label+'.lowir');exe=WORK/(label+'.exe')
 command=[cc,'--emit-lowir','-O0','-o',str(ir),str(source)]
 r=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=30)
 row=dict(compiler=cc,command=command,exit=r.returncode,diagnostic=r.stderr)
 if not r.returncode:
  row['lowir']=ir.read_text()
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if not b.returncode:row['native_exit']=subprocess.run([exe],timeout=30).returncode
 data['observations'].append(row)
OUT.write_text(json.dumps(data,indent=2)+'\n')
assert all(r['exit']==r['backend_exit']==0 for r in data['observations'])
assert data['observations'][0]['native_exit']!=0
assert data['observations'][1]['native_exit']==0
print('Reference violates constant initialization; student reducer exits zero.')

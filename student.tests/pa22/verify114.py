#!/usr/bin/env python3
"""Explicit PA22 member-pointer controls; optional compiler/work arguments."""
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else Path('/tmp/pa22-114/controls')
WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for source in sorted([*Path(__file__).parent.glob('member-*.cpp'),*Path(__file__).parent.glob('rtti-*.cpp')]):
 ir=WORK/(source.stem+'.lowir');exe=WORK/source.stem
 result=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,source],capture_output=True,text=True)
 reject='reject' in source.stem
 row=dict(source=str(source.relative_to(ROOT)),compile_exit=result.returncode,stderr=result.stderr,reject=reject)
 if not reject and result.returncode==0:
  backend=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row['backend_exit']=backend.returncode;row['backend_stderr']=backend.stderr
  if not backend.returncode:row['runtime_exit']=subprocess.run([exe],timeout=20).returncode
 row['passed']=bool(result.returncode) if reject else row.get('runtime_exit')==0
 rows.append(row)
print(json.dumps(rows,indent=2))
sys.exit(not all(r['passed'] for r in rows))

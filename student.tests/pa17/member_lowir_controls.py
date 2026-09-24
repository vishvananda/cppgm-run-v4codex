#!/usr/bin/env python3
"""Run the four inherited emission-property controls explicitly."""
from pathlib import Path
import json, re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
cc=Path(sys.argv[1]).resolve(); work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
rows=json.loads((ROOT/'student.tests/pa17/member-lowir-results.json').read_text())
for row in rows:
 src=work/(row['name']+'.cpp');src.write_text(row['source']);ir=src.with_suffix('.lowir')
 p=subprocess.run([str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
 row['compiler_exit']=p.returncode;row['diagnostic']=p.stderr
 out=ir.read_text() if not p.returncode else ''
 row['passed']=not p.returncode and bool(re.search(row['required'],out,re.M)) and not bool(re.search(row['forbidden'],out,re.M))
 print(row['name'],'PASS' if row['passed'] else 'FAIL')
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)

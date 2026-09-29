#!/usr/bin/env python3
"""Validate addressable object zeroinit without weakening size/type checks."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
cases=[('whole','slot $s : obj<8x8>','8x8 $s',True),('partial','slot $s : obj<8x8>','4x4 $s',True),
       ('overrun','slot $s : obj<8x8>','16x8 $s',False),('alignment','slot $s : obj<8x8>','8x16 $s',False),
       ('integer','slot $s : i64','8x8 $s',False),('object_value','','8x8 %v',False)]
rows=[]
for name,slot,operation,valid in cases:
 source='function @f('+('%v : obj<8x8>' if name=='object_value' else '')+') -> void {\n'+slot+'\nblock ^b:\nzeroinit '+operation+'\nreturn void\n}\n'
 path=WORK/(name+'.lowir');path.write_text(source)
 command=[str(ROOT/'dev/lowir'),'-o',str(WORK/(name+'.out')),str(path)]
 p=subprocess.run(command,capture_output=True,text=True)
 passed=(p.returncode==0)==valid
 rows.append(dict(name=name,source=source,expected_valid=valid,command=command,exit=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=passed))
 print(name,'PASS' if passed else 'FAIL')
assert all(r['passed'] for r in rows),rows
print(json.dumps(dict(validator_sha256=hashlib.sha256((ROOT/'dev/lowir').read_bytes()).hexdigest(),rows=rows),indent=2))
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')

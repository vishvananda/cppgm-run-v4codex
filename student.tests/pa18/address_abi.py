#!/usr/bin/env python3
"""Fixed Itanium §5.1.6.2/§5.1.10 ABI checks; no live host-name oracle."""
from pathlib import Path
import json,re,subprocess,sys
from address_controls import GOOD
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
CASES={
 'qualification':['_Z1fIXadL_Z1xEEEiv'],
 'reference':['_ZN1AIL_Z1xEE1fEv'],
 'function':['_Z1gIXadL_Z1fiEEEiv'],
 'function_reference':['_Z1gIL_Z1fiEEiv'],
 'function_template':['_Z1gIXadL_Z1fIiET_S1_EEEiv'],
 'function_static_template':['_Z1gIXadL_ZN1A1fIiEET_S2_EEEiv'],
 'array':['_Z1fIXadL_Z1xEEEiv'],
 'array_reference':['_Z1fIL_Z1xEEiv'],
 'null_keyword':['_ZN1AILPi0EE1fEv'],
 'null_cast':['_ZN1AILPi0EE1fEv'],
 'static_distinct':['_ZN1AIXadL_ZN1X1nEEEE1fEv','_ZN1AIXadL_ZN1Y1nEEEE1fEv'],
}
rows=[]
for name,expected in CASES.items():
 src=WORK/(name+'.cpp');src.write_text(GOOD[name]);ir=src.with_suffix('.lowir')
 r=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
 actual=re.findall(r'^function .*object=([^,\] ]+)',ir.read_text(),re.M) if not r.returncode else []
 row=dict(name=name,source=GOOD[name],expected=expected,actual=actual,exit=r.returncode,diagnostic=r.stderr,passed=r.returncode==0 and all(actual.count(x)==1 for x in expected));rows.append(row)
 print(name,'PASS' if row['passed'] else 'FAIL',actual,flush=True)
src=ROOT/'student.tests/pa18/address_abi_reducer.abi';out=WORK/'reducer.out'
r=subprocess.run([ROOT/'dev/abimangle','-o',out,src],capture_output=True,text=True)
expected='_ZN2ns6HolderIXadL_ZN1C1mEEEE1fERS1_\n';actual=out.read_text() if not r.returncode else ''
rows.append(dict(name='structured-reducer',source=src.read_text(),expected=expected,actual=actual,exit=r.returncode,passed=r.returncode==0 and actual==expected))
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
print('12 source/structured ABI checks pass.')

#!/usr/bin/env python3
"""Run the new controls through the handout's standalone LowIR backend."""
from pathlib import Path
import json,subprocess,hashlib,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();rows=[]
controls=json.loads((ROOT/'student.tests/pa23/controls123-layout.json').read_text())
for case in controls['cases']:
 name=case['name'];ir=WORK/(name+'.lowir');exe=WORK/(name+'.standalone')
 digest=hashlib.sha256(ir.read_bytes()).hexdigest();assert digest==case['lowir_sha256']
 p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
 row=dict(name=name,unfinished=case['unfinished'],lowir_sha256=digest,backend_exit=p.returncode,backend_diagnostic=p.stderr)
 if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=row.get('runtime_exit')==0;rows.append(row)
 print(name,'PASS' if row['passed'] else 'FAIL',file=sys.stderr,flush=True)
print(json.dumps(dict(cases=rows),indent=2));sys.exit(not all(r['passed'] for r in rows if not r['unfinished']))

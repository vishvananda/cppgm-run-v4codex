#!/usr/bin/env python3
"""Run the sealed lifecycle controls through PA23's standalone reference backend."""
from pathlib import Path
import json,subprocess,hashlib,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve()
controls=json.loads((ROOT/'student.tests/pa23/controls122.json').read_text());rows=[]
for c in controls['cases']:
 name=c['name'];ir=WORK/(name+('.merged.lowir' if len(c['sources'])>1 else '0.lowir'));exe=ir.with_suffix('.standalone')
 p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
 row=dict(name=name,lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest(),backend_exit=p.returncode,diagnostic=p.stderr)
 if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=row.get('runtime_exit')==0;rows.append(row)
print(json.dumps(dict(cases=rows),indent=2));sys.exit(not all(r['passed'] for r in rows))

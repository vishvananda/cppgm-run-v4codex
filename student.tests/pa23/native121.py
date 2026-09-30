#!/usr/bin/env python3
"""Explicit standalone-backend check of the retained PA23 controls."""
from pathlib import Path
import json,os,subprocess,hashlib
ROOT=Path(__file__).resolve().parents[2];ART=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa23-121'
controls=json.loads((ROOT/'student.tests/pa23/controls121-native.json').read_text());rows=[]
for case in controls['cases']:
 if case['reject'] or case['compile_exit']:continue
 name=case['name'];ir=ART/'controls-native'/(name+'.lowir');exe=ART/'controls-native'/(name+'.standalone')
 p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
 row=dict(name=name,lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest(),backend_exit=p.returncode,backend_diagnostic=p.stderr)
 if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=row.get('runtime_exit')==0;rows.append(row)
(ROOT/'student.tests/pa23/native121.json').write_text(json.dumps(dict(cases=rows),indent=2)+'\n')
print([(r['name'],r['backend_exit'],r.get('runtime_exit'),r['backend_diagnostic']) for r in rows if not r['passed']])
raise SystemExit(not all(r['passed'] for r in rows))

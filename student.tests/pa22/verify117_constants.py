#!/usr/bin/env python3
"""Member constant provenance through conversion, evaluation, ABI and static data."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];CC=Path(sys.argv[1]).resolve();WORK=Path(sys.argv[2]).resolve();WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for src in sorted((ROOT/'student.tests/pa22/constants').glob('*.cpp')):
 ir=WORK/(src.stem+'.lowir');exe=WORK/src.stem
 r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
 row=dict(source=str(src.relative_to(ROOT)),compile_exit=r.returncode,diagnostic=r.stderr)
 if not r.returncode:
  row['static_initialization']='role=init' not in ir.read_text()
  r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row.update(backend_exit=r.returncode,backend_diagnostic=r.stderr)
  if not r.returncode:row['runtime_exit']=subprocess.run([exe],timeout=20).returncode
 row['passed']=row.get('runtime_exit')==0 and row.get('static_initialization',False)
 rows.append(row)
print(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))

#!/usr/bin/env python3
"""Explicit dependent member-pointer execution and rejection controls."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else Path('/tmp/pa22-115/controls')
WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for source in sorted((Path(__file__).parent/'dependent').glob('*.cpp')):
 ir=WORK/(source.stem+'.lowir');exe=WORK/source.stem
 r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,source],capture_output=True,text=True)
 reject=source.stem.startswith('reject-')
 row=dict(source=str(source.relative_to(ROOT)),compile_exit=r.returncode,stderr=r.stderr,reject=reject)
 if not reject and not r.returncode:
  backend=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row.update(backend_exit=backend.returncode,backend_stderr=backend.stderr)
  if not backend.returncode:row['runtime_exit']=subprocess.run([exe],timeout=20).returncode
 row['passed']=bool(r.returncode) if reject else row.get('runtime_exit')==0
 rows.append(row)
print(json.dumps(rows,indent=2))
sys.exit(not all(r['passed'] for r in rows))

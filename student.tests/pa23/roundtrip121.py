#!/usr/bin/env python3
"""Roundtrip every accepted PA23 output, including remaining shape failures."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for src in sorted((ROOT/'pa23/tests/general').glob('*.t')):
 ir=src.with_suffix('.my');status=Path(str(ir)+'.exit_status')
 if not status.exists() or status.read_text().strip()!='EXIT_SUCCESS':continue
 first=WORK/(src.stem+'.a.lowir');second=WORK/(src.stem+'.b.lowir')
 commands=[[str(ROOT/'dev/lowir'),'-o',str(first),str(ir)],[str(ROOT/'dev/lowir'),'-o',str(second),str(first)]]
 row=dict(source=str(src.relative_to(ROOT)),lowir_sha256=hashlib.sha256(ir.read_bytes()).hexdigest())
 for label,out,cmd in [('first',first,commands[0]),('second',second,commands[1])]:
  p=subprocess.run(cmd,capture_output=True)
  row[label+'_exit']=p.returncode;row[label+'_diagnostic']=p.stderr.decode()
  if p.returncode:break
 row['stable']=row.get('second_exit')==0 and first.read_bytes()==second.read_bytes()
 rows.append(row)
print(json.dumps(dict(cases=rows),indent=2));sys.exit(not all(r['stable'] for r in rows))

#!/usr/bin/env python3
"""Replay the frozen accumulated PA17 controls plus this increment's controls."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]).resolve();out=Path(sys.argv[3])
groups=json.loads((ROOT/'student.tests/pa17/checkpoint56-controls.json').read_text())
import transfer_controls
for good,cases in [(True,transfer_controls.runner.GOOD),(False,transfer_controls.runner.BAD)]:
 groups.setdefault('transfer',[]).extend(dict(name=name,source=source,expected='native' if good else 'reject') for name,source in cases.items())
for group,rows in groups.items():
 folder=work/group;folder.mkdir(parents=True,exist_ok=True)
 for row in rows:
  src=folder/(row['name']+'.cpp');src.write_text(row['source']);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
  row['source_sha256']=hashlib.sha256(src.read_bytes()).hexdigest()
  r=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
  row.update(compiler_exit=r.returncode,diagnostic=r.stderr)
  if 'required' in row:
   text=ir.read_text() if not r.returncode else ''
   row['passed']=not r.returncode and bool(re.search(row['required'],text,re.M)) and not bool(re.search(row['forbidden'],text,re.M))
  elif row['expected']=='reject':row['passed']=r.returncode!=0
  else:
   row['passed']=False
   if not r.returncode:
    b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
    row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
    row['native_exit']=subprocess.run([exe],timeout=30).returncode if not b.returncode else None
    row['passed']=not b.returncode and row['native_exit']==0
  if not row['passed']:print(group,row['name'],'FAIL',r.stderr,flush=True)
 print(group,sum(r['passed'] for r in rows),'/',len(rows),flush=True)
 out.write_text(json.dumps(groups,indent=2)+'\n')
sys.exit(0 if all(r['passed'] for rows in groups.values() for r in rows) else 1)

#!/usr/bin/env python3
"""Roundtrip every accepted PA22 course output without changing its comparator."""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-117/roundtrips');WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for source in sorted((ROOT/'pa22/tests').rglob('*.t')):
 status=source.with_suffix('.my.exit_status').read_text().strip()
 if status!='EXIT_SUCCESS':
  assert status==source.with_suffix('.ref.exit_status').read_text().strip()
  rows.append(dict(source=str(source.relative_to(ROOT)),rejected=True));continue
 ir=source.with_suffix('.my');first=WORK/(source.stem+'.first.lowir');second=WORK/(source.stem+'.second.lowir')
 for a,b in [(ir,first),(first,second)]:
  subprocess.run([str(ROOT/'dev/lowir'),'-o',str(b),str(a)],check=True,capture_output=True,text=True)
 assert first.read_bytes()==second.read_bytes(),source
 rows.append(dict(source=str(source.relative_to(ROOT)),rejected=False,stable_roundtrip=True,
                  source_sha256=hashlib.sha256(ir.read_bytes()).hexdigest(),roundtrip_sha256=hashlib.sha256(first.read_bytes()).hexdigest()))
assert len(rows)==99
(ROOT/'student.tests/pa22/audit117-roundtrips.json').write_text(json.dumps(rows,indent=2)+'\n')
print(len(rows),'cases;',sum(not r['rejected'] for r in rows),'stable accepted roundtrips; all rejection statuses preserved')

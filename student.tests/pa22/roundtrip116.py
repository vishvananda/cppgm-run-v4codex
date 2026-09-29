#!/usr/bin/env python3
"""Validate and roundtrip each repaired contract case; compare native outcomes."""
from pathlib import Path
import json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-116/roundtrip');WORK.mkdir(parents=True,exist_ok=True)
fail=lambda p:set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',p.read_text()))
assert '94 / 99 TESTS PASSED' in Path('/tmp/pa22-116/stage-final.log').read_text()
old=fail(Path('/tmp/pa22-116/entry.log'));new=fail(Path('/tmp/pa22-116/stage-final.log'))
assert len(old)==8 and len(new)==5 and new<=old
rows=[]
def run(cmd):return subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=30)
for i,case in enumerate(sorted(old-new)):
 source=ROOT/case;ir=WORK/(str(i)+'.lowir');roundtrip=WORK/(str(i)+'.round.lowir')
 commands=[[ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,source],
 [ROOT/'dev/lowir','-o',roundtrip,ir]]
 row=dict(source=case,checks=[])
 for cmd in commands:
  r=run(cmd);row['checks'].append(dict(command=[str(x) for x in cmd],exit=r.returncode,stderr=r.stderr));assert not r.returncode,(case,r.stderr)
 again=WORK/(str(i)+'.again.lowir');r=run([ROOT/'dev/lowir','-o',again,roundtrip]);assert not r.returncode
 row['stable_roundtrip']=roundtrip.read_bytes()==again.read_bytes();assert row['stable_roundtrip']
 results=[]
 if not re.search(r"\brole=entry\b",ir.read_text()):
  row["execution_note"]="declaration-only fixture: no executable entry";rows.append(row);continue
 for label,text in [('student',ir),('reference',source.with_suffix('.ref'))]:
  exe=WORK/(str(i)+'.'+label);r=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,text])
  if r.returncode:
   assert 'undefined native symbol:' in r.stderr,(case,label,r.stderr)
   results.append(dict(lane=label,backend_exit=r.returncode,stderr=r.stderr));continue
  r=run([exe]);results.append(dict(lane=label,exit=r.returncode,stdout=r.stdout,stderr=r.stderr))
 row['execution']=results
 if 'backend_exit' in results[0] or 'backend_exit' in results[1]:
  assert 'backend_exit' in results[0] and 'backend_exit' in results[1],(case,results)
  row['execution_note']='Both lanes lack definitions for externally declared native symbols; LowIR validation/roundtrip passed'
 else: assert (results[0]['exit'],results[0]['stdout'])==(results[1]['exit'],results[1]['stdout']),(case,results)
 rows.append(row)
(ROOT/'student.tests/pa22/roundtrips116.json').write_text(json.dumps(rows,indent=2)+'\n')
print(len(rows),'repaired fixtures: validated roundtrips and matching native outcomes')
